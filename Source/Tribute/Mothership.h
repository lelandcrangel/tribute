// The mothership: a dark, angular hull the size of the sky, with one ring of 8 sectors on its underside.
// When the clock runs out it comes down over the valley (the arrival), then plays the tribute: it lights its
// sectors in a call, each throwing a pool of light on the ground, and the saucer answers by pulsing its beam
// inside each pool in turn. It's our own design: one ring, no tiered underside of lights, no light board.
//
// The rules are FTributeRun (TributeRules.h); this actor shows them and feeds them the saucer's pulses.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TributeRules.h"
#include "Mothership.generated.h"

class ASaucerPawn;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USpotLightComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTributeEnded, bool, bPassed);

UCLASS()
class TRIBUTE_API AMothership : public AActor
{
	GENERATED_BODY()

public:
	AMothership();

	virtual void Tick(float DeltaSeconds) override;

	// --- Parts ---

	/** Two dark slabs crossed at 45 degrees: an eight-pointed hull 200 m across. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mothership")
	TObjectPtr<UStaticMeshComponent> HullA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mothership")
	TObjectPtr<UStaticMeshComponent> HullB;

	/** The ring under the hull. It turns during the spin; the sectors and their pools turn with it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mothership")
	TObjectPtr<USceneComponent> Ring;

	/** The 8 sector panels, in order around the ring. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mothership")
	TArray<TObjectPtr<UStaticMeshComponent>> Sectors;

	/** Each sector's light on the ground: the pool the saucer answers in. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mothership")
	TArray<TObjectPtr<USpotLightComponent>> Pools;

	/** The ship's glow over the whole valley once it's down: the only time the valley is all lit. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mothership")
	TObjectPtr<USpotLightComponent> UnderLight;

	// --- Tuning ---

	/** How far out from the centre the sectors sit, in cm. Adjacent pools are about 46 m apart. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mothership")
	float RingRadius = 6000.f;

	/** How close to a pool's centre a pulse has to land to count for that sector, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mothership")
	float PoolRadius = 1300.f;

	/** How high above the valley floor it hangs once it's down, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mothership")
	float HoverHeight = 12000.f;

	/** How high it starts, out of sight above the clouds, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mothership")
	float StartHeight = 90000.f;

	/** How long the descent takes, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mothership")
	float ArrivalTime = 14.f;

	/** The ring's colour for each round; it changes in the new-colour phase. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mothership")
	TArray<FLinearColor> RoundColours;

	/** Seed for the calls. 0 picks one from the clock, so each night is different. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mothership")
	int32 Seed = 0;

	// --- Running it ---

	/** Starts the descent over the valley. The tribute begins when it's down. */
	UFUNCTION(BlueprintCallable, Category = "Mothership")
	void BeginArrival(ASaucerPawn* Saucer);

	/** Leaves without the saucer: rises back into the sky. */
	UFUNCTION(BlueprintCallable, Category = "Mothership")
	void Depart();

	/** Which sector's pool a ground point is in, or INDEX_NONE. */
	UFUNCTION(BlueprintPure, Category = "Mothership")
	int32 SectorAt(const FVector& GroundPoint) const;

	/** Where a sector's pool is on the ground. */
	UFUNCTION(BlueprintPure, Category = "Mothership")
	FVector GetPoolCentre(int32 Sector) const;

	/** Where the saucer docks: the middle of the hull's underside. */
	UFUNCTION(BlueprintPure, Category = "Mothership")
	FVector GetDockPoint() const;

	UFUNCTION(BlueprintPure, Category = "Mothership")
	bool IsDown() const { return bDown; }

	/** The tribute's state, for the HUD and the debrief. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Mothership")
	FTributeRun Run;

	/** The tribute is over: passed (dock) or failed (left behind). */
	UPROPERTY(BlueprintAssignable, Category = "Mothership")
	FOnTributeEnded OnTributeEnded;

	// --- Hooks for the Blueprint's sounds and effects ---

	/** A sector lit in the call: play its tone (one MetaSound per sector; Semitone is its pitch). */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mothership", meta = (DisplayName = "Play Tone"))
	void PlayTone(int32 Sector, int32 Semitone);

	/** The saucer answered a note right: the same tone, and a burst at the pool. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mothership", meta = (DisplayName = "Note Answered"))
	void NoteAnswered(int32 Sector, int32 Semitone);

	/** A wrong sector or too slow: a low hull resonance, and the call plays again. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mothership", meta = (DisplayName = "Call Missed"))
	void CallMissed(int32 MissesSoFar);

	/** The ship broke through the clouds: the first pulse of its presence. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Mothership", meta = (DisplayName = "Arrival Started"))
	void ArrivalStarted();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandlePulse(FVector GroundPoint);

	void HandleAnswer(ETributeAnswer Result, int32 Sector);
	void UpdateSectors(float DeltaSeconds);
	void SetSector(int32 Sector, const FLinearColor& Colour, float PoolBrightness);
	FLinearColor RoundColour() const;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> SectorTints;

	UPROPERTY(EditDefaultsOnly, Category = "Mothership")
	TObjectPtr<UMaterialInterface> GlowMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Mothership")
	TObjectPtr<UMaterialInterface> HullMaterial;

	TWeakObjectPtr<ASaucerPawn> Saucer;
	FVector DownLocation = FVector::ZeroVector;
	FVector UpLocation = FVector::ZeroVector;
	float MoveTime = 0.f;
	bool bArriving = false;
	bool bDeparting = false;
	bool bDown = false;
	bool bEnded = false;
	int32 LastLitSector = INDEX_NONE;
	ETributePhase LastPhase = ETributePhase::NotStarted;
	/** Seconds left on the flash after an answer, and which sector (or all, for a miss). */
	float FlashLeft = 0.f;
	int32 FlashSector = INDEX_NONE;
	bool bFlashMiss = false;
};
