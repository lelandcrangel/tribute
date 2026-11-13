#include "ResponderAIController.h"

#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "Responder.h"
#include "SaucerPawn.h"
#include "Tribute.h"

namespace
{
	/** Seconds between new routes. */
	constexpr float ReplanInterval = 0.5f;

	/** How close to home counts as gone, in cm. */
	constexpr float HomeRadius = 400.f;
}

AResponderAIController::AResponderAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AResponderAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AResponder* Responder = Cast<AResponder>(GetPawn());
	if (!Responder)
	{
		return;
	}

	if (Responder->IsLeaving() && FVector::Dist2D(Responder->GetActorLocation(), Responder->GetHome()) <= HomeRadius)
	{
		UE_LOG(LogTribute, Log, TEXT("%s is gone"), *Responder->GetName());
		Responder->Destroy();
		return;
	}

	ReplanLeft -= DeltaSeconds;
	if (ReplanLeft > 0.f)
	{
		return;
	}
	ReplanLeft = ReplanInterval;

	if (Responder->IsLeaving())
	{
		MoveToLocation(Responder->GetHome(), HomeRadius * 0.5f, false, true, true);
		return;
	}

	// Head for the ground under the saucer; the navmesh keeps it on fields and roads and around barns.
	if (const ASaucerPawn* Saucer = Cast<ASaucerPawn>(UGameplayStatics::GetPlayerPawn(this, 0)))
	{
		MoveToLocation(Saucer->GetBeamGroundPoint(), Responder->ChaseDistance, false, true, true);
	}
}
