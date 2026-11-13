// The saucer's two meters. Power drains while it hovers and faster while the beam is on; it refills over a
// transformer. Heat is how much the town has noticed: it climbs while the beam is on (faster when the beam
// holds a person) and while the saucer draws from a transformer, and falls once the saucer has lain low for
// a few seconds. Heat brings the police at 45% and the helicopter at 80%.
//
// The saucer calls Step() every frame with what it's doing; nothing here reads the world, so the rules can
// be tested on their own (see Tests/PowerHeatTests.cpp).

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PowerHeatComponent.generated.h"

/** How hard the town is looking. Each level brings a responder. */
UENUM(BlueprintType)
enum class EHeatLevel : uint8
{
	Calm,
	Police,
	Helicopter,
};

/** What the saucer is doing this frame, as far as Power and Heat care. */
USTRUCT(BlueprintType)
struct FPowerHeatInputs
{
	GENERATED_BODY()

	/** Hovering costs Power. False once the saucer is down. */
	UPROPERTY(BlueprintReadWrite, Category = "Power and Heat")
	bool bHovering = true;

	UPROPERTY(BlueprintReadWrite, Category = "Power and Heat")
	bool bBeamOn = false;

	/** The beam is holding a person, which the town notices more than a cow. */
	UPROPERTY(BlueprintReadWrite, Category = "Power and Heat")
	bool bLiftingPerson = false;

	/** The saucer is over a transformer that still has charge. */
	UPROPERTY(BlueprintReadWrite, Category = "Power and Heat")
	bool bDrawingPower = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeatLevelChanged, EHeatLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPowerEmpty);

UCLASS(ClassGroup = (Tribute), meta = (BlueprintSpawnableComponent))
class TRIBUTE_API UPowerHeatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPowerHeatComponent();

	/**
	 * Moves Power and Heat on by DeltaSeconds of doing what Inputs says.
	 * @return How much Power was drawn from a transformer, so it can take that from its charge.
	 */
	float Step(float DeltaSeconds, const FPowerHeatInputs& Inputs);

	// --- Power ---

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power", meta = (ClampMin = "1"))
	float MaxPower = 100.f;

	/** Power lost per second just staying up. At this rate a full tank lasts about four and a half minutes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power")
	float HoverDrain = 0.35f;

	/** Extra Power lost per second while the beam is on. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power")
	float BeamDrain = 2.f;

	/** Power gained per second over a transformer. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Power")
	float RefillRate = 15.f;

	// --- Heat (0 to 100) ---

	/** Heat gained per second while the beam is on. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat")
	float BeamHeat = 2.5f;

	/** Extra Heat per second while the beam holds a person. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat")
	float PersonHeat = 4.f;

	/** Heat per second while drawing from a transformer (a farm going dark gets noticed). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat")
	float DrawHeat = 3.f;

	/** Seconds with no beam and no drawing before Heat starts to fall. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat")
	float LieLowDelay = 4.f;

	/** Heat lost per second once lying low. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat")
	float CoolRate = 2.f;

	/** Heat at which the police come. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat")
	float PoliceHeat = 45.f;

	/** Heat at which the searchlight helicopter comes. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat")
	float HelicopterHeat = 80.f;

	/** How far below a threshold Heat must fall before that level is called off, so it doesn't flicker. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Heat")
	float LevelHysteresis = 5.f;

	// --- Reading it ---

	UFUNCTION(BlueprintPure, Category = "Power and Heat")
	float GetPower() const { return Power; }

	UFUNCTION(BlueprintPure, Category = "Power and Heat")
	float GetPowerFraction() const { return Power / MaxPower; }

	UFUNCTION(BlueprintPure, Category = "Power and Heat")
	float GetHeat() const { return Heat; }

	/** Heat from 0 to 1, for the rim colour and the HUD. */
	UFUNCTION(BlueprintPure, Category = "Power and Heat")
	float GetHeatFraction() const { return Heat / 100.f; }

	UFUNCTION(BlueprintPure, Category = "Power and Heat")
	EHeatLevel GetHeatLevel() const { return HeatLevel; }

	/** True once Power has hit zero. It stays true: the saucer is down. */
	UFUNCTION(BlueprintPure, Category = "Power and Heat")
	bool IsPowerEmpty() const { return bPowerEmpty; }

	/** For tests and debugging: start from a given Heat (the level follows on the next Step). */
	void SetHeatForTesting(float NewHeat) { Heat = FMath::Clamp(NewHeat, 0.f, 100.f); }

	/** The police or the helicopter were called, or called off. Responders spawn and leave on this. */
	UPROPERTY(BlueprintAssignable, Category = "Power and Heat")
	FOnHeatLevelChanged OnHeatLevelChanged;

	/** Power hit zero. The saucer goes down and the run ends. */
	UPROPERTY(BlueprintAssignable, Category = "Power and Heat")
	FOnPowerEmpty OnPowerEmpty;

private:
	void UpdateHeatLevel();

	float Power = 100.f;
	float Heat = 0.f;
	float QuietTime = 0.f;
	EHeatLevel HeatLevel = EHeatLevel::Calm;
	bool bPowerEmpty = false;
};
