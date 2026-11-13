// A plain object the beam can lift: the named object (a scarecrow, a tractor), and grey boxes for testing.
// Cows and people are characters with their own class; this is for things that don't walk.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiftableProp.generated.h"

class UStaticMeshComponent;
class ULiftableComponent;

UCLASS()
class TRIBUTE_API ALiftableProp : public AActor
{
	GENERATED_BODY()

public:
	ALiftableProp();

	/** What it looks like. It simulates physics, so a dropped prop falls and tumbles. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/** Lets the saucer lift it, and says what it counts as on the manifest. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Prop")
	TObjectPtr<ULiftableComponent> Liftable;
};
