// Runs one night. The raid: the player flies the saucer, its lifts are counted against the manifest, and the
// police are sent in and home as Heat rises and falls. The arrival: when the clock runs out, the police
// leave, the valley's lights die and the mothership comes down. The tribute: it plays its calls. Then one of
// three endings, each followed by the debrief: docked (the saucer rides up into the hull), left behind (three
// missed calls; the ship goes without it), or saucer down (Power ran out).

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LiftableComponent.h"
#include "PowerHeatComponent.h"
#include "TributeGameState.h"
#include "TributeGameMode.generated.h"

class ACameraActor;
class AMothership;
class AResponder;
class ASaucerPawn;

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

	/** What arrives if the level has no mothership placed. BP_Mothership once it exists. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mothership")
	TSubclassOf<AMothership> MothershipClass;

	/** Where the arrival camera stands, relative to the ship's spot over the valley: far out and low, so the
	 *  ship's scale shows. It frames the arrival, and the dock and ascent. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mothership")
	FVector ArrivalCameraOffset = FVector(-26000.f, -9000.f, 500.f);

	/** Seconds between the end and the debrief, so the ending can be seen. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mothership")
	float DebriefDelay = 4.f;

protected:
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	UFUNCTION()
	void HandleHeatLevelChanged(EHeatLevel NewLevel);

	/** Counts whatever the saucer takes aboard against the manifest. */
	UFUNCTION()
	void HandleTargetLifted(AActor* Target, ETargetKind Kind);

	UFUNCTION()
	void HandleShipArrived();

	UFUNCTION()
	void HandleTributeEnded(bool bPassed);

	UFUNCTION()
	void HandleDocked();

	UFUNCTION()
	void HandlePowerEmpty();

	/** Cuts to the wide, low view of the ship (blending over BlendSeconds), or back to the saucer. */
	void ShowShipView(bool bOnShip, float BlendSeconds);

	/** Ends the night: works out the score and shows the debrief after DebriefDelay. */
	void EndRun(ERunEnding Ending);

	/** Where to spawn the police: the tagged start, or failing that, far down the valley from the saucer. */
	FTransform FindPoliceStart() const;

	TWeakObjectPtr<AResponder> PoliceCar;
	TWeakObjectPtr<ASaucerPawn> Saucer;
	TWeakObjectPtr<AMothership> Mothership;
	TWeakObjectPtr<ACameraActor> ShipCamera;
	bool bShipArrived = false;
	bool bRunOver = false;
};
