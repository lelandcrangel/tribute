// Runs one night: the raid, the arrival, the tribute, and the dock and ascent (or saucer down, or left
// behind). For now it only puts the player in the saucer; the run flow comes with the manifest and clock.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TributeGameMode.generated.h"

UCLASS()
class TRIBUTE_API ATributeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATributeGameMode();
};
