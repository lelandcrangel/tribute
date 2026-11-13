#include "PowerHeatComponent.h"

UPowerHeatComponent::UPowerHeatComponent()
{
	// The saucer steps it in its own Tick, so the order against hover and beam is fixed.
	PrimaryComponentTick.bCanEverTick = false;
}

float UPowerHeatComponent::Step(float DeltaSeconds, const FPowerHeatInputs& Inputs)
{
	// --- Power: once it's empty the saucer is down, and nothing refills it ---
	float Drawn = 0.f;
	if (!bPowerEmpty)
	{
		float Drain = 0.f;
		if (Inputs.bHovering)
		{
			Drain += HoverDrain;
		}
		if (Inputs.bBeamOn)
		{
			Drain += BeamDrain;
		}
		const float AfterDrain = Power - Drain * DeltaSeconds;

		if (Inputs.bDrawingPower)
		{
			// Only take what fits in the tank, so a transformer isn't emptied into a full saucer.
			Drawn = FMath::Clamp(RefillRate * DeltaSeconds, 0.f, FMath::Max(MaxPower - AfterDrain, 0.f));
		}

		Power = FMath::Clamp(AfterDrain + Drawn, 0.f, MaxPower);
		if (Power <= 0.f)
		{
			bPowerEmpty = true;
			OnPowerEmpty.Broadcast();
		}
	}

	// --- Heat: anything that gets noticed resets the lie-low clock ---
	float HeatRate = 0.f;
	if (Inputs.bBeamOn)
	{
		HeatRate += BeamHeat;
		if (Inputs.bLiftingPerson)
		{
			HeatRate += PersonHeat;
		}
	}
	// Drawing makes Heat in proportion to how hard the farm is being drained: hanging over a transformer
	// with a full tank only tops up the hover drain, which nobody notices.
	const float FullDraw = RefillRate * DeltaSeconds;
	if (Drawn > 0.f && FullDraw > 0.f)
	{
		HeatRate += DrawHeat * (Drawn / FullDraw);
	}

	// Being lit by the police keeps the town looking, so it can't be waited out under a spotlight.
	if (Inputs.bInSpotlight)
	{
		HeatRate += SpotlightHeat;
	}

	if (HeatRate > 0.f)
	{
		Heat += HeatRate * DeltaSeconds;
		QuietTime = 0.f;
	}
	else
	{
		QuietTime += DeltaSeconds;
		if (QuietTime >= LieLowDelay)
		{
			Heat -= CoolRate * DeltaSeconds;
		}
	}
	Heat = FMath::Clamp(Heat, 0.f, 100.f);
	UpdateHeatLevel();

	return Drawn;
}

void UPowerHeatComponent::UpdateHeatLevel()
{
	// Rising, a level starts at its threshold. Falling, it ends a little below, so it doesn't flicker.
	EHeatLevel NewLevel = HeatLevel;
	if (Heat >= HelicopterHeat)
	{
		NewLevel = EHeatLevel::Helicopter;
	}
	else if (Heat >= PoliceHeat)
	{
		NewLevel = (HeatLevel == EHeatLevel::Helicopter && Heat >= HelicopterHeat - LevelHysteresis)
			? EHeatLevel::Helicopter : EHeatLevel::Police;
	}
	else if (HeatLevel == EHeatLevel::Calm || Heat < PoliceHeat - LevelHysteresis)
	{
		NewLevel = EHeatLevel::Calm;
	}
	else
	{
		NewLevel = EHeatLevel::Police;
	}

	if (NewLevel != HeatLevel)
	{
		HeatLevel = NewLevel;
		OnHeatLevelChanged.Broadcast(HeatLevel);
	}
}
