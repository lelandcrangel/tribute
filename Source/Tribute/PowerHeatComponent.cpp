#include "PowerHeatComponent.h"

UPowerHeatComponent::UPowerHeatComponent()
{
	// The saucer steps it in its own Tick, so the order against hover and beam is fixed.
	PrimaryComponentTick.bCanEverTick = false;
}

float UPowerHeatComponent::Step(float DeltaSeconds, const FPowerHeatInputs& Inputs)
{
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
	if (Inputs.bDrawingPower && !bPowerEmpty)
	{
		HeatRate += DrawHeat;
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

	// --- Power: once it's empty the saucer is down, and nothing refills it ---
	if (bPowerEmpty)
	{
		return 0.f;
	}

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

	float Drawn = 0.f;
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
