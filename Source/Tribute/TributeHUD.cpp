#include "TributeHUD.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/World.h"
#include "PowerHeatComponent.h"
#include "SaucerPawn.h"
#include "TributeGameState.h"

namespace
{
	/** How long a manifest line flashes: longer when the line fills, a blink for any other lift. */
	constexpr float FilledFlashSeconds = 1.2f;
	constexpr float CountedFlashSeconds = 0.4f;
}

// --- The widget ---

UTextBlock* UTributeHUDWidget::MakeText(const FString& Text, int32 Size)
{
	UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Block->SetText(FText::FromString(Text));
	FSlateFontInfo Font = Block->GetFont();
	Font.Size = Size;
	Block->SetFont(Font);
	Block->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	// A drop shadow keeps it readable over the beam's glare and the bright fields.
	Block->SetShadowOffset(FVector2D(1.5f, 1.5f));
	Block->SetShadowColorAndOpacity(FLinearColor(0.f, 0.f, 0.f, 0.85f));
	return Block;
}

UProgressBar* UTributeHUDWidget::AddMeter(UVerticalBox* Box, const FString& Label)
{
	UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	UVerticalBoxSlot* RowSlot = Box->AddChildToVerticalBox(Row);
	RowSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

	USizeBox* LabelBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	LabelBox->SetWidthOverride(70.f);
	LabelBox->AddChild(MakeText(Label, 14));
	UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(LabelBox);
	LabelSlot->SetVerticalAlignment(VAlign_Center);

	USizeBox* BarBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	BarBox->SetWidthOverride(220.f);
	BarBox->SetHeightOverride(12.f);
	UProgressBar* Bar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
	BarBox->AddChild(Bar);
	UHorizontalBoxSlot* BarSlot = Row->AddChildToHorizontalBox(BarBox);
	BarSlot->SetVerticalAlignment(VAlign_Center);
	return Bar;
}

void UTributeHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// A WBP_HUD with its own layout keeps it; the plain class builds this one.
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	// Top left: Power and Heat.
	UVerticalBox* Meters = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UCanvasPanelSlot* MetersSlot = Root->AddChildToCanvas(Meters);
	MetersSlot->SetAnchors(FAnchors(0.f, 0.f));
	MetersSlot->SetAlignment(FVector2D(0.f, 0.f));
	MetersSlot->SetPosition(FVector2D(40.f, 32.f));
	MetersSlot->SetAutoSize(true);
	PowerBar = AddMeter(Meters, TEXT("POWER"));
	HeatBar = AddMeter(Meters, TEXT("HEAT"));
	PowerBar->SetFillColorAndOpacity(PowerColour);

	// Top centre: the countdown to the ship.
	ClockText = MakeText(TEXT("SHIP IN 7:00"), 22);
	UCanvasPanelSlot* ClockSlot = Root->AddChildToCanvas(ClockText);
	ClockSlot->SetAnchors(FAnchors(0.5f, 0.f));
	ClockSlot->SetAlignment(FVector2D(0.5f, 0.f));
	ClockSlot->SetPosition(FVector2D(0.f, 28.f));
	ClockSlot->SetAutoSize(true);

	// Top right: the manifest.
	UVerticalBox* Lines = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UCanvasPanelSlot* LinesSlot = Root->AddChildToCanvas(Lines);
	LinesSlot->SetAnchors(FAnchors(1.f, 0.f));
	LinesSlot->SetAlignment(FVector2D(1.f, 0.f));
	LinesSlot->SetPosition(FVector2D(-40.f, 30.f));
	LinesSlot->SetAutoSize(true);

	UTextBlock* Title = MakeText(TEXT("MANIFEST"), 13);
	Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.7f, 0.75f)));
	Lines->AddChildToVerticalBox(Title)->SetHorizontalAlignment(HAlign_Right);
	CowsLine = MakeText(TEXT(""), 18);
	PeopleLine = MakeText(TEXT(""), 18);
	NamedLine = MakeText(TEXT(""), 18);
	for (UTextBlock* Line : { CowsLine.Get(), PeopleLine.Get(), NamedLine.Get() })
	{
		UVerticalBoxSlot* LineSlot = Lines->AddChildToVerticalBox(Line);
		LineSlot->SetHorizontalAlignment(HAlign_Right);
		LineSlot->SetPadding(FMargin(0.f, 2.f, 0.f, 0.f));
	}
}

void UTributeHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (const ASaucerPawn* Saucer = Cast<ASaucerPawn>(GetOwningPlayerPawn()))
	{
		const UPowerHeatComponent* Meters = Saucer->PowerHeat;
		if (PowerBar)
		{
			PowerBar->SetPercent(Meters->GetPowerFraction());
			// Running low turns the bar orange: time to find a transformer.
			PowerBar->SetFillColorAndOpacity(Meters->GetPowerFraction() < 0.25f ? FLinearColor(1.f, 0.55f, 0.1f) : PowerColour);
		}
		if (HeatBar)
		{
			HeatBar->SetPercent(Meters->GetHeatFraction());
			HeatBar->SetFillColorAndOpacity(FLinearColor::LerpUsingHSV(CalmColour, HotColour, Meters->GetHeatFraction()));
		}
	}

	ATributeGameState* State = GetWorld() ? GetWorld()->GetGameState<ATributeGameState>() : nullptr;
	if (!State)
	{
		return;
	}
	if (!bBoundToGameState)
	{
		State->OnManifestCounted.AddDynamic(this, &UTributeHUDWidget::HandleManifestCounted);
		bBoundToGameState = true;
	}

	if (ClockText)
	{
		ClockText->SetText(FText::FromString(State->Clock.bArrived
			? TEXT("THE SHIP IS HERE") : FString::Printf(TEXT("SHIP IN %s"), *State->Clock.ToMinutesSeconds())));
	}
	UpdateLine(ETargetKind::Cow, CowsLine, InDeltaTime);
	UpdateLine(ETargetKind::Person, PeopleLine, InDeltaTime);
	UpdateLine(ETargetKind::NamedObject, NamedLine, InDeltaTime);
}

void UTributeHUDWidget::UpdateLine(ETargetKind Kind, UTextBlock* Line, float DeltaSeconds)
{
	const ATributeGameState* State = GetWorld() ? GetWorld()->GetGameState<ATributeGameState>() : nullptr;
	if (!Line || !State)
	{
		return;
	}
	const FManifest& Manifest = State->Manifest;

	FString Name;
	switch (Kind)
	{
	case ETargetKind::Cow:
		Name = TEXT("Cows");
		break;
	case ETargetKind::Person:
		Name = TEXT("People");
		break;
	case ETargetKind::NamedObject:
		Name = Manifest.NamedObjectName.ToString();
		break;
	}
	Line->SetText(FText::FromString(FString::Printf(TEXT("%s   %d / %d"), *Name, Manifest.Taken(Kind), Manifest.Wanted(Kind))));

	float& Flash = FlashLeft[static_cast<int32>(Kind)];
	const FLinearColor Settled = Manifest.IsLineFilled(Kind) ? FilledColour : FLinearColor::White;
	if (Flash > 0.f)
	{
		Flash = FMath::Max(Flash - DeltaSeconds, 0.f);
		Line->SetColorAndOpacity(FSlateColor(FLinearColor::LerpUsingHSV(Settled, FlashColour, FMath::Min(Flash / CountedFlashSeconds, 1.f))));
		Line->SetRenderScale(FVector2D(1.f + 0.15f * FMath::Min(Flash / CountedFlashSeconds, 1.f)));
	}
	else
	{
		Line->SetColorAndOpacity(FSlateColor(Settled));
		Line->SetRenderScale(FVector2D(1.f));
	}
}

void UTributeHUDWidget::HandleManifestCounted(ETargetKind Kind, bool bFilledLine)
{
	FlashLeft[static_cast<int32>(Kind)] = bFilledLine ? FilledFlashSeconds : CountedFlashSeconds;
	if (bFilledLine)
	{
		LineFilled(Kind);
	}
}

// --- The HUD actor ---

ATributeHUD::ATributeHUD()
{
	HUDWidgetClass = UTributeHUDWidget::StaticClass();
}

void ATributeHUD::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* Player = GetOwningPlayerController())
	{
		HUDWidget = CreateWidget<UTributeHUDWidget>(Player, HUDWidgetClass ? HUDWidgetClass.Get() : UTributeHUDWidget::StaticClass());
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
}
