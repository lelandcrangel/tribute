// Tests for the manifest and the raid clock: counting lifts against the order, lines filling once,
// extras not making up for short lines, and the countdown to the ship.

#include "Manifest.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace ManifestTests
{
	constexpr EAutomationTestFlags ManifestTestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

using namespace ManifestTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FManifestDefaultOrderTest, "Tribute.Manifest.DefaultOrderIsThreeCowsTwoPeopleOneObject", ManifestTestFlags)
bool FManifestDefaultOrderTest::RunTest(const FString& Parameters)
{
	const FManifestRow Row;
	const FManifest Manifest = FManifest::FromRow(Row);
	TestEqual(TEXT("Three cows"), Manifest.Wanted(ETargetKind::Cow), 3);
	TestEqual(TEXT("Two people"), Manifest.Wanted(ETargetKind::Person), 2);
	TestEqual(TEXT("One named object"), Manifest.Wanted(ETargetKind::NamedObject), 1);
	TestEqual(TEXT("The ship comes at seven minutes"), Row.RaidSeconds, 420.f);
	TestFalse(TEXT("An empty manifest isn't complete"), Manifest.IsComplete());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FManifestLineFillsOnceTest, "Tribute.Manifest.ALineFillsExactlyOnce", ManifestTestFlags)
bool FManifestLineFillsOnceTest::RunTest(const FString& Parameters)
{
	FManifest Manifest = FManifest::FromRow(FManifestRow());
	TestFalse(TEXT("First cow doesn't fill the line"), Manifest.Count(ETargetKind::Cow));
	TestFalse(TEXT("Second cow doesn't either"), Manifest.Count(ETargetKind::Cow));
	TestTrue(TEXT("Third cow fills it"), Manifest.Count(ETargetKind::Cow));
	TestFalse(TEXT("A fourth cow doesn't fill it again"), Manifest.Count(ETargetKind::Cow));
	TestEqual(TEXT("All four are counted"), Manifest.Taken(ETargetKind::Cow), 4);
	TestTrue(TEXT("The cow line is filled"), Manifest.IsLineFilled(ETargetKind::Cow));
	TestFalse(TEXT("The people line isn't"), Manifest.IsLineFilled(ETargetKind::Person));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FManifestFillFractionTest, "Tribute.Manifest.ExtrasDontMakeUpForShortLines", ManifestTestFlags)
bool FManifestFillFractionTest::RunTest(const FString& Parameters)
{
	FManifest Manifest = FManifest::FromRow(FManifestRow());
	TestEqual(TEXT("Nothing taken is 0"), Manifest.FillFraction(), 0.f);
	for (int32 i = 0; i < 6; ++i)
	{
		Manifest.Count(ETargetKind::Cow);
	}
	TestEqual(TEXT("Six cows still only fill the three cow places of six"), Manifest.FillFraction(), 3.f / 6.f, 0.001f);

	Manifest.Count(ETargetKind::Person);
	Manifest.Count(ETargetKind::Person);
	TestFalse(TEXT("Not complete without the named object"), Manifest.IsComplete());
	TestTrue(TEXT("The named object completes it"), Manifest.Count(ETargetKind::NamedObject));
	TestTrue(TEXT("Complete"), Manifest.IsComplete());
	TestEqual(TEXT("A complete manifest is 1"), Manifest.FillFraction(), 1.f, 0.001f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRaidClockTest, "Tribute.Manifest.ClockCountsDownToTheShipOnce", ManifestTestFlags)
bool FRaidClockTest::RunTest(const FString& Parameters)
{
	FRaidClock Clock;
	Clock.Start(420.f);
	TestEqual(TEXT("It starts at 7:00"), Clock.ToMinutesSeconds(), FString(TEXT("7:00")));

	TestFalse(TEXT("A second in, no ship"), Clock.Advance(1.f));
	TestEqual(TEXT("6:59 left"), Clock.ToMinutesSeconds(), FString(TEXT("6:59")));

	TestFalse(TEXT("Just before the end, no ship"), Clock.Advance(418.5f));
	TestEqual(TEXT("Half a second reads 0:01"), Clock.ToMinutesSeconds(), FString(TEXT("0:01")));

	TestTrue(TEXT("The ship arrives as it runs out"), Clock.Advance(1.f));
	TestEqual(TEXT("It reads 0:00"), Clock.ToMinutesSeconds(), FString(TEXT("0:00")));
	TestFalse(TEXT("It only arrives once"), Clock.Advance(1.f));
	return true;
}

#endif
