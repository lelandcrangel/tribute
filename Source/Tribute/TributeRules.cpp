#include "TributeRules.h"

bool FTributeCalls::HasForbiddenShape(const TArray<int32>& Semitones)
{
	static constexpr int32 Shape[4] = { 2, -4, -12, 7 };
	for (int32 Start = 0; Start + 4 < Semitones.Num(); ++Start)
	{
		bool bMatches = true;
		for (int32 Step = 0; Step < 4 && bMatches; ++Step)
		{
			bMatches = Semitones[Start + Step + 1] - Semitones[Start + Step] == Shape[Step];
		}
		if (bMatches)
		{
			return true;
		}
	}
	return false;
}

bool FTributeCalls::IsAllowed(const TArray<int32>& Sectors)
{
	if (Sectors.Num() == 0 || Sectors.Num() == 5)
	{
		return false;
	}
	TArray<int32> Semitones;
	for (const int32 Sector : Sectors)
	{
		if (Sector < 0 || Sector >= FTributeScale::SectorCount)
		{
			return false;
		}
		Semitones.Add(FTributeScale::SemitoneOf(Sector));
	}
	return !FTributeCalls::HasForbiddenShape(Semitones);
}

TArray<int32> FTributeCalls::Make(FRandomStream& Stream, int32 Length)
{
	TArray<int32> Call;
	// The scale can't make the shape, so this never loops in practice; the cap is a guard, not a hope.
	for (int32 Attempt = 0; Attempt < 1000; ++Attempt)
	{
		Call.Reset();
		int32 Previous = INDEX_NONE;
		for (int32 Note = 0; Note < Length; ++Note)
		{
			int32 Sector = Stream.RandRange(0, FTributeScale::SectorCount - 2);
			// Skip the previous sector, so a call never asks for the same place twice in a row.
			if (Previous != INDEX_NONE && Sector >= Previous)
			{
				++Sector;
			}
			Call.Add(Sector);
			Previous = Sector;
		}
		if (IsAllowed(Call))
		{
			return Call;
		}
	}
	checkf(false, TEXT("No allowed call of length %d"), Length);
	return Call;
}

void FTributeRun::Start(int32 Seed)
{
	Stream.Initialize(Seed);
	Round = 0;
	Misses = 0;
	NotesRight = 0;
	Call = FTributeCalls::Make(Stream, FTributeCalls::RoundLengths[0]);
	Enter(ETributePhase::Spin);
}

void FTributeRun::Enter(ETributePhase NewPhase)
{
	Phase = NewPhase;
	PhaseTime = 0.f;
	if (NewPhase == ETributePhase::Catch || NewPhase == ETributePhase::Pulse)
	{
		Answered = 0;
	}
}

int32 FTributeRun::GetLitSector() const
{
	if (Phase != ETributePhase::Catch)
	{
		return INDEX_NONE;
	}
	const float NoteSpan = NoteTime + NoteGap;
	const int32 Note = FMath::FloorToInt(PhaseTime / NoteSpan);
	const bool bInNote = FMath::Fmod(PhaseTime, NoteSpan) < NoteTime;
	return (Call.IsValidIndex(Note) && bInNote) ? Call[Note] : INDEX_NONE;
}

ETributeAnswer FTributeRun::Advance(float DeltaSeconds)
{
	PhaseTime += DeltaSeconds;
	switch (Phase)
	{
	case ETributePhase::Spin:
		if (PhaseTime >= SpinTime)
		{
			Enter(ETributePhase::Catch);
		}
		break;
	case ETributePhase::Catch:
		if (PhaseTime >= Call.Num() * (NoteTime + NoteGap))
		{
			Enter(ETributePhase::Pulse);
		}
		break;
	case ETributePhase::Pulse:
		// Each note has its own time limit, counted from the last right answer.
		if (PhaseTime >= AnswerTime)
		{
			return Miss();
		}
		break;
	case ETributePhase::Stop:
		if (PhaseTime >= StopTime)
		{
			if (Round + 1 >= FTributeCalls::RoundCount)
			{
				Enter(ETributePhase::Passed);
			}
			else
			{
				Enter(ETributePhase::NewColour);
			}
		}
		break;
	case ETributePhase::NewColour:
		if (PhaseTime >= NewColourTime)
		{
			++Round;
			Call = FTributeCalls::Make(Stream, FTributeCalls::RoundLengths[Round]);
			Enter(ETributePhase::Spin);
		}
		break;
	default:
		break;
	}
	return ETributeAnswer::Ignored;
}

ETributeAnswer FTributeRun::Answer(int32 Sector)
{
	if (Phase != ETributePhase::Pulse || !Call.IsValidIndex(Answered))
	{
		return ETributeAnswer::Ignored;
	}
	if (Sector != Call[Answered])
	{
		return Miss();
	}

	++Answered;
	++NotesRight;
	PhaseTime = 0.f;
	if (Answered >= Call.Num())
	{
		Enter(ETributePhase::Stop);
		return ETributeAnswer::CallAnswered;
	}
	return ETributeAnswer::Correct;
}

ETributeAnswer FTributeRun::Miss()
{
	++Misses;
	if (Misses >= MissesAllowed)
	{
		Enter(ETributePhase::Failed);
		return ETributeAnswer::LeftBehind;
	}
	// The ship plays the same call again.
	Enter(ETributePhase::Catch);
	return ETributeAnswer::Missed;
}
