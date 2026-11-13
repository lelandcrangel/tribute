#include "Transformer.h"

#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Tribute.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	const FLinearColor ChargedColour(0.9f, 0.75f, 0.2f);
	const FLinearColor EmptyColour(0.08f, 0.08f, 0.08f);
}

ATransformer::ATransformer()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> TintableMaterial(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

	// The root sits on the ground; the engine shapes are centred, so each part is raised by half its height.
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Base"));

	// A pole 30 cm thick and 8 m tall.
	Pole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pole"));
	Pole->SetupAttachment(RootComponent);
	if (CylinderMesh.Succeeded())
	{
		Pole->SetStaticMesh(CylinderMesh.Object);
	}
	Pole->SetRelativeScale3D(FVector(0.3f, 0.3f, 8.f));
	Pole->SetRelativeLocation(FVector(0.f, 0.f, 400.f));

	// The can hangs off the pole near the top.
	Can = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Can"));
	Can->SetupAttachment(RootComponent);
	if (CubeMesh.Succeeded())
	{
		Can->SetStaticMesh(CubeMesh.Object);
	}
	Can->SetRelativeScale3D(FVector(0.7f, 0.7f, 0.9f));
	Can->SetRelativeLocation(FVector(50.f, 0.f, 690.f));
	Can->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	YardLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("YardLight"));
	YardLight->SetupAttachment(RootComponent);
	YardLight->SetRelativeLocation(FVector(0.f, 0.f, 760.f));
	YardLight->SetMobility(EComponentMobility::Movable);
	YardLight->SetIntensityUnits(ELightUnits::Candelas);
	YardLight->SetIntensity(300.f);
	YardLight->SetAttenuationRadius(2500.f);
	// Sodium orange, like a farm's yard light.
	YardLight->SetLightColor(FLinearColor(1.f, 0.6f, 0.25f));

	if (TintableMaterial.Succeeded())
	{
		GreyBoxMaterial = TintableMaterial.Object;
	}
}

void ATransformer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	Charge = MaxCharge;
	TintCan();
}

void ATransformer::BeginPlay()
{
	Super::BeginPlay();
	Charge = MaxCharge;
	bBlackedOut = false;
	TintCan();
}

bool ATransformer::CanFeed(const FVector& SaucerLocation) const
{
	return Charge > 0.f && FVector::Dist2D(SaucerLocation, GetActorLocation()) <= DrawRadius;
}

void ATransformer::Feed(float Amount)
{
	if (Amount <= 0.f || Charge <= 0.f)
	{
		return;
	}
	if (!bBlackedOut)
	{
		BlackOut();
	}

	Charge = FMath::Max(Charge - Amount, 0.f);
	if (Charge <= 0.f)
	{
		UE_LOG(LogTribute, Log, TEXT("%s is empty"), *GetName());
		TintCan();
		Emptied();
	}
}

void ATransformer::BlackOut()
{
	bBlackedOut = true;
	YardLight->SetVisibility(false);
	for (AActor* Light : FarmLights)
	{
		if (!Light)
		{
			continue;
		}
		TArray<ULightComponent*> Lights;
		Light->GetComponents(Lights);
		for (ULightComponent* Component : Lights)
		{
			Component->SetVisibility(false);
		}
	}
	UE_LOG(LogTribute, Log, TEXT("%s blacked out its farm"), *GetName());
	BlackedOut();
}

void ATransformer::TintCan()
{
	if (!GreyBoxMaterial || !Can)
	{
		return;
	}
	if (!CanTint)
	{
		CanTint = UMaterialInstanceDynamic::Create(GreyBoxMaterial, this);
		Can->SetMaterial(0, CanTint);
	}
	CanTint->SetVectorParameterValue(TEXT("Color"), Charge > 0.f ? ChargedColour : EmptyColour);
}
