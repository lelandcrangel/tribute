// Runs one night: the raid, the arrival, the tribute, and the dock and ascent (or saucer down, or left
// behind). So far it puts the player in the saucer and sends the police in and home as Heat rises and falls;
// the rest of the run flow comes with the manifest and clock.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PowerHeatComponent.h"
#include "TributeGameMode.generated.h"

class AResponder;

UCLASS()
class TRIBUTE_API ATributeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATributeGameMode();

	/** What comes when Heat reaches the police level. BP_PoliceCar once it exists. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Responders")
	TSubclassOf<AResponder> PoliceCarClass;

	/** Actors with this tag mark where the police come from (the edge of town). The first one found is used. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Responders")
	FName PoliceStartTag = TEXT("PoliceStart");

protected:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	UFUNCTION()
	void HandleHeatLevelChanged(EHeatLevel NewLevel);

	/** Where to spawn the police: the tagged start, or failing that, far down the valley from the saucer. */
	FTransform FindPoliceStart() const;

	TWeakObjectPtr<AResponder> PoliceCar;
};
