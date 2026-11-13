#include "Responder.h"

#include "Components/CapsuleComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "ResponderAIController.h"
#include "Tribute.h"
#include "UObject/ConstructorHelpers.h"

AResponder::AResponder()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AResponderAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Drive like a vehicle: face the way it's going and turn wide, not on the spot.
	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* CarMovement = GetCharacterMovement();
	CarMovement->bOrientRotationToMovement = true;
	CarMovement->RotationRate = FRotator(0.f, 120.f, 0.f);
	CarMovement->MaxAcceleration = 700.f;
	CarMovement->BrakingDecelerationWalking = 1200.f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TintableMaterial(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	GreyBoxBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GreyBoxBody"));
	GreyBoxBody->SetupAttachment(GetCapsuleComponent());
	GreyBoxBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GreyBoxBody->SetCanEverAffectNavigation(false);
	if (CubeMesh.Succeeded())
	{
		GreyBoxBody->SetStaticMesh(CubeMesh.Object);
	}
	if (TintableMaterial.Succeeded())
	{
		GreyBoxMaterial = TintableMaterial.Object;
	}

	// The searchlight is aimed in world space every frame, so it ignores the body's turning.
	Searchlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Searchlight"));
	Searchlight->SetupAttachment(GetCapsuleComponent());
	Searchlight->SetUsingAbsoluteRotation(true);
	Searchlight->SetMobility(EComponentMobility::Movable);
	Searchlight->SetIntensityUnits(ELightUnits::Candelas);
	Searchlight->SetIntensity(0.f);
	Searchlight->SetOuterConeAngle(9.f);
	Searchlight->SetInnerConeAngle(5.f);
	Searchlight->SetLightColor(FLinearColor(1.f, 0.95f, 0.85f));
	Searchlight->SetVisibility(false);
}

void AResponder::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	Searchlight->SetAttenuationRadius(SearchlightRange + 1500.f);
	if (GreyBoxMaterial)
	{
		UMaterialInstanceDynamic* Tint = UMaterialInstanceDynamic::Create(GreyBoxMaterial, GreyBoxBody);
		Tint->SetVectorParameterValue(TEXT("Color"), GreyBoxColor);
		GreyBoxBody->SetMaterial(0, Tint);
	}
}

void AResponder::BeginPlay()
{
	Super::BeginPlay();
	Home = GetActorLocation();
	GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;
	UE_LOG(LogTribute, Log, TEXT("%s is on its way"), *GetName());
	Arrived();
}

void AResponder::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool bWasLighting = LitTarget.IsValid();
	const bool bLighting = AimSearchlight();
	if (bLighting && !bWasLighting)
	{
		UE_LOG(LogTribute, Log, TEXT("%s has the saucer in its searchlight"), *GetName());
		SpottedSaucer();
	}
	else if (!bLighting && bWasLighting)
	{
		UE_LOG(LogTribute, Log, TEXT("%s lost the saucer"), *GetName());
		LostSaucer();
	}
}

bool AResponder::AimSearchlight()
{
	APawn* Saucer = bLeaving ? nullptr : UGameplayStatics::GetPlayerPawn(this, 0);
	const FVector LightLocation = Searchlight->GetComponentLocation();

	bool bCanSee = false;
	if (Saucer && FVector::Dist(LightLocation, Saucer->GetActorLocation()) <= SearchlightRange)
	{
		// Barns and hills hide the saucer from the light; cows and other cars don't.
		FCollisionQueryParams Params(SCENE_QUERY_STAT(ResponderSearchlight), false, this);
		Params.AddIgnoredActor(Saucer);
		FCollisionObjectQueryParams Scenery;
		Scenery.AddObjectTypesToQuery(ECC_WorldStatic);
		Scenery.AddObjectTypesToQuery(ECC_WorldDynamic);
		FHitResult Hit;
		bCanSee = !GetWorld()->LineTraceSingleByObjectType(Hit, LightLocation, Saucer->GetActorLocation(), Scenery, Params);
	}

	if (bCanSee)
	{
		Searchlight->SetWorldRotation((Saucer->GetActorLocation() - LightLocation).Rotation());
		Searchlight->SetIntensity(SearchlightBrightness);
		Searchlight->SetVisibility(true);
		LitTarget = Saucer;
		return true;
	}

	Searchlight->SetIntensity(0.f);
	Searchlight->SetVisibility(false);
	LitTarget.Reset();
	return false;
}

void AResponder::Leave()
{
	if (bLeaving)
	{
		return;
	}
	bLeaving = true;
	LitTarget.Reset();
	Searchlight->SetVisibility(false);
	UE_LOG(LogTribute, Log, TEXT("%s was called off"), *GetName());
	CalledOff();
}

APoliceCar::APoliceCar()
{
	// About 4.6 m long and 1.4 m high, in a capsule that fits its width.
	GetCapsuleComponent()->InitCapsuleSize(110.f, 75.f);
	GreyBoxBody->SetRelativeScale3D(FVector(4.6f, 1.9f, 1.4f));
	GreyBoxBody->SetRelativeLocation(FVector(0.f, 0.f, -75.f + 70.f));
	GreyBoxColor = FLinearColor(0.04f, 0.06f, 0.2f);

	Searchlight->SetRelativeLocation(FVector(60.f, 60.f, 90.f));

	// The light bar: red on one side, blue on the other, taking turns.
	RedLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RedLight"));
	RedLight->SetupAttachment(GetCapsuleComponent());
	RedLight->SetRelativeLocation(FVector(0.f, -50.f, 85.f));
	BlueLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BlueLight"));
	BlueLight->SetupAttachment(GetCapsuleComponent());
	BlueLight->SetRelativeLocation(FVector(0.f, 50.f, 85.f));
	for (UPointLightComponent* Light : { RedLight.Get(), BlueLight.Get() })
	{
		Light->SetMobility(EComponentMobility::Movable);
		Light->SetIntensityUnits(ELightUnits::Candelas);
		Light->SetIntensity(1500.f);
		Light->SetAttenuationRadius(1500.f);
		Light->SetCastShadows(false);
	}
	RedLight->SetLightColor(FLinearColor(1.f, 0.05f, 0.05f));
	BlueLight->SetLightColor(FLinearColor(0.1f, 0.2f, 1.f));
	BlueLight->SetVisibility(false);
}

void APoliceCar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FlashTime += DeltaSeconds;
	if (FlashTime >= FlashInterval)
	{
		FlashTime = 0.f;
		RedLight->ToggleVisibility();
		BlueLight->ToggleVisibility();
	}
}
