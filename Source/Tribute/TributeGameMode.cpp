#include "TributeGameMode.h"

#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Responder.h"
#include "SaucerPawn.h"
#include "Tribute.h"

ATributeGameMode::ATributeGameMode()
{
	// BP_Saucer and BP_PoliceCar replace these once they exist, through a GameMode Blueprint.
	DefaultPawnClass = ASaucerPawn::StaticClass();
	PoliceCarClass = APoliceCar::StaticClass();
}

void ATributeGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// The saucer exists now: listen to its Heat to send responders in and call them off.
	if (ASaucerPawn* Saucer = NewPlayer ? Cast<ASaucerPawn>(NewPlayer->GetPawn()) : nullptr)
	{
		Saucer->PowerHeat->OnHeatLevelChanged.AddDynamic(this, &ATributeGameMode::HandleHeatLevelChanged);
	}
}

void ATributeGameMode::HandleHeatLevelChanged(EHeatLevel NewLevel)
{
	// One police car at a time: it comes at the police level, stays through the helicopter, and goes when
	// the town calms down. If Heat climbs again after it's been called off, a new one comes.
	if (NewLevel != EHeatLevel::Calm)
	{
		if ((!PoliceCar.IsValid() || PoliceCar->IsLeaving()) && PoliceCarClass)
		{
			FActorSpawnParameters Spawn;
			Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			PoliceCar = GetWorld()->SpawnActor<AResponder>(PoliceCarClass, FindPoliceStart(), Spawn);
			UE_LOG(LogTribute, Log, TEXT("Heat reached the police: sent %s"), PoliceCar.IsValid() ? *PoliceCar->GetName() : TEXT("nothing"));
		}
	}
	else if (PoliceCar.IsValid())
	{
		PoliceCar->Leave();
	}
}

FTransform ATributeGameMode::FindPoliceStart() const
{
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(PoliceStartTag))
		{
			return It->GetActorTransform();
		}
	}

	// No marked start: come from 100 m down the valley, so it's never on top of the saucer.
	UE_LOG(LogTribute, Warning, TEXT("No actor tagged %s; the police start 100 m from the saucer"), *PoliceStartTag.ToString());
	const APawn* Saucer = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
	const FVector From = Saucer ? Saucer->GetActorLocation() : FVector::ZeroVector;
	return FTransform(FVector(From.X - 10000.f, From.Y, 100.f));
}
