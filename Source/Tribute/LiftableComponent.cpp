#include "LiftableComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

ULiftableComponent::ULiftableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULiftableComponent::BeginLift(ASaucerPawn* Saucer)
{
	AActor* Owner = GetOwner();
	if (bBeingLifted || !Owner)
	{
		return;
	}
	bBeingLifted = true;

	// While it rises, the saucer moves it by hand: nothing should push it, and it shouldn't walk or fall.
	bHadCollision = Owner->GetActorEnableCollision();
	Owner->SetActorEnableCollision(false);

	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
	{
		bWasSimulatingPhysics = Root->IsSimulatingPhysics();
		Root->SetSimulatePhysics(false);
	}
	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		Character->GetCharacterMovement()->StopMovementImmediately();
		Character->GetCharacterMovement()->DisableMovement();
	}

	OnLiftStarted.Broadcast(Saucer);
}

void ULiftableComponent::EndLift(ASaucerPawn* Saucer)
{
	AActor* Owner = GetOwner();
	if (!bBeingLifted || !Owner)
	{
		return;
	}
	bBeingLifted = false;

	// Give it back to the world so it drops from wherever the beam let go.
	Owner->SetActorEnableCollision(bHadCollision);

	if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Owner->GetRootComponent()))
	{
		if (bWasSimulatingPhysics)
		{
			Root->SetSimulatePhysics(true);
		}
	}
	if (ACharacter* Character = Cast<ACharacter>(Owner))
	{
		Character->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
	}

	OnLiftDropped.Broadcast(Saucer);
}

void ULiftableComponent::CompleteLift(ASaucerPawn* Saucer)
{
	if (!bBeingLifted)
	{
		return;
	}
	bBeingLifted = false;
	OnLiftCompleted.Broadcast(Saucer);
}
