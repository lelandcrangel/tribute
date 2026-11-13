#include "Mothership.h"

#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SaucerPawn.h"
#include "Tribute.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	/** Pool brightness, in candelas: dim marks where each sector's pool is; lit is the call or an answer. */
	constexpr float DimPool = 40000.f;
	constexpr float LitPool = 180000.f;
	/** How bright the valley glow gets once the ship is down. */
	constexpr float UnderGlow = 14000.f;
	/** How long an answer's flash lasts. */
	constexpr float FlashSeconds = 0.5f;

	float EaseOut(float Alpha) { return 1.f - FMath::Pow(1.f - FMath::Clamp(Alpha, 0.f, 1.f), 3.f); }
	float EaseIn(float Alpha) { return FMath::Pow(FMath::Clamp(Alpha, 0.f, 1.f), 3.f); }
}

AMothership::AMothership()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Glow(TEXT("/Game/Tribute/Mothership/M_SectorGlow.M_SectorGlow"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Tintable(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	GlowMaterial = Glow.Succeeded() ? Glow.Object : nullptr;
	HullMaterial = Tintable.Succeeded() ? Tintable.Object : nullptr;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Two 200 m slabs, 10 m thick, crossed at 45 degrees: dark and angular.
	auto MakeSlab = [this](const TCHAR* Name, float Yaw)
	{
		UStaticMeshComponent* Slab = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Slab->SetupAttachment(RootComponent);
		if (CubeMesh.Succeeded())
		{
			Slab->SetStaticMesh(CubeMesh.Object);
		}
		Slab->SetRelativeScale3D(FVector(200.f, 200.f, 10.f));
		Slab->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
		Slab->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return Slab;
	};
	HullA = MakeSlab(TEXT("HullA"), 0.f);
	HullB = MakeSlab(TEXT("HullB"), 45.f);

	Ring = CreateDefaultSubobject<USceneComponent>(TEXT("Ring"));
	Ring->SetupAttachment(RootComponent);
	Ring->SetRelativeLocation(FVector(0.f, 0.f, -650.f));

	for (int32 Sector = 0; Sector < FTributeScale::SectorCount; ++Sector)
	{
		UStaticMeshComponent* Panel = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Sector%d"), Sector));
		Panel->SetupAttachment(Ring);
		if (CubeMesh.Succeeded())
		{
			Panel->SetStaticMesh(CubeMesh.Object);
		}
		// 30 m along the ring, 12 m across, 1.5 m deep.
		Panel->SetRelativeScale3D(FVector(12.f, 30.f, 1.5f));
		Panel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Panel->SetCastShadow(false);
		Sectors.Add(Panel);

		USpotLightComponent* Pool = CreateDefaultSubobject<USpotLightComponent>(*FString::Printf(TEXT("Pool%d"), Sector));
		Pool->SetupAttachment(Ring);
		Pool->SetMobility(EComponentMobility::Movable);
		Pool->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
		Pool->SetIntensityUnits(ELightUnits::Candelas);
		Pool->SetIntensity(0.f);
		Pool->SetCastShadows(false);
		Pool->SetVisibility(false);
		Pools.Add(Pool);
	}

	UnderLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("UnderLight"));
	UnderLight->SetupAttachment(RootComponent);
	UnderLight->SetRelativeLocation(FVector(0.f, 0.f, -700.f));
	UnderLight->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	UnderLight->SetMobility(EComponentMobility::Movable);
	UnderLight->SetIntensityUnits(ELightUnits::Candelas);
	UnderLight->SetIntensity(0.f);
	UnderLight->SetOuterConeAngle(70.f);
	UnderLight->SetInnerConeAngle(40.f);
	UnderLight->SetAttenuationRadius(60000.f);
	UnderLight->SetCastShadows(false);

	RoundColours = { FLinearColor(0.2f, 0.8f, 1.f), FLinearColor(1.f, 0.6f, 0.15f), FLinearColor(0.75f, 0.3f, 1.f) };
}

void AMothership::BeginPlay()
{
	Super::BeginPlay();

	// Placed above the valley's centre; it waits high above the clouds until the clock runs out.
	const FVector Ground(GetActorLocation().X, GetActorLocation().Y, ValleyFloorZ);
	DownLocation = Ground + FVector(0.f, 0.f, HoverHeight);
	UpLocation = Ground + FVector(0.f, 0.f, StartHeight);
	SetActorLocation(UpLocation);

	const float ConeDegrees = FMath::RadiansToDegrees(FMath::Atan2(PoolRadius, HoverHeight));
	for (int32 Sector = 0; Sector < Sectors.Num(); ++Sector)
	{
		const float Angle = Sector * 2.f * PI / Sectors.Num();
		const FVector Offset(RingRadius * FMath::Cos(Angle), RingRadius * FMath::Sin(Angle), 0.f);
		Sectors[Sector]->SetRelativeLocation(Offset);
		Sectors[Sector]->SetRelativeRotation(FRotator(0.f, FMath::RadiansToDegrees(Angle), 0.f));
		Pools[Sector]->SetRelativeLocation(Offset - FVector(0.f, 0.f, 150.f));
		Pools[Sector]->SetOuterConeAngle(ConeDegrees);
		Pools[Sector]->SetInnerConeAngle(ConeDegrees * 0.8f);
		Pools[Sector]->SetAttenuationRadius(HoverHeight * 1.6f);
		if (GlowMaterial)
		{
			UMaterialInstanceDynamic* Tint = UMaterialInstanceDynamic::Create(GlowMaterial, this);
			Sectors[Sector]->SetMaterial(0, Tint);
			SectorTints.Add(Tint);
		}
	}
	if (HullMaterial)
	{
		UMaterialInstanceDynamic* Dark = UMaterialInstanceDynamic::Create(HullMaterial, this);
		Dark->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.012f, 0.012f, 0.016f));
		HullA->SetMaterial(0, Dark);
		HullB->SetMaterial(0, Dark);
	}
	UpdateSectors(0.f);
}

void AMothership::BeginArrival(ASaucerPawn* InSaucer)
{
	if (bArriving || bDown)
	{
		return;
	}
	Saucer = InSaucer;
	if (InSaucer)
	{
		InSaucer->OnBeamPulsed.AddDynamic(this, &AMothership::HandlePulse);
	}
	bArriving = true;
	MoveTime = 0.f;
	UE_LOG(LogTribute, Log, TEXT("The mothership is coming down"));
	ArrivalStarted();
}

void AMothership::Depart()
{
	bDeparting = true;
	bArriving = false;
	bDown = false;
	MoveTime = 0.f;
	UE_LOG(LogTribute, Log, TEXT("The mothership is leaving"));
}

void AMothership::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bArriving)
	{
		MoveTime += DeltaSeconds;
		const float Alpha = EaseOut(MoveTime / ArrivalTime);
		SetActorLocation(FMath::Lerp(UpLocation, DownLocation, Alpha));
		UnderLight->SetIntensity(UnderGlow * Alpha);
		if (MoveTime >= ArrivalTime)
		{
			bArriving = false;
			bDown = true;
			Run.Start(Seed != 0 ? Seed : FMath::Rand());
			UE_LOG(LogTribute, Log, TEXT("The mothership is down; the tribute begins"));
		}
	}
	else if (bDeparting)
	{
		MoveTime += DeltaSeconds;
		const float Alpha = EaseIn(MoveTime / ArrivalTime);
		SetActorLocation(FMath::Lerp(DownLocation, UpLocation, Alpha));
		UnderLight->SetIntensity(UnderGlow * (1.f - Alpha));
	}

	if (bDown && !bEnded)
	{
		const ETributeAnswer Timeout = Run.Advance(DeltaSeconds);
		if (Timeout != ETributeAnswer::Ignored)
		{
			HandleAnswer(Timeout, INDEX_NONE);
		}
		if (Run.Phase != LastPhase)
		{
			UE_LOG(LogTribute, Log, TEXT("Tribute: %s, round %d, misses %d"), *UEnum::GetValueAsString(Run.Phase), Run.Round + 1, Run.Misses);
			LastPhase = Run.Phase;
		}
		if (Run.Phase == ETributePhase::Passed || Run.Phase == ETributePhase::Failed)
		{
			bEnded = true;
			OnTributeEnded.Broadcast(Run.Phase == ETributePhase::Passed);
		}
	}

	// The spin: two turns, easing in and out, so the pools sweep the valley before the call.
	float SpinYaw = 0.f;
	if (bDown && Run.Phase == ETributePhase::Spin)
	{
		SpinYaw = 720.f * FMath::SmoothStep(0.f, 1.f, Run.PhaseTime / Run.SpinTime);
	}
	Ring->SetRelativeRotation(FRotator(0.f, SpinYaw, 0.f));

	// A new note of the call: its tone.
	const int32 Lit = bDown ? Run.GetLitSector() : INDEX_NONE;
	if (Lit != INDEX_NONE && Lit != LastLitSector)
	{
		UE_LOG(LogTribute, Log, TEXT("Call note: sector %d"), Lit);
		PlayTone(Lit, FTributeScale::SemitoneOf(Lit));
	}
	LastLitSector = Lit;

	UpdateSectors(DeltaSeconds);
}

void AMothership::HandlePulse(FVector GroundPoint)
{
	if (!bDown || bEnded)
	{
		return;
	}
	// A pulse outside every pool isn't an answer at all; only a pulse in the wrong pool is a miss.
	const int32 Sector = SectorAt(GroundPoint);
	if (Sector == INDEX_NONE)
	{
		return;
	}
	HandleAnswer(Run.Answer(Sector), Sector);
}

void AMothership::HandleAnswer(ETributeAnswer Result, int32 Sector)
{
	switch (Result)
	{
	case ETributeAnswer::Correct:
	case ETributeAnswer::CallAnswered:
		UE_LOG(LogTribute, Log, TEXT("Answered sector %d%s"), Sector, Result == ETributeAnswer::CallAnswered ? TEXT(": call complete") : TEXT(""));
		FlashLeft = FlashSeconds;
		FlashSector = Sector;
		bFlashMiss = false;
		NoteAnswered(Sector, FTributeScale::SemitoneOf(Sector));
		break;
	case ETributeAnswer::Missed:
	case ETributeAnswer::LeftBehind:
		UE_LOG(LogTribute, Log, TEXT("Missed (%s): %d of %d"), Sector == INDEX_NONE ? TEXT("too slow") : TEXT("wrong sector"), Run.Misses, Run.MissesAllowed);
		FlashLeft = FlashSeconds * 2.f;
		FlashSector = INDEX_NONE;
		bFlashMiss = true;
		CallMissed(Run.Misses);
		break;
	default:
		break;
	}
}

FLinearColor AMothership::RoundColour() const
{
	if (RoundColours.Num() == 0)
	{
		return FLinearColor::White;
	}
	const FLinearColor Current = RoundColours[FMath::Clamp(Run.Round, 0, RoundColours.Num() - 1)];
	if (Run.Phase == ETributePhase::NewColour)
	{
		const FLinearColor Next = RoundColours[FMath::Clamp(Run.Round + 1, 0, RoundColours.Num() - 1)];
		return FLinearColor::LerpUsingHSV(Current, Next, FMath::Clamp(Run.PhaseTime / Run.NewColourTime, 0.f, 1.f));
	}
	return Current;
}

void AMothership::SetSector(int32 Sector, const FLinearColor& Colour, float PoolBrightness)
{
	if (SectorTints.IsValidIndex(Sector))
	{
		SectorTints[Sector]->SetVectorParameterValue(TEXT("Color"), Colour);
	}
	USpotLightComponent* Pool = Pools[Sector];
	Pool->SetIntensity(PoolBrightness);
	Pool->SetLightColor(Colour);
	Pool->SetVisibility(PoolBrightness > 0.f);
}

void AMothership::UpdateSectors(float DeltaSeconds)
{
	FlashLeft = FMath::Max(FlashLeft - DeltaSeconds, 0.f);
	const FLinearColor Base = RoundColour();
	const bool bActive = bDown && !bDeparting;
	const int32 Lit = bActive ? Run.GetLitSector() : INDEX_NONE;

	for (int32 Sector = 0; Sector < Sectors.Num(); ++Sector)
	{
		// Before arrival and after leaving: dark. While it's down: every sector glows faintly and marks its pool.
		FLinearColor Colour = bActive ? Base * 0.2f : FLinearColor(0.01f, 0.01f, 0.01f);
		float Pool = bActive ? DimPool : 0.f;

		if (bActive)
		{
			switch (Run.Phase)
			{
			case ETributePhase::Catch:
				if (Sector == Lit)
				{
					Colour = Base * 10.f;
					Pool = LitPool;
				}
				break;
			case ETributePhase::Stop:
			case ETributePhase::Passed:
				// The ring holds, all lit: the answer was heard.
				Colour = Base * 4.f;
				Pool = LitPool * 0.3f;
				break;
			case ETributePhase::Failed:
				Colour = FLinearColor(0.3f, 0.02f, 0.02f);
				Pool = 0.f;
				break;
			default:
				break;
			}
			if (FlashLeft > 0.f)
			{
				if (bFlashMiss)
				{
					Colour = FLinearColor(4.f, 0.1f, 0.05f);
				}
				else if (Sector == FlashSector)
				{
					Colour = FLinearColor(10.f, 10.f, 10.f);
					Pool = LitPool;
				}
			}
		}
		SetSector(Sector, Colour, Pool);
	}
}

int32 AMothership::SectorAt(const FVector& GroundPoint) const
{
	for (int32 Sector = 0; Sector < Pools.Num(); ++Sector)
	{
		if (FVector::Dist2D(GroundPoint, GetPoolCentre(Sector)) <= PoolRadius)
		{
			return Sector;
		}
	}
	return INDEX_NONE;
}

FVector AMothership::GetPoolCentre(int32 Sector) const
{
	if (!Pools.IsValidIndex(Sector))
	{
		return FVector::ZeroVector;
	}
	const FVector Light = Pools[Sector]->GetComponentLocation();
	return FVector(Light.X, Light.Y, DownLocation.Z - HoverHeight);
}

FVector AMothership::GetDockPoint() const
{
	return GetActorLocation() - FVector(0.f, 0.f, 800.f);
}
