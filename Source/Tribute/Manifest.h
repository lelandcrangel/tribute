// The mothership's order for the night, and the clock to its arrival.
//
// FManifestRow is one row of DT_Manifests: how many cows, people and named objects the ship wants, and how
// long the raid lasts. FManifest counts what's been taken against it. FRaidClock counts down to the
// ship. Both are plain structs so the rules can be tested without a world (Tests/ManifestTests.cpp).

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "LiftableComponent.h"
#include "Manifest.generated.h"

/** One night's order, as a row of DT_Manifests. */
USTRUCT(BlueprintType)
struct TRIBUTE_API FManifestRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Manifest", meta = (ClampMin = "0"))
	int32 Cows = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Manifest", meta = (ClampMin = "0"))
	int32 People = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Manifest", meta = (ClampMin = "0"))
	int32 NamedObjects = 1;

	/** What the named object is called on the HUD (the scarecrow, the tractor, the preacher). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Manifest")
	FText NamedObjectName = NSLOCTEXT("Tribute", "DefaultNamedObject", "The scarecrow");

	/** Seconds from the start of the raid to the ship's arrival. Seven minutes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Manifest", meta = (ClampMin = "10"))
	float RaidSeconds = 420.f;
};

/** What's been taken against the order. */
USTRUCT(BlueprintType)
struct TRIBUTE_API FManifest
{
	GENERATED_BODY()

	static FManifest FromRow(const FManifestRow& Row);

	/**
	 * Counts one thing taken aboard.
	 * @return True if this one filled its line (the line was short and now isn't).
	 */
	bool Count(ETargetKind Kind);

	int32 Wanted(ETargetKind Kind) const;
	int32 Taken(ETargetKind Kind) const;
	bool IsLineFilled(ETargetKind Kind) const { return Taken(Kind) >= Wanted(Kind); }
	bool IsComplete() const;

	/** How much of the order is filled, 0 to 1, counting only what was asked for. The debrief scores this. */
	float FillFraction() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manifest")
	int32 CowsWanted = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manifest")
	int32 PeopleWanted = 2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manifest")
	int32 NamedWanted = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manifest")
	int32 CowsTaken = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manifest")
	int32 PeopleTaken = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manifest")
	int32 NamedTaken = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Manifest")
	FText NamedObjectName;
};

/** Counts down to the ship's arrival. */
USTRUCT(BlueprintType)
struct TRIBUTE_API FRaidClock
{
	GENERATED_BODY()

	void Start(float Seconds) { SecondsLeft = Seconds; bArrived = false; }

	/**
	 * Runs the clock on.
	 * @return True on the one step where the ship arrives.
	 */
	bool Advance(float DeltaSeconds);

	/** The countdown as minutes and seconds, rounded up, so it reads 0:01 until the last moment. */
	FString ToMinutesSeconds() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clock")
	float SecondsLeft = 420.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clock")
	bool bArrived = false;
};
