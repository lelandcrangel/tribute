// A cow or a person in the valley: something that wanders, panics when the beam comes near, and can be
// lifted. ATargetAIController does the wandering and fleeing; this class holds the tuning and the look.
//
// Until BP_Cow and BP_Person give them real meshes, each one is drawn as grey-box shapes (a body and a
// head) tinted by kind, so a herd is readable from the saucer's height.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TargetCharacter.generated.h"

class ULiftableComponent;
class UStaticMeshComponent;

UCLASS(Abstract)
class TRIBUTE_API ATargetCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATargetCharacter();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** Lets the saucer lift it, and says what it counts as on the manifest. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
	TObjectPtr<ULiftableComponent> Liftable;

	/** Grey-box body. BP_Cow and BP_Person hide it once they have a real mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target|Grey Box")
	TObjectPtr<UStaticMeshComponent> GreyBoxBody;

	/** Grey-box head, at the front, so you can see which way it's going. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target|Grey Box")
	TObjectPtr<UStaticMeshComponent> GreyBoxHead;

	/** The grey-box shapes' colour. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target|Grey Box")
	FLinearColor GreyBoxColor = FLinearColor::White;

	// --- Behaviour (read by ATargetAIController) ---

	/** How far from where it started it wanders, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target|Behaviour")
	float WanderRadius = 1200.f;

	/** Walking speed while wandering, in cm per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target|Behaviour")
	float WanderSpeed = 120.f;

	/** Shortest and longest pause between wanders (grazing, standing about), in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target|Behaviour")
	FVector2D PauseTime = FVector2D(2.f, 6.f);

	/** A lit beam this close (to where it meets the ground) makes it panic, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target|Behaviour")
	float PanicRadius = 1400.f;

	/** Running speed while fleeing, in cm per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target|Behaviour")
	float FleeSpeed = 420.f;

	/** How far each dash away from the beam goes, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target|Behaviour")
	float FleeDistance = 1500.f;

	/** How long it keeps running after the beam goes off or out of range, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Target|Behaviour")
	float CalmDownTime = 3.f;

	// --- Hooks for the Blueprint's sounds and effects (called by the controller) ---

	/** It saw the beam and started to run: a moo, a shout. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Target", meta = (DisplayName = "Panicked"))
	void Panicked();

	/** It stopped running and went back to wandering. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Target", meta = (DisplayName = "Calmed Down"))
	void CalmedDown();

protected:
	/** Fits the capsule and the grey-box shapes to a body of this size, all in cm. */
	void SetGreyBoxShape(float CapsuleRadius, float CapsuleHalfHeight, const FVector& BodySize, const FVector& HeadOffset, float HeadSize);
};

/** A cow: slow, close to home, and a bit slow to notice. Counts as a cow on the manifest. */
UCLASS()
class TRIBUTE_API ACowCharacter : public ATargetCharacter
{
	GENERATED_BODY()

public:
	ACowCharacter();
};

/** A person: wanders further, notices sooner and runs faster. Counts as a person on the manifest. */
UCLASS()
class TRIBUTE_API APersonCharacter : public ATargetCharacter
{
	GENERATED_BODY()

public:
	APersonCharacter();
};
