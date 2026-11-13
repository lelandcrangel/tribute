#include "LiftableProp.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "LiftableComponent.h"
#include "UObject/ConstructorHelpers.h"

ALiftableProp::ALiftableProp()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Mesh->SetSimulatePhysics(true);

	// A 1 m engine cube until a Blueprint gives it a real mesh.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}

	Liftable = CreateDefaultSubobject<ULiftableComponent>(TEXT("Liftable"));
	Liftable->Kind = ETargetKind::NamedObject;
}
