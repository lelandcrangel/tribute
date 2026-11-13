// Tests for the Power and Heat rules: drain and refill rates, the police and helicopter thresholds, lying
// low, and the saucer going down. Run with `Automation RunTests Tribute` or from Session Frontend.

#include "Misc/AutomationTest.h"
#include "PowerHeatComponent.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace PowerHeatTests
{
	constexpr EAutomationTestFlags PowerHeatTestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	UPowerHeatComponent* MakeMeters()
	{
		return NewObject<UPowerHeatComponent>(GetTransientPackage());
	}

	/** Steps the meters for Seconds in tenth-of-a-second frames, the way a game would. */
	float Run(UPowerHeatComponent* Meters, float Seconds, const FPowerHeatInputs& Inputs)
	{
		float Drawn = 0.f;
		const int32 Frames = FMath::RoundToInt(Seconds / 0.1f);
		for (int32 Frame = 0; Frame < Frames; ++Frame)
		{
			Drawn += Meters->Step(0.1f, Inputs);
		}
		return Drawn;
	}

	FPowerHeatInputs Hovering()
	{
		return FPowerHeatInputs();
	}

	FPowerHeatInputs Beaming(bool bPerson = false)
	{
		FPowerHeatInputs Inputs;
		Inputs.bBeamOn = true;
		Inputs.bLiftingPerson = bPerson;
		return Inputs;
	}

	FPowerHeatInputs Drawing()
	{
		FPowerHeatInputs Inputs;
		Inputs.bDrawingPower = true;
		return Inputs;
	}
}

using namespace PowerHeatTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerHeatHoverDrainsTest, "Tribute.PowerHeat.HoveringDrainsPower", PowerHeatTestFlags)
bool FPowerHeatHoverDrainsTest::RunTest(const FString& Parameters)
{
	UPowerHeatComponent* Meters = MakeMeters();
	Run(Meters, 10.f, Hovering());
	TestEqual(TEXT("Ten seconds of hovering costs ten times HoverDrain"), Meters->GetPower(), 100.f - 10.f * Meters->HoverDrain, 0.01f);
	TestEqual(TEXT("Hovering alone makes no Heat"), Meters->GetHeat(), 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerHeatBeamDrainsFasterTest, "Tribute.PowerHeat.BeamDrainsFaster", PowerHeatTestFlags)
bool FPowerHeatBeamDrainsFasterTest::RunTest(const FString& Parameters)
{
	UPowerHeatComponent* Hover = MakeMeters();
	UPowerHeatComponent* Beam = MakeMeters();
	Run(Hover, 5.f, Hovering());
	Run(Beam, 5.f, Beaming());
	TestTrue(TEXT("The beam costs more than hovering"), Beam->GetPower() < Hover->GetPower());
	TestEqual(TEXT("Beaming costs HoverDrain plus BeamDrain"), Beam->GetPower(), 100.f - 5.f * (Beam->HoverDrain + Beam->BeamDrain), 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerHeatRefillTest, "Tribute.PowerHeat.TransformerRefillsAndStopsWhenFull", PowerHeatTestFlags)
bool FPowerHeatRefillTest::RunTest(const FString& Parameters)
{
	UPowerHeatComponent* Meters = MakeMeters();
	Run(Meters, 20.f, Beaming());
	const float Low = Meters->GetPower();
	const float Drawn = Run(Meters, 2.f, Drawing());
	TestTrue(TEXT("Drawing from a transformer raises Power"), Meters->GetPower() > Low);
	TestEqual(TEXT("What was drawn is what Power gained, less the hover drain"), Meters->GetPower(), Low + Drawn - 2.f * Meters->HoverDrain, 0.01f);

	const float DrawnWhenFull = Run(Meters, 20.f, Drawing());
	TestEqual(TEXT("Power stops at the top"), Meters->GetPower(), Meters->MaxPower, 0.01f);
	TestTrue(TEXT("A full saucer doesn't empty the transformer"), DrawnWhenFull < 20.f * Meters->RefillRate * 0.5f);
	TestTrue(TEXT("Drawing power makes Heat"), Meters->GetHeat() > 0.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerHeatFullTankTest, "Tribute.PowerHeat.FullTankOverTransformerMakesLittleHeat", PowerHeatTestFlags)
bool FPowerHeatFullTankTest::RunTest(const FString& Parameters)
{
	UPowerHeatComponent* Draining = MakeMeters();
	UPowerHeatComponent* Full = MakeMeters();
	Run(Draining, 30.f, Beaming());
	Run(Draining, 0.f, Hovering());
	const float HeatBefore = Draining->GetHeat();
	Run(Draining, 2.f, Drawing());
	const float DrainingHeat = Draining->GetHeat() - HeatBefore;

	Run(Full, 2.f, Drawing());
	TestEqual(TEXT("Drawing at full rate adds DrawHeat a second"), DrainingHeat, 2.f * Draining->DrawHeat, 0.01f);
	TestTrue(TEXT("Topping off a full tank adds almost none"), Full->GetHeat() < 0.1f * 2.f * Full->DrawHeat);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerHeatPersonHeatTest, "Tribute.PowerHeat.PeopleMakeMoreHeatThanCows", PowerHeatTestFlags)
bool FPowerHeatPersonHeatTest::RunTest(const FString& Parameters)
{
	UPowerHeatComponent* Cow = MakeMeters();
	UPowerHeatComponent* Person = MakeMeters();
	Run(Cow, 4.f, Beaming(false));
	Run(Person, 4.f, Beaming(true));
	TestEqual(TEXT("A beam alone adds BeamHeat a second"), Cow->GetHeat(), 4.f * Cow->BeamHeat, 0.01f);
	TestEqual(TEXT("A person in the beam adds PersonHeat on top"), Person->GetHeat(), 4.f * (Person->BeamHeat + Person->PersonHeat), 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerHeatThresholdsTest, "Tribute.PowerHeat.PoliceAt45HelicopterAt80", PowerHeatTestFlags)
bool FPowerHeatThresholdsTest::RunTest(const FString& Parameters)
{
	UPowerHeatComponent* Meters = MakeMeters();
	TestEqual(TEXT("It starts calm"), Meters->GetHeatLevel(), EHeatLevel::Calm);

	Meters->SetHeatForTesting(44.9f);
	Meters->Step(0.f, Hovering());
	TestEqual(TEXT("Just under 45 is still calm"), Meters->GetHeatLevel(), EHeatLevel::Calm);

	Meters->SetHeatForTesting(45.f);
	Meters->Step(0.f, Hovering());
	TestEqual(TEXT("45 brings the police"), Meters->GetHeatLevel(), EHeatLevel::Police);

	Meters->SetHeatForTesting(79.9f);
	Meters->Step(0.f, Hovering());
	TestEqual(TEXT("Just under 80 is still the police"), Meters->GetHeatLevel(), EHeatLevel::Police);

	Meters->SetHeatForTesting(80.f);
	Meters->Step(0.f, Hovering());
	TestEqual(TEXT("80 brings the helicopter"), Meters->GetHeatLevel(), EHeatLevel::Helicopter);

	// Falling back, a level holds until Heat is LevelHysteresis below its threshold.
	Meters->SetHeatForTesting(76.f);
	Meters->Step(0.f, Hovering());
	TestEqual(TEXT("At 76 the helicopter stays"), Meters->GetHeatLevel(), EHeatLevel::Helicopter);
	Meters->SetHeatForTesting(74.f);
	Meters->Step(0.f, Hovering());
	TestEqual(TEXT("At 74 it's back to the police"), Meters->GetHeatLevel(), EHeatLevel::Police);
	Meters->SetHeatForTesting(41.f);
	Meters->Step(0.f, Hovering());
	TestEqual(TEXT("At 41 the police stay"), Meters->GetHeatLevel(), EHeatLevel::Police);
	Meters->SetHeatForTesting(39.f);
	Meters->Step(0.f, Hovering());
	TestEqual(TEXT("At 39 it's calm again"), Meters->GetHeatLevel(), EHeatLevel::Calm);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerHeatLieLowTest, "Tribute.PowerHeat.HeatFallsOnlyAfterLyingLow", PowerHeatTestFlags)
bool FPowerHeatLieLowTest::RunTest(const FString& Parameters)
{
	UPowerHeatComponent* Meters = MakeMeters();
	Run(Meters, 10.f, Beaming());
	const float Hot = Meters->GetHeat();

	Run(Meters, Meters->LieLowDelay - 0.5f, Hovering());
	TestEqual(TEXT("Heat holds until the saucer has lain low for LieLowDelay"), Meters->GetHeat(), Hot, 0.01f);

	Run(Meters, 5.f, Hovering());
	TestTrue(TEXT("Then it falls"), Meters->GetHeat() < Hot);

	Run(Meters, 0.5f, Beaming());
	const float AfterBlip = Meters->GetHeat();
	Run(Meters, 2.f, Hovering());
	TestEqual(TEXT("Any beam restarts the lie-low clock"), Meters->GetHeat(), AfterBlip, 0.01f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerHeatSpotlightTest, "Tribute.PowerHeat.SpotlightAddsHeatAndStopsLyingLow", PowerHeatTestFlags)
bool FPowerHeatSpotlightTest::RunTest(const FString& Parameters)
{
	UPowerHeatComponent* Meters = MakeMeters();
	FPowerHeatInputs Lit;
	Lit.bInSpotlight = true;
	Run(Meters, 4.f, Lit);
	TestEqual(TEXT("A spotlight adds SpotlightHeat a second, beam or no beam"), Meters->GetHeat(), 4.f * Meters->SpotlightHeat, 0.01f);

	Run(Meters, 10.f, Lit);
	const float StillLit = Meters->GetHeat();
	TestTrue(TEXT("Under a spotlight, Heat never starts to fall"), StillLit > 4.f * Meters->SpotlightHeat);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerHeatEmptyTest, "Tribute.PowerHeat.ZeroPowerBringsTheSaucerDown", PowerHeatTestFlags)
bool FPowerHeatEmptyTest::RunTest(const FString& Parameters)
{
	UPowerHeatComponent* Meters = MakeMeters();
	Run(Meters, 60.f, Beaming());
	TestTrue(TEXT("A minute of beaming empties a full tank"), Meters->IsPowerEmpty());
	TestEqual(TEXT("Power stops at zero"), Meters->GetPower(), 0.f);

	const float Drawn = Run(Meters, 5.f, Drawing());
	TestEqual(TEXT("A downed saucer can't refill"), Meters->GetPower(), 0.f);
	TestEqual(TEXT("and draws nothing"), Drawn, 0.f);
	TestTrue(TEXT("It stays down"), Meters->IsPowerEmpty());
	return true;
}

#endif
