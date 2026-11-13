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

/** How the night ended. */
UENUM(BlueprintType)
enum class ERunEnding : uint8
{
	Docked,     // the tribute was answered and the saucer rode up into the hull
	LeftBehind, // three missed calls: the ship left without it
	SaucerDown, // Power ran out
};

/** The debrief's numbers. */
USTRUCT(BlueprintType)
struct TRIBUTE_API FRunResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Debrief")
	ERunEnding Ending = ERunEnding::SaucerDown;

	UPROPERTY(BlueprintReadOnly, Category = "Debrief")
	FManifest Manifest;

	/** Notes answered right, and calls missed, in the tribute (0 and 0 if it never got there). */
	UPROPERTY(BlueprintReadOnly, Category = "Debrief")
	int32 NotesRight = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Debrief")
	int32 Misses = 0;

	/** Heat when the night ended: how stirred up the town was left. */
	UPROPERTY(BlueprintReadOnly, Category = "Debrief")
	float Heat = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Debrief")
	int32 Score = 0;

	/** The tribute's accuracy, 0 to 1: right notes over right notes plus misses. */
	float Accuracy() const { return (NotesRight + Misses) > 0 ? static_cast<float>(NotesRight) / (NotesRight + Misses) : 0.f; }

	/**
	 * The score: up to 1000 for the manifest, 250 for the named object, up to 500 for the tribute's accuracy,
	 * 500 for docking, less 2 a point of Heat left behind. Never below zero.
	 */
	static int32 ScoreFor(ERunEnding Ending, const FManifest& Manifest, int32 NotesRight, int32 Misses, float Heat);
};

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

	/** Stops or restarts the clock: it waits while the title card is up, so the night starts at a full 7:00. */
	UFUNCTION(BlueprintCallable, Category = "Manifest")
	void HoldClock(bool bHold) { bClockHeld = bHold; }

	/** For testing: brings the ship in now (`ke * SkipToArrival` in the console). */
	UFUNCTION(BlueprintCallable, Category = "Manifest")
	void SkipToArrival() { Clock.SecondsLeft = FMath::Min(Clock.SecondsLeft, 1.f); }

	/** Counts one thing taken aboard against the manifest. */
	UFUNCTION(BlueprintCallable, Category = "Manifest")
	void CountTakenAboard(ETargetKind Kind);

	/** Something was counted; bFilledLine if it completed its line. The HUD flashes and plays a sound on this. */
	UPROPERTY(BlueprintAssignable, Category = "Manifest")
	FOnManifestCounted OnManifestCounted;

	/** The clock ran out: the ship is here. The run moves on to the arrival. */
	UPROPERTY(BlueprintAssignable, Category = "Manifest")
	FOnShipArrived OnShipArrived;

private:
	bool bClockHeld = false;
};
