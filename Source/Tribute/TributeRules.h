// The tribute: the mothership plays a call on its ring of 8 sectors, and the saucer answers by flying under
// each lit sector in order and pulsing the beam. Each round goes spin, catch (the call plays), pulse (the
// answer), stop (the ring holds), new colour. Three rounds of 3, 4 and 6 notes. Three missed calls and the
// ship leaves without you.
//
// The rules here are plain C++ with no world, so they can be tested (Tests/TributeTests.cpp):
// - FTributeScale: the ship's eight notes, one per sector, all inside one octave.
// - FTributeCalls: makes calls from a seed and refuses any with the forbidden interval shape.
// - FTributeRun: the round-by-round state machine. AMothership drives it and shows it.

#pragma once

#include "CoreMinimal.h"
#include "TributeRules.generated.h"

/** The ship's notes: one per sector, as semitones above the lowest. */
struct TRIBUTE_API FTributeScale
{
	static constexpr int32 SectorCount = 8;

	/**
	 * Our own eight-note scale. It spans 11 semitones, under an octave, so no call can ever drop an octave.
	 * That alone rules out the famous five-note figure; FTributeCalls checks for its shape as well.
	 */
	static constexpr int32 Semitones[SectorCount] = { 0, 2, 3, 5, 7, 8, 10, 11 };

	static int32 SemitoneOf(int32 Sector) { return Semitones[((Sector % SectorCount) + SectorCount) % SectorCount]; }
};

/** Making calls, and the check every call has to pass. */
struct TRIBUTE_API FTributeCalls
{
	/** The note counts of the three rounds. Never a five-note call. */
	static constexpr int32 RoundCount = 3;
	static constexpr int32 RoundLengths[RoundCount] = { 3, 4, 6 };

	/**
	 * True if any five notes in a row move up 2, down 4, down 12, up 7 semitones, in any key.
	 * That is the shape of a famous film's motif, and no call may have it.
	 */
	static bool HasForbiddenShape(const TArray<int32>& Semitones);

	/** True if a call (as sector numbers) may be played. */
	static bool IsAllowed(const TArray<int32>& Sectors);

	/** A call of Length sectors from the stream, with no sector twice in a row, that passes IsAllowed. */
	static TArray<int32> Make(FRandomStream& Stream, int32 Length);
};

/** Where the tribute is. */
UENUM(BlueprintType)
enum class ETributePhase : uint8
{
	NotStarted,
	Spin,      // the ring spins up before a round
	Catch,     // the ship plays the call: one sector lit at a time, with its tone
	Pulse,     // the saucer answers under each sector in turn
	Stop,      // the ring holds after a good answer
	NewColour, // the ring changes colour; the next call is longer
	Passed,    // all three rounds answered: dock
	Failed,    // three missed calls: the ship leaves without you
};

/** What an answer did. */
UENUM(BlueprintType)
enum class ETributeAnswer : uint8
{
	Ignored,      // not answering now (or not under any sector)
	Correct,      // the right sector; more notes to go
	CallAnswered, // the right sector, and the last note of the call
	Missed,       // the wrong sector, or too slow: the call plays again
	LeftBehind,   // that was the third miss
};

/** The tribute as a state machine. Advance it with time and answers; read the phase and the lit sector. */
USTRUCT(BlueprintType)
struct TRIBUTE_API FTributeRun
{
	GENERATED_BODY()

	// --- Timing, in seconds ---
	float SpinTime = 3.f;
	float NoteTime = 0.7f;
	float NoteGap = 0.3f;
	/** How long the saucer has for each note of an answer before it counts as a miss. */
	float AnswerTime = 12.f;
	float StopTime = 1.5f;
	float NewColourTime = 1.5f;
	int32 MissesAllowed = 3;

	/** Starts round one. The seed picks the calls, so a run can be replayed. */
	void Start(int32 Seed);

	/** Runs the clock on: plays the call, times the answer, moves between phases. */
	ETributeAnswer Advance(float DeltaSeconds);

	/** The saucer pulsed under Sector (or under none, as INDEX_NONE). */
	ETributeAnswer Answer(int32 Sector);

	/** The sector lit right now while the call plays, or INDEX_NONE. */
	int32 GetLitSector() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribute")
	ETributePhase Phase = ETributePhase::NotStarted;

	/** 0, 1 or 2. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribute")
	int32 Round = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribute")
	TArray<int32> Call;

	/** How many notes of the current call have been answered. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribute")
	int32 Answered = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribute")
	int32 Misses = 0;

	/** Notes answered right over the whole tribute, for the debrief's accuracy. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribute")
	int32 NotesRight = 0;

	/** Seconds into the current phase. */
	float PhaseTime = 0.f;

private:
	void Enter(ETributePhase NewPhase);
	ETributeAnswer Miss();

	FRandomStream Stream;
};
