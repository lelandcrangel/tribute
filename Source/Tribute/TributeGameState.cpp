#include "TributeGameState.h"

#include "Engine/DataTable.h"
#include "Tribute.h"

ATributeGameState::ATributeGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	ManifestTable = TSoftObjectPtr<UDataTable>(FSoftObjectPath(TEXT("/Game/Tribute/Data/DT_Manifests.DT_Manifests")));
}

void ATributeGameState::BeginPlay()
{
	Super::BeginPlay();

	FManifestRow Order;
	if (const UDataTable* Table = ManifestTable.LoadSynchronous())
	{
		if (const FManifestRow* Row = Table->FindRow<FManifestRow>(ManifestRowName, TEXT("Manifest")))
		{
			Order = *Row;
		}
		else
		{
			UE_LOG(LogTribute, Warning, TEXT("DT_Manifests has no row %s; using the default order"), *ManifestRowName.ToString());
		}
	}
	else
	{
		UE_LOG(LogTribute, Warning, TEXT("No manifest table; using the default order"));
	}

	Manifest = FManifest::FromRow(Order);
	Clock.Start(Order.RaidSeconds);
	UE_LOG(LogTribute, Log, TEXT("Manifest: %d cows, %d people, %s; the ship comes in %s"), Manifest.CowsWanted,
		Manifest.PeopleWanted, *Manifest.NamedObjectName.ToString(), *Clock.ToMinutesSeconds());
}

void ATributeGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (Clock.Advance(DeltaSeconds))
	{
		UE_LOG(LogTribute, Log, TEXT("The ship is here. Manifest %.0f%% filled"), Manifest.FillFraction() * 100.f);
		OnShipArrived.Broadcast();
	}
}

void ATributeGameState::CountTakenAboard(ETargetKind Kind)
{
	const bool bFilledLine = Manifest.Count(Kind);
	UE_LOG(LogTribute, Log, TEXT("Manifest: %d/%d cows, %d/%d people, %d/%d named%s"), Manifest.CowsTaken, Manifest.CowsWanted,
		Manifest.PeopleTaken, Manifest.PeopleWanted, Manifest.NamedTaken, Manifest.NamedWanted,
		bFilledLine ? TEXT(" (a line filled)") : TEXT(""));
	OnManifestCounted.Broadcast(Kind, bFilledLine);
}
