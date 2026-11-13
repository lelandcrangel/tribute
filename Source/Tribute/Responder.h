// Something the town sends after the saucer when Heat gets high: the police car at 45%, later the
// helicopter at 80%. A responder chases the saucer and keeps a spotlight on it whenever it can see it;
// while the saucer is lit, Heat climbs and can't fall. When the town calms down, it goes back the way it came.
//
// ATributeGameMode sends responders in and calls them off; AResponderAIController does the driving.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Responder.generated.h"

class UMaterialInterface;
class UPointLightComponent;
class USpotLightComponent;
class UStaticMeshComponent;

UCLASS(Abstract)
class TRIBUTE_API AResponder : public ACharacter
{
	GENERATED_BODY()

public:
	AResponder();

	virtual void Tick(float DeltaSeconds) override;
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Grey-box body until BP_PoliceCar gives it a mesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Responder|Grey Box")
	TObjectPtr<UStaticMeshComponent> GreyBoxBody;

	/** The searchlight. It turns to follow the saucer whenever the saucer is in range and in sight. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Responder")
	TObjectPtr<USpotLightComponent> Searchlight;

	/** How close the saucer has to be for the searchlight to reach it, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Responder")
	float SearchlightRange = 4000.f;

	/** The searchlight's brightness when it's on the saucer, in candelas. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Responder")
	float SearchlightBrightness = 60000.f;

	/** Driving speed, in cm per second. Slower than the saucer, so it can be outrun. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Responder")
	float ChaseSpeed = 900.f;

	/** How close to the saucer's ground point it tries to get, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Responder")
	float ChaseDistance = 800.f;

	/** The grey-box body's colour. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Responder|Grey Box")
	FLinearColor GreyBoxColor = FLinearColor(0.2f, 0.2f, 0.2f);

	/** True while its searchlight is on this actor. */
	UFUNCTION(BlueprintPure, Category = "Responder")
	bool IsLighting(const AActor* Target) const { return Target && LitTarget == Target; }

	/** True once it's been called off and is heading home. */
	UFUNCTION(BlueprintPure, Category = "Responder")
	bool IsLeaving() const { return bLeaving; }

	/** Where it came from, and goes back to when called off. */
	FVector GetHome() const { return Home; }

	/** Called off: switch the searchlight off and drive home, then disappear. */
	void Leave();

	// --- Hooks for the Blueprint's sounds and effects ---

	/** It's on the scene: sirens. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Responder", meta = (DisplayName = "Arrived"))
	void Arrived();

	/** The searchlight found the saucer. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Responder", meta = (DisplayName = "Spotted Saucer"))
	void SpottedSaucer();

	/** The saucer got out of the searchlight. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Responder", meta = (DisplayName = "Lost Saucer"))
	void LostSaucer();

	/** Called off and heading home. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Responder", meta = (DisplayName = "Called Off"))
	void CalledOff();

protected:
	virtual void BeginPlay() override;

	/** Points the searchlight at the saucer if it can reach and see it, and says whether it did. */
	bool AimSearchlight();

	UPROPERTY(EditDefaultsOnly, Category = "Responder|Grey Box")
	TObjectPtr<UMaterialInterface> GreyBoxMaterial;

private:
	TWeakObjectPtr<AActor> LitTarget;
	FVector Home = FVector::ZeroVector;
	bool bLeaving = false;
};

/** The police car: comes at 45% Heat, flashing red and blue, and chases on the roads and fields. */
UCLASS()
class TRIBUTE_API APoliceCar : public AResponder
{
	GENERATED_BODY()

public:
	APoliceCar();

	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Police Car")
	TObjectPtr<UPointLightComponent> RedLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Police Car")
	TObjectPtr<UPointLightComponent> BlueLight;

	/** Seconds between the light bar switching sides. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Police Car")
	float FlashInterval = 0.35f;

private:
	float FlashTime = 0.f;
};
