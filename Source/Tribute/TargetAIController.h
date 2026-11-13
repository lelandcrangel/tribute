// Drives a cow or a person: wander about home, pause, wander again; run from the saucer's beam when it
// comes near; go still while lifted; and bolt when dropped. The tuning lives on ATargetCharacter.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "TargetAIController.generated.h"

class ASaucerPawn;
class ATargetCharacter;

UCLASS()
class TRIBUTE_API ATargetAIController : public AAIController
{
	GENERATED_BODY()

public:
	ATargetAIController();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	/** What it's doing now. */
	enum class EMood : uint8
	{
		Pausing,   // standing about until PauseLeft runs out
		Wandering, // walking to a random point near home
		Fleeing,   // running from ThreatPoint
		Lifted,    // in the beam; the saucer moves it
		Falling,   // dropped by the beam, waiting to land
	};

	EMood GetMood() const { return Mood; }

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

private:
	UFUNCTION()
	void HandleLiftStarted(ASaucerPawn* Saucer);

	UFUNCTION()
	void HandleLiftDropped(ASaucerPawn* Saucer);

	/** Where a lit beam meets the ground, if it's close enough to panic over. */
	bool FindThreat(FVector& OutThreatPoint) const;

	void StartPause();
	void StartWander();
	void StartFleeing(const FVector& From);
	void RunFrom(const FVector& From);
	void SetSpeed(float Speed);

	ATargetCharacter* GetTarget() const;

	EMood Mood = EMood::Pausing;
	FVector Home = FVector::ZeroVector;
	FVector ThreatPoint = FVector::ZeroVector;
	float PauseLeft = 0.f;
	float CalmLeft = 0.f;
	float LookAroundLeft = 0.f;
};
