#include "TributeGameMode.h"

#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/LightComponent.h"
#include "Engine/DirectionalLight.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Mothership.h"
#include "Responder.h"
#include "SaucerPawn.h"
#include "TimerManager.h"
#include "Transformer.h"
#include "Tribute.h"
#include "TributeHUD.h"

ATributeGameMode::ATributeGameMode()
{
	// BP_Saucer, BP_PoliceCar and BP_Mothership replace these once they exist, through a GameMode Blueprint.
	DefaultPawnClass = ASaucerPawn::StaticClass();
	PoliceCarClass = APoliceCar::StaticClass();
	MothershipClass = AMothership::StaticClass();
	GameStateClass = ATributeGameState::StaticClass();
	HUDClass = ATributeHUD::StaticClass();
}

void ATributeGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// The saucer exists now: count what it takes, send responders by its Heat, and watch its Power.
	if (ASaucerPawn* NewSaucer = NewPlayer ? Cast<ASaucerPawn>(NewPlayer->GetPawn()) : nullptr)
	{
		Saucer = NewSaucer;
		NewSaucer->PowerHeat->OnHeatLevelChanged.AddDynamic(this, &ATributeGameMode::HandleHeatLevelChanged);
		NewSaucer->PowerHeat->OnPowerEmpty.AddDynamic(this, &ATributeGameMode::HandlePowerEmpty);
		NewSaucer->OnTargetLifted.AddDynamic(this, &ATributeGameMode::HandleTargetLifted);
		NewSaucer->OnDocked.AddDynamic(this, &ATributeGameMode::HandleDocked);
	}
	if (ATributeGameState* State = GetGameState<ATributeGameState>())
	{
		State->OnShipArrived.AddUniqueDynamic(this, &ATributeGameMode::HandleShipArrived);
	}
}

void ATributeGameMode::HandleTargetLifted(AActor* Target, ETargetKind Kind)
{
	if (ATributeGameState* State = GetGameState<ATributeGameState>())
	{
		State->CountTakenAboard(Kind);
	}
}

void ATributeGameMode::HandleHeatLevelChanged(EHeatLevel NewLevel)
{
	// Once the ship is here, the town has other things to look at.
	if (bShipArrived)
	{
		return;
	}

	// One police car at a time: it comes at the police level, stays through the helicopter, and goes when
	// the town calms down. If Heat climbs again after it's been called off, a new one comes.
	if (NewLevel != EHeatLevel::Calm)
	{
		if ((!PoliceCar.IsValid() || PoliceCar->IsLeaving()) && PoliceCarClass)
		{
			FActorSpawnParameters Spawn;
			Spawn.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			PoliceCar = GetWorld()->SpawnActor<AResponder>(PoliceCarClass, FindPoliceStart(), Spawn);
			UE_LOG(LogTribute, Log, TEXT("Heat reached the police: sent %s"), PoliceCar.IsValid() ? *PoliceCar->GetName() : TEXT("nothing"));
		}
	}
	else if (PoliceCar.IsValid())
	{
		PoliceCar->Leave();
	}
}

void ATributeGameMode::HandleShipArrived()
{
	if (bShipArrived || bRunOver)
	{
		return;
	}
	bShipArrived = true;

	// The police scatter.
	for (TActorIterator<AResponder> It(GetWorld()); It; ++It)
	{
		It->Leave();
	}
	// The valley's lights die, and the moon goes behind the ship.
	for (TActorIterator<ATransformer> It(GetWorld()); It; ++It)
	{
		It->PowerDown();
	}
	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		if (ULightComponent* Moon = It->GetLightComponent())
		{
			Moon->SetIntensity(Moon->Intensity * 0.15f);
		}
	}

	// The ship: the one placed in the level, or a new one over the valley's centre.
	if (TActorIterator<AMothership> Placed(GetWorld()); Placed)
	{
		Mothership = *Placed;
	}
	if (!Mothership.IsValid() && MothershipClass)
	{
		Mothership = GetWorld()->SpawnActor<AMothership>(MothershipClass, FTransform(FVector::ZeroVector));
	}
	if (Mothership.IsValid())
	{
		Mothership->OnTributeEnded.AddUniqueDynamic(this, &ATributeGameMode::HandleTributeEnded);
		Mothership->BeginArrival(Saucer.Get());

		// Watch it come down from far out and low, then hand the view back for the tribute.
		ShowShipView(true, 2.f);
		FTimerHandle Back;
		GetWorldTimerManager().SetTimer(Back, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			// Back to the saucer, pulled up wide so all eight pools are in view.
			if (Saucer.IsValid())
			{
				Saucer->SetWideView(true);
			}
			ShowShipView(false, 2.5f);
		}),
			Mothership->ArrivalTime + 1.5f, false);
	}
}

void ATributeGameMode::ShowShipView(bool bOnShip, float BlendSeconds)
{
	APlayerController* Player = GetWorld()->GetFirstPlayerController();
	if (!Player || !Mothership.IsValid())
	{
		return;
	}
	if (!bOnShip)
	{
		if (Saucer.IsValid())
		{
			Player->SetViewTargetWithBlend(Saucer.Get(), BlendSeconds, VTBlend_Cubic);
		}
		return;
	}
	if (!ShipCamera.IsValid())
	{
		const FVector ShipSpot(Mothership->GetActorLocation().X, Mothership->GetActorLocation().Y, Mothership->ValleyFloorZ);
		const FVector CameraAt = ShipSpot + ArrivalCameraOffset;
		const FVector LookAt = ShipSpot + FVector(0.f, 0.f, Mothership->HoverHeight * 0.7f);
		ShipCamera = GetWorld()->SpawnActor<ACameraActor>(CameraAt, (LookAt - CameraAt).Rotation());
		if (ShipCamera.IsValid())
		{
			ShipCamera->GetCameraComponent()->SetFieldOfView(70.f);
			ShipCamera->GetCameraComponent()->bConstrainAspectRatio = false;
		}
	}
	if (ShipCamera.IsValid())
	{
		Player->SetViewTargetWithBlend(ShipCamera.Get(), BlendSeconds, VTBlend_Cubic);
	}
}

void ATributeGameMode::HandleTributeEnded(bool bPassed)
{
	if (bRunOver)
	{
		return;
	}
	if (bPassed && Saucer.IsValid() && Mothership.IsValid())
	{
		// The debrief waits for the saucer to be aboard. Watch it rise from far out, the valley falling away.
		Saucer->BeginDocking(Mothership->GetDockPoint());
		ShowShipView(true, 2.f);
		return;
	}
	if (Mothership.IsValid())
	{
		Mothership->Depart();
	}
	ShowShipView(true, 2.f);
	EndRun(ERunEnding::LeftBehind);
}

void ATributeGameMode::HandleDocked()
{
	EndRun(ERunEnding::Docked);
}

void ATributeGameMode::HandlePowerEmpty()
{
	EndRun(ERunEnding::SaucerDown);
}

void ATributeGameMode::EndRun(ERunEnding Ending)
{
	if (bRunOver)
	{
		return;
	}
	bRunOver = true;

	ATributeGameState* State = GetGameState<ATributeGameState>();
	FRunResult Result;
	Result.Ending = Ending;
	if (State)
	{
		Result.Manifest = State->Manifest;
	}
	if (Mothership.IsValid())
	{
		Result.NotesRight = Mothership->Run.NotesRight;
		Result.Misses = Mothership->Run.Misses;
	}
	if (Saucer.IsValid())
	{
		Result.Heat = Saucer->PowerHeat->GetHeat();
	}
	Result.Score = FRunResult::ScoreFor(Ending, Result.Manifest, Result.NotesRight, Result.Misses, Result.Heat);
	UE_LOG(LogTribute, Log, TEXT("The night is over: %s, score %d"), *UEnum::GetValueAsString(Ending), Result.Score);

	// A moment to watch the ending before the debrief comes up.
	FTimerHandle Timer;
	GetWorldTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this, Result]()
	{
		if (APlayerController* Player = GetWorld()->GetFirstPlayerController())
		{
			if (ATributeHUD* HUD = Player->GetHUD<ATributeHUD>())
			{
				HUD->ShowDebrief(Result);
			}
		}
	}), DebriefDelay, false);
}

FTransform ATributeGameMode::FindPoliceStart() const
{
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(PoliceStartTag))
		{
			return It->GetActorTransform();
		}
	}

	// No marked start: come from 100 m down the valley, so it's never on top of the saucer.
	UE_LOG(LogTribute, Warning, TEXT("No actor tagged %s; the police start 100 m from the saucer"), *PoliceStartTag.ToString());
	const FVector From = Saucer.IsValid() ? Saucer->GetActorLocation() : FVector::ZeroVector;
	return FTransform(FVector(From.X - 10000.f, From.Y, 100.f));
}
