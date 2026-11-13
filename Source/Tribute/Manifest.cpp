#include "Manifest.h"

FManifest FManifest::FromRow(const FManifestRow& Row)
{
	FManifest Manifest;
	Manifest.CowsWanted = Row.Cows;
	Manifest.PeopleWanted = Row.People;
	Manifest.NamedWanted = Row.NamedObjects;
	Manifest.NamedObjectName = Row.NamedObjectName;
	return Manifest;
}

bool FManifest::Count(ETargetKind Kind)
{
	const bool bWasShort = !IsLineFilled(Kind);
	switch (Kind)
	{
	case ETargetKind::Cow:
		++CowsTaken;
		break;
	case ETargetKind::Person:
		++PeopleTaken;
		break;
	case ETargetKind::NamedObject:
		++NamedTaken;
		break;
	}
	return bWasShort && IsLineFilled(Kind);
}

int32 FManifest::Wanted(ETargetKind Kind) const
{
	switch (Kind)
	{
	case ETargetKind::Cow:
		return CowsWanted;
	case ETargetKind::Person:
		return PeopleWanted;
	case ETargetKind::NamedObject:
		return NamedWanted;
	}
	return 0;
}

int32 FManifest::Taken(ETargetKind Kind) const
{
	switch (Kind)
	{
	case ETargetKind::Cow:
		return CowsTaken;
	case ETargetKind::Person:
		return PeopleTaken;
	case ETargetKind::NamedObject:
		return NamedTaken;
	}
	return 0;
}

bool FManifest::IsComplete() const
{
	return IsLineFilled(ETargetKind::Cow) && IsLineFilled(ETargetKind::Person) && IsLineFilled(ETargetKind::NamedObject);
}

float FManifest::FillFraction() const
{
	const int32 WantedTotal = CowsWanted + PeopleWanted + NamedWanted;
	if (WantedTotal <= 0)
	{
		return 1.f;
	}
	// Extras beyond a line don't make up for a short line elsewhere.
	const int32 Useful = FMath::Min(CowsTaken, CowsWanted) + FMath::Min(PeopleTaken, PeopleWanted) + FMath::Min(NamedTaken, NamedWanted);
	return static_cast<float>(Useful) / static_cast<float>(WantedTotal);
}

bool FRaidClock::Advance(float DeltaSeconds)
{
	if (bArrived)
	{
		return false;
	}
	SecondsLeft = FMath::Max(SecondsLeft - DeltaSeconds, 0.f);
	if (SecondsLeft <= 0.f)
	{
		bArrived = true;
		return true;
	}
	return false;
}

FString FRaidClock::ToMinutesSeconds() const
{
	const int32 Whole = FMath::CeilToInt(SecondsLeft);
	return FString::Printf(TEXT("%d:%02d"), Whole / 60, Whole % 60);
}
