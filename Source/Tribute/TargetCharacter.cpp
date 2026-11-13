#include "TargetCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "LiftableComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TargetAIController.h"
#include "UObject/ConstructorHelpers.h"

ATargetCharacter::ATargetCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Placed or spawned, every cow and person gets the wander-and-flee controller.
	AIControllerClass = ATargetAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Face the way it walks, turning at an animal's pace rather than snapping.
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 300.f, 0.f);

	Liftable = CreateDefaultSubobject<ULiftableComponent>(TEXT("Liftable"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	GreyBoxBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GreyBoxBody"));
	GreyBoxBody->SetupAttachment(GetCapsuleComponent());
	GreyBoxBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GreyBoxBody->SetCanEverAffectNavigation(false);
	if (CubeMesh.Succeeded())
	{
		GreyBoxBody->SetStaticMesh(CubeMesh.Object);
	}

	GreyBoxHead = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GreyBoxHead"));
	GreyBoxHead->SetupAttachment(GetCapsuleComponent());
	GreyBoxHead->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GreyBoxHead->SetCanEverAffectNavigation(false);
	if (SphereMesh.Succeeded())
	{
		GreyBoxHead->SetStaticMesh(SphereMesh.Object);
	}
}

void ATargetCharacter::SetGreyBoxShape(float CapsuleRadius, float CapsuleHalfHeight, const FVector& BodySize,
	const FVector& HeadOffset, float HeadSize)
{
	GetCapsuleComponent()->InitCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

	// The engine cube and sphere are 100 cm across. Offsets are measured from the feet.
	const float Feet = -CapsuleHalfHeight;
	GreyBoxBody->SetRelativeScale3D(BodySize / 100.f);
	GreyBoxBody->SetRelativeLocation(FVector(0.f, 0.f, Feet + BodySize.Z * 0.5f));
	GreyBoxHead->SetRelativeScale3D(FVector(HeadSize / 100.f));
	GreyBoxHead->SetRelativeLocation(FVector(HeadOffset.X, HeadOffset.Y, Feet + HeadOffset.Z));
}

void ATargetCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// The engine's basic shape material takes a "Color", which is all a grey box needs to tell kinds apart.
	for (UStaticMeshComponent* Shape : { GreyBoxBody.Get(), GreyBoxHead.Get() })
	{
		if (Shape && Shape->GetStaticMesh())
		{
			if (UMaterialInstanceDynamic* Tint = Shape->CreateDynamicMaterialInstance(0))
			{
				Tint->SetVectorParameterValue(TEXT("Color"), GreyBoxColor);
			}
		}
	}
}

ACowCharacter::ACowCharacter()
{
	Liftable->Kind = ETargetKind::Cow;
	// About 2 m long and 1.1 m high, with the head out in front.
	SetGreyBoxShape(60.f, 70.f, FVector(200.f, 90.f, 110.f), FVector(115.f, 0.f, 115.f), 50.f);
	GreyBoxColor = FLinearColor(0.85f, 0.78f, 0.65f);

	WanderRadius = 1000.f;
	WanderSpeed = 100.f;
	PauseTime = FVector2D(3.f, 8.f);
	PanicRadius = 1200.f;
	FleeSpeed = 380.f;
	CalmDownTime = 3.f;
}

APersonCharacter::APersonCharacter()
{
	Liftable->Kind = ETargetKind::Person;
	// 1.8 m standing, head on top.
	SetGreyBoxShape(35.f, 90.f, FVector(45.f, 45.f, 150.f), FVector(0.f, 0.f, 165.f), 30.f);
	GreyBoxColor = FLinearColor(0.35f, 0.55f, 0.95f);

	WanderRadius = 2000.f;
	WanderSpeed = 150.f;
	PauseTime = FVector2D(1.f, 4.f);
	PanicRadius = 1800.f;
	FleeSpeed = 550.f;
	CalmDownTime = 5.f;
}
