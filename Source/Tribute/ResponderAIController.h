// Drives a responder: toward the point under the saucer while it's on the job, and home once it's called
// off, where it disappears. It re-plans twice a second, which is plenty for something slower than the saucer.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ResponderAIController.generated.h"

UCLASS()
class TRIBUTE_API AResponderAIController : public AAIController
{
	GENERATED_BODY()

public:
	AResponderAIController();

	virtual void Tick(float DeltaSeconds) override;

private:
	float ReplanLeft = 0.f;
};
