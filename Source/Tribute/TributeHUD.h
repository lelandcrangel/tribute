// The raid's HUD, kept small: Power and Heat top left, the countdown to the ship top centre, the manifest
// top right. Nothing floats over the world. ATributeHUD puts UTributeHUDWidget on screen.
//
// The widget builds its own layout in code, so it works before any widget asset exists. A WBP_HUD made from
// UTributeHUDWidget can restyle it later (set HUDWidgetClass on ATributeHUD).

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/HUD.h"
#include "LiftableComponent.h"
#include "TributeHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UVerticalBox;
class UPanelWidget;

UCLASS()
class TRIBUTE_API UTributeHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** The meters' colours: Heat runs from calm to hot, like the saucer's rim. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FLinearColor PowerColour = FLinearColor(0.55f, 0.9f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FLinearColor CalmColour = FLinearColor(0.1f, 0.45f, 1.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FLinearColor HotColour = FLinearColor(1.f, 0.08f, 0.03f);

	/** A manifest line that's been filled. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FLinearColor FilledColour = FLinearColor(0.45f, 1.f, 0.55f);

	/** The flash when a line fills, fading to FilledColour. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	FLinearColor FlashColour = FLinearColor(1.f, 0.95f, 0.4f);

	/** A line filled: the place for a chime and a burst in WBP_HUD. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HUD", meta = (DisplayName = "Line Filled"))
	void LineFilled(ETargetKind Kind);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UFUNCTION()
	void HandleManifestCounted(ETargetKind Kind, bool bFilledLine);

	UTextBlock* MakeText(const FString& Text, int32 Size);
	UProgressBar* AddMeter(UVerticalBox* Box, const FString& Label);
	void UpdateLine(ETargetKind Kind, UTextBlock* Line, float DeltaSeconds);

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> PowerBar;

	UPROPERTY(Transient)
	TObjectPtr<UProgressBar> HeatBar;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> ClockText;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> CowsLine;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> PeopleLine;

	UPROPERTY(Transient)
	TObjectPtr<UTextBlock> NamedLine;

	/** Seconds left on each line's flash, by ETargetKind. */
	float FlashLeft[3] = { 0.f, 0.f, 0.f };
	bool bBoundToGameState = false;
};

UCLASS()
class TRIBUTE_API ATributeHUD : public AHUD
{
	GENERATED_BODY()

public:
	ATributeHUD();

	/** The widget to show. A WBP_HUD made from UTributeHUDWidget, or the plain one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UTributeHUDWidget> HUDWidgetClass;

	UFUNCTION(BlueprintPure, Category = "HUD")
	UTributeHUDWidget* GetHUDWidget() const { return HUDWidget; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UTributeHUDWidget> HUDWidget;
};
