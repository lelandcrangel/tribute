#include "SaucerPawn.h"

#include "Tribute.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "UObject/ConstructorHelpers.h"

ASaucerPawn::ASaucerPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// The saucer never turns as a whole; only the hull spins and leans, so the controls stay screen-relative.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(250.f);
	Collision->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	RootComponent = Collision;

	Hull = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hull"));
	Hull->SetupAttachment(Collision);
	Hull->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// A flattened engine cylinder, 5 m across, until BP_Saucer gives it the real hull.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DiscMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (DiscMesh.Succeeded())
	{
		Hull->SetStaticMesh(DiscMesh.Object);
	}
	Hull->SetRelativeScale3D(FVector(5.f, 5.f, 0.5f));

	BeamLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("BeamLight"));
	BeamLight->SetupAttachment(Collision);
	BeamLight->SetMobility(EComponentMobility::Movable);
	BeamLight->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	BeamLight->SetIntensityUnits(ELightUnits::Candelas);
	BeamLight->SetIntensity(0.f);
	BeamLight->SetLightColor(FLinearColor(0.6f, 0.9f, 1.f));
	BeamLight->SetVisibility(false);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(Collision);
	// A high three-quarter view that doesn't turn with anything: up on screen is always +X in the world.
	// 50 m back and a little lower than 45 degrees, so a fleeing herd stays in frame and the beam's column
	// shows from the side instead of being hidden under the hull.
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->SetRelativeRotation(FRotator(-48.f, 0.f, 0.f));
	CameraBoom->TargetArmLength = 5000.f;
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 5.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->SetFieldOfView(60.f);

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
	Movement->MaxSpeed = 1500.f;
	Movement->Acceleration = 3000.f;
	Movement->Deceleration = 4000.f;
}

void ASaucerPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	FitBeamLight();
}

void ASaucerPawn::FitBeamLight()
{
	const float ConeDegrees = FMath::RadiansToDegrees(FMath::Atan2(BeamRadius, HoverHeight));
	BeamLight->SetOuterConeAngle(ConeDegrees);
	BeamLight->SetInnerConeAngle(ConeDegrees * 0.7f);
	BeamLight->SetAttenuationRadius(HoverHeight * 2.5f);
}

// --- Input ---

void ASaucerPawn::BuildDefaultInput()
{
	if (!MoveAction)
	{
		MoveAction = NewObject<UInputAction>(this, TEXT("IA_Move_Default"));
		MoveAction->ValueType = EInputActionValueType::Axis2D;
	}
	if (!BeamAction)
	{
		BeamAction = NewObject<UInputAction>(this, TEXT("IA_Beam_Default"));
		BeamAction->ValueType = EInputActionValueType::Boolean;
	}
	if (InputMapping)
	{
		return;
	}
	InputMapping = NewObject<UInputMappingContext>(this, TEXT("IMC_Default_Default"));

	// The Move action is X = right, Y = forward. A key gives (1, 0), so forward and back keys are
	// swizzled onto Y, and back and left are negated.
	auto MapMoveKey = [this](const FKey& Key, bool bOntoForward, bool bNegate)
	{
		FEnhancedActionKeyMapping& Mapping = InputMapping->MapKey(MoveAction, Key);
		if (bOntoForward)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierSwizzleAxis>(InputMapping));
		}
		if (bNegate)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(InputMapping));
		}
	};
	MapMoveKey(EKeys::W, true, false);
	MapMoveKey(EKeys::S, true, true);
	MapMoveKey(EKeys::D, false, false);
	MapMoveKey(EKeys::A, false, true);
	MapMoveKey(EKeys::Up, true, false);
	MapMoveKey(EKeys::Down, true, true);
	MapMoveKey(EKeys::Right, false, false);
	MapMoveKey(EKeys::Left, false, true);

	FEnhancedActionKeyMapping& StickMapping = InputMapping->MapKey(MoveAction, EKeys::Gamepad_Left2D);
	StickMapping.Modifiers.Add(NewObject<UInputModifierDeadZone>(InputMapping));

	InputMapping->MapKey(BeamAction, EKeys::SpaceBar);
	InputMapping->MapKey(BeamAction, EKeys::Gamepad_FaceButton_Bottom);
}

void ASaucerPawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	BuildDefaultInput();
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMapping, 0);
		}
	}
}

void ASaucerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	BuildDefaultInput();
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASaucerPawn::Move);
		Input->BindAction(BeamAction, ETriggerEvent::Started, this, &ASaucerPawn::StartBeam);
		Input->BindAction(BeamAction, ETriggerEvent::Completed, this, &ASaucerPawn::StopBeam);
	}
}

void ASaucerPawn::Move(const FInputActionValue& Value)
{
	const FVector2D Direction = Value.Get<FVector2D>();
	AddMovementInput(FVector::ForwardVector, Direction.Y);
	AddMovementInput(FVector::RightVector, Direction.X);
}

void ASaucerPawn::StartBeam()
{
	bBeamHeld = true;
	BeamHeldTime = 0.f;
	BeamStarted();
}

void ASaucerPawn::StopBeam()
{
	if (!bBeamHeld)
	{
		return;
	}
	bBeamHeld = false;
	DropLift();
	BeamStopped();

	// A short tap is a pulse: it answers the mothership's call rather than lifting.
	if (BeamHeldTime <= PulseMaxHoldTime)
	{
		PulseFlashLeft = PulseFlashTime;
		const FVector GroundPoint = GetBeamGroundPoint();
		UE_LOG(LogTribute, Log, TEXT("Beam pulse at %s"), *GroundPoint.ToCompactString());
		OnBeamPulsed.Broadcast(GroundPoint);
		BeamPulsed(GroundPoint);
	}
}

// --- Every frame ---

void ASaucerPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateHover(DeltaSeconds);

	if (bBeamHeld)
	{
		BeamHeldTime += DeltaSeconds;
		// Only a held beam catches things, so a pulse never yanks a cow off its feet.
		if (BeamHeldTime > PulseMaxHoldTime && !LiftTarget.IsValid())
		{
			if (ULiftableComponent* Target = FindTargetInBeam())
			{
				StartLift(Target);
			}
		}
	}

	UpdateLift(DeltaSeconds);
	UpdateHull(DeltaSeconds);
	UpdateBeamLight(DeltaSeconds);
}

void ASaucerPawn::UpdateHover(float DeltaSeconds)
{
	const FVector Location = GetActorLocation();

	// Look for the ground (terrain, roads, buildings) straight below. Movable scenery counts too
	// (WorldDynamic), but cows and people (Pawn) and props (PhysicsBody) don't.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SaucerHover), false, this);
	if (LiftTarget.IsValid())
	{
		Params.AddIgnoredActor(LiftTarget->GetOwner());
	}
	FCollisionObjectQueryParams GroundTypes;
	GroundTypes.AddObjectTypesToQuery(ECC_WorldStatic);
	GroundTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
	FHitResult Hit;
	const FVector End = Location - FVector(0.f, 0.f, HoverHeight * 4.f);
	if (GetWorld()->LineTraceSingleByObjectType(Hit, Location, End, GroundTypes, Params))
	{
		GroundZ = Hit.ImpactPoint.Z;
		bHasGround = true;
	}
	if (!bHasGround)
	{
		return;
	}

	FVector NewLocation = Location;
	NewLocation.Z = FMath::FInterpTo(Location.Z, GroundZ + HoverHeight, DeltaSeconds, HoverFollowSpeed);
	SetActorLocation(NewLocation, true);
}

void ASaucerPawn::UpdateHull(float DeltaSeconds)
{
	HoverTime += DeltaSeconds;
	HullYaw = FMath::Fmod(HullYaw + HullSpinRate * DeltaSeconds, 360.f);

	// Lean toward where it's going: nose down going forward, a side down going sideways.
	const FVector Velocity = GetVelocity();
	const float TopSpeed = FMath::Max(Movement->GetMaxSpeed(), 1.f);
	const FRotator TargetTilt(-Velocity.X / TopSpeed * MaxTilt, 0.f, Velocity.Y / TopSpeed * MaxTilt);
	HullTilt = FMath::RInterpTo(HullTilt, TargetTilt, DeltaSeconds, TiltFollowSpeed);

	const float Bob = FMath::Sin(HoverTime * BobRate * UE_TWO_PI) * BobHeight;
	const FQuat Rotation = HullTilt.Quaternion() * FRotator(0.f, HullYaw, 0.f).Quaternion();
	Hull->SetRelativeLocationAndRotation(FVector(0.f, 0.f, Bob), Rotation);
}

void ASaucerPawn::UpdateBeamLight(float DeltaSeconds)
{
	float Brightness = 0.f;
	if (PulseFlashLeft > 0.f)
	{
		PulseFlashLeft -= DeltaSeconds;
		Brightness = BeamBrightness * 3.f;
	}
	else if (bBeamHeld)
	{
		Brightness = BeamBrightness;
	}
	BeamLight->SetIntensity(Brightness);
	BeamLight->SetVisibility(Brightness > 0.f);
}

// --- The beam and the lift ---

FVector ASaucerPawn::GetBeamGroundPoint() const
{
	const FVector Location = GetActorLocation();
	return FVector(Location.X, Location.Y, bHasGround ? GroundZ : Location.Z - HoverHeight);
}

ULiftableComponent* ASaucerPawn::FindTargetInBeam() const
{
	const FVector Top = GetActorLocation();
	const FVector Ground = GetBeamGroundPoint();

	// A column the width of the beam, from a little below the ground up to the saucer.
	const float HalfHeight = (Top.Z - Ground.Z) * 0.5f + 100.f;
	const FVector Centre = FVector(Top.X, Top.Y, (Top.Z + Ground.Z) * 0.5f);

	FCollisionObjectQueryParams ObjectTypes;
	ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
	ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);
	ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SaucerBeam), false, this);

	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(Overlaps, Centre, FQuat::Identity, ObjectTypes,
		FCollisionShape::MakeCapsule(BeamRadius, HalfHeight), Params);

	ULiftableComponent* Nearest = nullptr;
	float NearestDistance = TNumericLimits<float>::Max();
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Actor = Overlap.GetActor();
		ULiftableComponent* Liftable = Actor ? Actor->FindComponentByClass<ULiftableComponent>() : nullptr;
		if (!Liftable || Liftable->IsBeingLifted())
		{
			continue;
		}
		const float Distance = FVector::Dist2D(Actor->GetActorLocation(), Ground);
		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			Nearest = Liftable;
		}
	}
	return Nearest;
}

void ASaucerPawn::StartLift(ULiftableComponent* Target)
{
	LiftTarget = Target;
	Target->BeginLift(this);
	LiftStarted(Target->GetOwner());
}

void ASaucerPawn::UpdateLift(float DeltaSeconds)
{
	ULiftableComponent* Liftable = LiftTarget.Get();
	AActor* Target = Liftable ? Liftable->GetOwner() : nullptr;
	if (!Target)
	{
		LiftTarget.Reset();
		return;
	}

	// Rise straight up, drift into the middle of the beam, and turn slowly on the way.
	const FVector Saucer = GetActorLocation();
	FVector Location = Target->GetActorLocation();
	Location.X = FMath::FInterpTo(Location.X, Saucer.X, DeltaSeconds, LiftCentreSpeed);
	Location.Y = FMath::FInterpTo(Location.Y, Saucer.Y, DeltaSeconds, LiftCentreSpeed);
	Location.Z = FMath::Min(Location.Z + LiftSpeed * Liftable->LiftSpeedScale * DeltaSeconds, Saucer.Z);

	FRotator Rotation = Target->GetActorRotation();
	Rotation.Yaw += LiftSpinRate * DeltaSeconds;
	Target->SetActorLocationAndRotation(Location, Rotation);

	if (Saucer.Z - Location.Z <= TakeAboardDistance)
	{
		TakeAboard();
	}
}

void ASaucerPawn::DropLift()
{
	ULiftableComponent* Liftable = LiftTarget.Get();
	LiftTarget.Reset();
	if (Liftable && Liftable->GetOwner())
	{
		Liftable->EndLift(this);
		LiftDropped(Liftable->GetOwner());
	}
}

void ASaucerPawn::TakeAboard()
{
	ULiftableComponent* Liftable = LiftTarget.Get();
	LiftTarget.Reset();
	AActor* Target = Liftable ? Liftable->GetOwner() : nullptr;
	if (!Target)
	{
		return;
	}

	const ETargetKind Kind = Liftable->Kind;
	UE_LOG(LogTribute, Log, TEXT("Took %s aboard (%s)"), *Target->GetName(), *UEnum::GetValueAsString(Kind));
	Liftable->CompleteLift(this);
	OnTargetLifted.Broadcast(Target, Kind);
	TakenAboard(Target, Kind);
	Target->Destroy();
}
