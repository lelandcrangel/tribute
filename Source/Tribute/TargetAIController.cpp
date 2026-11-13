#include "TargetAIController.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LiftableComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "SaucerPawn.h"
#include "TargetCharacter.h"

namespace
{
	/** How often it looks for the beam, in seconds. Often enough to feel alert, rarely enough to be cheap. */
	constexpr float LookAroundInterval = 0.2f;
}

ATargetAIController::ATargetAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

ATargetCharacter* ATargetAIController::GetTarget() const
{
	return Cast<ATargetCharacter>(GetPawn());
}

void ATargetAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ATargetCharacter* Target = GetTarget();
	if (!Target)
	{
		return;
	}
	Home = Target->GetActorLocation();
	Target->Liftable->OnLiftStarted.AddDynamic(this, &ATargetAIController::HandleLiftStarted);
	Target->Liftable->OnLiftDropped.AddDynamic(this, &ATargetAIController::HandleLiftDropped);

	// Stagger the first pause so a herd doesn't all set off on the same frame.
	StartPause();
	PauseLeft *= FMath::FRand();
	LookAroundLeft = FMath::FRand() * LookAroundInterval;
}

void ATargetAIController::OnUnPossess()
{
	if (ATargetCharacter* Target = GetTarget())
	{
		Target->Liftable->OnLiftStarted.RemoveDynamic(this, &ATargetAIController::HandleLiftStarted);
		Target->Liftable->OnLiftDropped.RemoveDynamic(this, &ATargetAIController::HandleLiftDropped);
	}
	Super::OnUnPossess();
}

void ATargetAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ATargetCharacter* Target = GetTarget();
	if (!Target || Mood == EMood::Lifted)
	{
		return;
	}

	if (Mood == EMood::Falling)
	{
		// Once it hits the ground, it runs from where the beam dropped it.
		if (Target->GetCharacterMovement()->IsMovingOnGround())
		{
			StartFleeing(ThreatPoint);
		}
		return;
	}

	LookAroundLeft -= DeltaSeconds;
	if (LookAroundLeft <= 0.f)
	{
		LookAroundLeft = LookAroundInterval;

		FVector Threat;
		if (FindThreat(Threat))
		{
			if (Mood != EMood::Fleeing)
			{
				StartFleeing(Threat);
			}
			ThreatPoint = Threat;
			CalmLeft = Target->CalmDownTime;
		}

		// A finished or failed dash while still frightened: run again.
		if (Mood == EMood::Fleeing && GetMoveStatus() == EPathFollowingStatus::Idle)
		{
			RunFrom(ThreatPoint);
		}
	}

	if (Mood == EMood::Fleeing)
	{
		CalmLeft -= DeltaSeconds;
		if (CalmLeft <= 0.f)
		{
			StopMovement();
			SetSpeed(Target->WanderSpeed);
			Target->CalmedDown();
			StartPause();
		}
	}
	else if (Mood == EMood::Pausing)
	{
		PauseLeft -= DeltaSeconds;
		if (PauseLeft <= 0.f)
		{
			StartWander();
		}
	}
}

void ATargetAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	// A move replaced by a newer one reports as aborted; only a real arrival or failure ends a wander.
	if (Mood == EMood::Wandering && !Result.IsInterrupted())
	{
		StartPause();
	}
}

bool ATargetAIController::FindThreat(FVector& OutThreatPoint) const
{
	const ATargetCharacter* Target = GetTarget();
	const ASaucerPawn* Saucer = Cast<ASaucerPawn>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!Target || !Saucer || !Saucer->IsBeamOn())
	{
		return false;
	}

	const FVector BeamPoint = Saucer->GetBeamGroundPoint();
	if (FVector::Dist2D(Target->GetActorLocation(), BeamPoint) > Target->PanicRadius)
	{
		return false;
	}
	OutThreatPoint = BeamPoint;
	return true;
}

void ATargetAIController::StartPause()
{
	Mood = EMood::Pausing;
	if (const ATargetCharacter* Target = GetTarget())
	{
		PauseLeft = FMath::FRandRange(Target->PauseTime.X, Target->PauseTime.Y);
	}
}

void ATargetAIController::StartWander()
{
	ATargetCharacter* Target = GetTarget();
	UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation Destination;
	if (!Target || !Navigation || !Navigation->GetRandomReachablePointInRadius(Home, Target->WanderRadius, Destination))
	{
		StartPause();
		return;
	}

	Mood = EMood::Wandering;
	SetSpeed(Target->WanderSpeed);
	if (MoveToLocation(Destination.Location, 50.f) == EPathFollowingRequestResult::Failed)
	{
		StartPause();
	}
}

void ATargetAIController::StartFleeing(const FVector& From)
{
	ATargetCharacter* Target = GetTarget();
	if (!Target)
	{
		return;
	}
	Mood = EMood::Fleeing;
	ThreatPoint = From;
	CalmLeft = Target->CalmDownTime;
	SetSpeed(Target->FleeSpeed);
	Target->Panicked();
	RunFrom(From);
}

void ATargetAIController::RunFrom(const FVector& From)
{
	const ATargetCharacter* Target = GetTarget();
	if (!Target)
	{
		return;
	}

	// Straight away from the beam; if it's right on top, any way will do.
	const FVector Location = Target->GetActorLocation();
	FVector Away = (Location - From).GetSafeNormal2D();
	if (Away.IsNearlyZero())
	{
		Away = FVector(FMath::VRand().X, FMath::VRand().Y, 0.f).GetSafeNormal();
	}
	const FVector Destination = Location + Away * Target->FleeDistance;

	if (MoveToLocation(Destination, 100.f, false, true, true) != EPathFollowingRequestResult::Failed)
	{
		return;
	}

	// Straight away was off the navigable ground (a wall, the valley's edge): take any open spot near it.
	UNavigationSystemV1* Navigation = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation Fallback;
	if (Navigation && Navigation->GetRandomReachablePointInRadius(Location, Target->FleeDistance, Fallback))
	{
		MoveToLocation(Fallback.Location, 100.f, false);
	}
}

void ATargetAIController::SetSpeed(float Speed)
{
	if (ATargetCharacter* Target = GetTarget())
	{
		Target->GetCharacterMovement()->MaxWalkSpeed = Speed;
	}
}

void ATargetAIController::HandleLiftStarted(ASaucerPawn* Saucer)
{
	// The saucer moves it now; walking would fight the beam.
	Mood = EMood::Lifted;
	StopMovement();
}

void ATargetAIController::HandleLiftDropped(ASaucerPawn* Saucer)
{
	Mood = EMood::Falling;
	if (const ATargetCharacter* Target = GetTarget())
	{
		ThreatPoint = Saucer ? Saucer->GetBeamGroundPoint() : Target->GetActorLocation();
	}
}
