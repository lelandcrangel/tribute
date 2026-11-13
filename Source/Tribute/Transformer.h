// A farm's pole transformer: the saucer's refill. Hover over it and it feeds the saucer Power, which blacks
// the farm out (its lights go dark) and raises Heat. Each one holds a limited charge, so the player has to
// find the next one.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Transformer.generated.h"

class UPointLightComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class TRIBUTE_API ATransformer : public AActor
{
	GENERATED_BODY()

public:
	ATransformer();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** The pole, 8 m tall. The saucer's hover ignores it, so hanging over one doesn't bounce the saucer up. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transformer")
	TObjectPtr<UStaticMeshComponent> Pole;

	/** The can at the top. Grey-box yellow while it has charge, dark once it's empty. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transformer")
	TObjectPtr<UStaticMeshComponent> Can;

	/** The farm's yard light, which goes out with the blackout. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transformer")
	TObjectPtr<UPointLightComponent> YardLight;

	/** Other lights on this farm (porch lights, windows) that go dark with it. */
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Transformer")
	TArray<TObjectPtr<AActor>> FarmLights;

	/** How much Power it can give in all. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transformer", meta = (ClampMin = "1"))
	float MaxCharge = 120.f;

	/** How close the saucer has to be, measured flat across the ground, to draw from it, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Transformer")
	float DrawRadius = 600.f;

	/** True if a saucer at this location can draw from it now. */
	UFUNCTION(BlueprintPure, Category = "Transformer")
	bool CanFeed(const FVector& SaucerLocation) const;

	/** Takes Amount of charge. The first draw blacks the farm out. */
	void Feed(float Amount);

	UFUNCTION(BlueprintPure, Category = "Transformer")
	float GetCharge() const { return Charge; }

	UFUNCTION(BlueprintPure, Category = "Transformer")
	bool IsBlackedOut() const { return bBlackedOut; }

	/** The ship's arrival kills every farm's lights, drawn from or not. */
	UFUNCTION(BlueprintCallable, Category = "Transformer")
	void PowerDown() { if (!bBlackedOut) { BlackOut(); } }

	// --- Hooks for the Blueprint's sounds and effects ---

	/** The farm just went dark: a thunk, the hum dying, dogs. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Transformer", meta = (DisplayName = "Blacked Out"))
	void BlackedOut();

	/** The last of its charge is gone: sparks. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Transformer", meta = (DisplayName = "Emptied"))
	void Emptied();

protected:
	virtual void BeginPlay() override;

private:
	void BlackOut();
	void TintCan();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CanTint;

	UPROPERTY(EditDefaultsOnly, Category = "Transformer")
	TObjectPtr<UMaterialInterface> GreyBoxMaterial;

	float Charge = 0.f;
	bool bBlackedOut = false;
};
