#include "TributeGameMode.h"

#include "SaucerPawn.h"

ATributeGameMode::ATributeGameMode()
{
	// BP_Saucer replaces this once it exists, through the GameMode Blueprint or the World Settings.
	DefaultPawnClass = ASaucerPawn::StaticClass();
}
