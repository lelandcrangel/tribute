// Marks an actor as something the saucer's beam can lift: a cow, a person, or the named object.
// The saucer finds targets by this component, so any actor with one can go up the beam.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LiftableComponent.generated.h"

class ASaucerPawn;

/** What a lifted thing counts as on the manifest. */
UENUM(BlueprintType)
enum class ETargetKind : uint8
{
	Cow,
	Person,
	NamedObject,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLiftEvent, ASaucerPawn*, Saucer);

UCLASS(ClassGroup = (Tribute), meta = (BlueprintSpawnableComponent))
class TRIBUTE_API ULiftableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULiftableComponent();

	/** What this counts as on the manifest. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lift")
	ETargetKind Kind = ETargetKind::Cow;

	/** Multiplies the saucer's lift speed. Heavier things (a tractor) rise slower. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lift", meta = (ClampMin = "0.1"))
	float LiftSpeedScale = 1.f;

	/** The beam caught this and it started to rise. Targets stop walking here. */
	UPROPERTY(BlueprintAssignable, Category = "Lift")
	FOnLiftEvent OnLiftStarted;

	/** The beam let go before it reached the hull, and it falls. Targets get up and run here. */
	UPROPERTY(BlueprintAssignable, Category = "Lift")
	FOnLiftEvent OnLiftDropped;

	/** It reached the hull and is about to be taken out of the world. */
	UPROPERTY(BlueprintAssignable, Category = "Lift")
	FOnLiftEvent OnLiftCompleted;

	/** True while a beam holds it. */
	UFUNCTION(BlueprintPure, Category = "Lift")
	bool IsBeingLifted() const { return bBeingLifted; }

	/** Called by the saucer. Turns off the owner's collision, physics and walking while it rises. */
	void BeginLift(ASaucerPawn* Saucer);

	/** Called by the saucer when the beam lets go. Gives the owner its collision back so it falls. */
	void EndLift(ASaucerPawn* Saucer);

	/** Called by the saucer when it reaches the hull. */
	void CompleteLift(ASaucerPawn* Saucer);

private:
	bool bBeingLifted = false;
	bool bHadCollision = true;
	bool bWasSimulatingPhysics = false;
};
