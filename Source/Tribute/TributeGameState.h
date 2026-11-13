// The night's shared state: the manifest the ship sent and what's been taken against it, and the clock to
// the ship's arrival. The GameMode counts each lift in here; the HUD and the debrief read it.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Manifest.h"
#include "TributeGameState.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnManifestCounted, ETargetKind, Kind, bool, bFilledLine);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShipArrived);

UCLASS()
class TRIBUTE_API ATributeGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ATributeGameState();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** The orders the ship can send. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Manifest")
	TSoftObjectPtr<UDataTable> ManifestTable;

	/** Which order this night uses. Without the table or the row, the defaults are used (3 cows, 2 people, 1 object, 7 minutes). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Manifest")
	FName ManifestRowName = TEXT("Default");

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Manifest")
	FManifest Manifest;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Manifest")
	FRaidClock Clock;

	/** Counts one thing taken aboard against the manifest. */
	UFUNCTION(BlueprintCallable, Category = "Manifest")
	void CountTakenAboard(ETargetKind Kind);

	/** Something was counted; bFilledLine if it completed its line. The HUD flashes and plays a sound on this. */
	UPROPERTY(BlueprintAssignable, Category = "Manifest")
	FOnManifestCounted OnManifestCounted;

	/** The clock ran out: the ship is here. The run moves on to the arrival. */
	UPROPERTY(BlueprintAssignable, Category = "Manifest")
	FOnShipArrived OnShipArrived;
};
