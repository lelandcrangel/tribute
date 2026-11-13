// Tests for the tribute: the scale stays inside an octave, no call ever has the forbidden interval shape,
// the rounds are 3, 4 and 6 notes, and the state machine passes good answers and leaves the saucer behind
// after three misses.

#include "Misc/AutomationTest.h"
#include "TributeRules.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace TributeTests
{
	constexpr EAutomationTestFlags TributeTestFlags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Runs the tribute on until it's waiting for an answer (or it ends). */
	void RunToPulse(FTributeRun& Run)
	{
		for (int32 Frame = 0; Frame < 1000 && Run.Phase != ETributePhase::Pulse
			&& Run.Phase != ETributePhase::Passed && Run.Phase != ETributePhase::Failed; ++Frame)
		{
			Run.Advance(0.05f);
		}
	}
}

using namespace TributeTests;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTributeScaleTest, "Tribute.Tribute.SectorNotesStayInsideOneOctave", TributeTestFlags)
bool FTributeScaleTest::RunTest(const FString& Parameters)
{
	int32 Lowest = TNumericLimits<int32>::Max();
	int32 Highest = TNumericLimits<int32>::Lowest();
	for (int32 Sector = 0; Sector < FTributeScale::SectorCount; ++Sector)
	{
		Lowest = FMath::Min(Lowest, FTributeScale::SemitoneOf(Sector));
		Highest = FMath::Max(Highest, FTributeScale::SemitoneOf(Sector));
	}
	TestEqual(TEXT("Eight sectors"), FTributeScale::SectorCount, 8);
	TestTrue(TEXT("The whole scale spans less than an octave, so no call can drop an octave"), Highest - Lowest < 12);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTributeShapeCheckTest, "Tribute.Tribute.ForbiddenShapeIsRecognisedInAnyKey", TributeTestFlags)
bool FTributeShapeCheckTest::RunTest(const FString& Parameters)
{
	// The shape is up 2, down 4, down 12, up 7. Written out from three different starting notes.
	TestTrue(TEXT("From 60"), FTributeCalls::HasForbiddenShape({ 60, 62, 58, 46, 53 }));
	TestTrue(TEXT("From 0"), FTributeCalls::HasForbiddenShape({ 0, 2, -2, -14, -7 }));
	TestTrue(TEXT("Inside a longer run"), FTributeCalls::HasForbiddenShape({ 5, 9, 67, 69, 65, 53, 60, 61 }));
	TestFalse(TEXT("Same notes, different order"), FTributeCalls::HasForbiddenShape({ 60, 58, 62, 46, 53 }));
	TestFalse(TEXT("Four of the five isn't the shape"), FTributeCalls::HasForbiddenShape({ 60, 62, 58, 46 }));
	TestFalse(TEXT("Near miss: down 11 instead of 12"), FTributeCalls::HasForbiddenShape({ 60, 62, 58, 47, 54 }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTributeCallsNeverHaveShapeTest, "Tribute.Tribute.GeneratedCallsNeverHaveTheShape", TributeTestFlags)
bool FTributeCallsNeverHaveShapeTest::RunTest(const FString& Parameters)
{
	int32 Checked = 0;
	for (int32 Seed = 0; Seed < 2000; ++Seed)
	{
		FRandomStream Stream(Seed);
		for (const int32 Length : FTributeCalls::RoundLengths)
		{
			const TArray<int32> Call = FTributeCalls::Make(Stream, Length);
			TArray<int32> Semitones;
			for (int32 i = 0; i < Call.Num(); ++i)
			{
				Semitones.Add(FTributeScale::SemitoneOf(Call[i]));
				if (i > 0 && Call[i] == Call[i - 1])
				{
					AddError(FString::Printf(TEXT("Seed %d repeats a sector"), Seed));
				}
			}
			if (FTributeCalls::HasForbiddenShape(Semitones) || Call.Num() != Length)
			{
				AddError(FString::Printf(TEXT("Seed %d made a bad call of %d"), Seed, Length));
			}
			++Checked;
		}
	}
	TestEqual(TEXT("Every call checked"), Checked, 6000);
	TestFalse(TEXT("A five-note call is never allowed"), FTributeCalls::IsAllowed({ 0, 1, 2, 3, 4 }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTributeRoundsTest, "Tribute.Tribute.RoundsAreThreeFourAndSix", TributeTestFlags)
bool FTributeRoundsTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("Three rounds"), FTributeCalls::RoundCount, 3);
	TestEqual(TEXT("Round one has 3 notes"), FTributeCalls::RoundLengths[0], 3);
	TestEqual(TEXT("Round two has 4"), FTributeCalls::RoundLengths[1], 4);
	TestEqual(TEXT("Round three has 6, never 5"), FTributeCalls::RoundLengths[2], 6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTributePassTest, "Tribute.Tribute.RightAnswersPassAllThreeRounds", TributeTestFlags)
bool FTributePassTest::RunTest(const FString& Parameters)
{
	FTributeRun Run;
	Run.Start(42);
	TestEqual(TEXT("It starts by spinning"), Run.Phase, ETributePhase::Spin);

	for (int32 Round = 0; Round < 3; ++Round)
	{
		RunToPulse(Run);
		TestEqual(TEXT("Waiting for the answer"), Run.Phase, ETributePhase::Pulse);
		TestEqual(TEXT("This round's call length"), Run.Call.Num(), FTributeCalls::RoundLengths[Round]);
		const TArray<int32> Call = Run.Call;
		for (int32 Note = 0; Note < Call.Num(); ++Note)
		{
			const ETributeAnswer Result = Run.Answer(Call[Note]);
			TestEqual(TEXT("Each note answers right"), Result,
				Note + 1 == Call.Num() ? ETributeAnswer::CallAnswered : ETributeAnswer::Correct);
		}
	}
	RunToPulse(Run);
	TestEqual(TEXT("All three answered: passed"), Run.Phase, ETributePhase::Passed);
	TestEqual(TEXT("No misses"), Run.Misses, 0);
	TestEqual(TEXT("Every note right"), Run.NotesRight, 3 + 4 + 6);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTributeMissTest, "Tribute.Tribute.ThreeMissesLeaveTheSaucerBehind", TributeTestFlags)
bool FTributeMissTest::RunTest(const FString& Parameters)
{
	FTributeRun Run;
	Run.Start(7);
	RunToPulse(Run);
	const int32 Wrong = (Run.Call[0] + 1) % FTributeScale::SectorCount;

	TestEqual(TEXT("A wrong sector is a miss"), Run.Answer(Wrong), ETributeAnswer::Missed);
	TestEqual(TEXT("and the call plays again"), Run.Phase, ETributePhase::Catch);

	RunToPulse(Run);
	Run.Answer(Run.Call[0]);
	TestEqual(TEXT("A right first note, then waiting too long, is a miss"), Run.Advance(Run.AnswerTime + 0.1f), ETributeAnswer::Missed);
	TestEqual(TEXT("Two misses"), Run.Misses, 2);

	RunToPulse(Run);
	TestEqual(TEXT("The third miss leaves the saucer behind"), Run.Answer(Wrong), ETributeAnswer::LeftBehind);
	TestEqual(TEXT("Failed"), Run.Phase, ETributePhase::Failed);
	TestEqual(TEXT("Nothing more counts"), Run.Answer(Run.Call[0]), ETributeAnswer::Ignored);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTributeCatchTest, "Tribute.Tribute.TheCallLightsEachSectorInTurn", TributeTestFlags)
bool FTributeCatchTest::RunTest(const FString& Parameters)
{
	FTributeRun Run;
	Run.Start(3);
	TestEqual(TEXT("Nothing lit while spinning"), Run.GetLitSector(), static_cast<int32>(INDEX_NONE));
	TestEqual(TEXT("Pulsing during the spin is ignored"), Run.Answer(0), ETributeAnswer::Ignored);
	Run.Advance(Run.SpinTime);
	TestEqual(TEXT("Then the call plays"), Run.Phase, ETributePhase::Catch);

	TArray<int32> Seen;
	int32 Last = INDEX_NONE;
	while (Run.Phase == ETributePhase::Catch)
	{
		const int32 Lit = Run.GetLitSector();
		if (Lit != INDEX_NONE && Lit != Last)
		{
			Seen.Add(Lit);
		}
		Last = Lit;
		Run.Advance(0.05f);
	}
	TestTrue(TEXT("The sectors lit in order are the call"), Seen == Run.Call);
	return true;
}

#endif
