// The player's saucer. It hovers a set height over the valley, leans into its movement, and holds a
// beam down that lifts whatever it catches up into the hull. A quick tap of the beam is a pulse, which
// is how the player answers the mothership's calls.
//
// The rules live here; BP_Saucer gives it a mesh, the beam's look and its sounds through the
// Blueprint events at the bottom.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LiftableComponent.h"
#include "SaucerPawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class UInputAction;
class UInputMappingContext;
class USphereComponent;
class USpotLightComponent;
class USpringArmComponent;
class UPointLightComponent;
class UPowerHeatComponent;
class UMaterialInstanceDynamic;
class ATransformer;
class UStaticMeshComponent;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetLifted, AActor*, Target, ETargetKind, Kind);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeamPulsed, FVector, GroundPoint);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaucerDocked);

UCLASS()
class TRIBUTE_API ASaucerPawn : public APawn
{
	GENERATED_BODY()

public:
	ASaucerPawn();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// --- Parts ---

	/** What the saucer bumps into the world with. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saucer")
	TObjectPtr<USphereComponent> Collision;

	/** The hull. It bobs, spins slowly and leans into movement; the rest of the saucer stays level. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saucer")
	TObjectPtr<UStaticMeshComponent> Hull;

	/** The beam's light on the ground. On only while the beam is held, and flashes for a pulse. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saucer")
	TObjectPtr<USpotLightComponent> BeamLight;

	/** Holds the camera high and behind, looking down at a three-quarter angle. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saucer")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saucer")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saucer")
	TObjectPtr<UFloatingPawnMovement> Movement;

	/** Power and Heat. The saucer steps them every frame with what it's doing. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saucer")
	TObjectPtr<UPowerHeatComponent> PowerHeat;

	/** A band around the hull that shows Heat: blue when calm, red when the town is hunting. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saucer")
	TObjectPtr<UStaticMeshComponent> Rim;

	/** Throws the rim's colour on the ground under the saucer, so Heat reads at night. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Saucer")
	TObjectPtr<UPointLightComponent> RimGlow;

	/** The rim's colour at no Heat. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Heat")
	FLinearColor CalmRimColour = FLinearColor(0.1f, 0.45f, 1.f);

	/** The rim's colour at full Heat. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Heat")
	FLinearColor HotRimColour = FLinearColor(1.f, 0.08f, 0.03f);

	/** Shows Power, Heat and the spotlight as text on screen, for tuning. Never in a Shipping build. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Heat")
	bool bShowDebugReadout = false;

	// --- Hover and tilt ---

	/** How high above the ground the saucer floats, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Hover", meta = (ClampMin = "100"))
	float HoverHeight = 900.f;

	/** How quickly it rises and sinks to follow the ground. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Hover", meta = (ClampMin = "0.1"))
	float HoverFollowSpeed = 3.f;

	/** How far the hull bobs up and down while hovering, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Hover")
	float BobHeight = 12.f;

	/** Bobs per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Hover")
	float BobRate = 0.5f;

	/** How fast the hull turns on its own, in degrees per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Hover")
	float HullSpinRate = 25.f;

	/** How far the hull leans at full speed, in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Hover")
	float MaxTilt = 14.f;

	/** How quickly the lean follows the movement. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Hover", meta = (ClampMin = "0.1"))
	float TiltFollowSpeed = 5.f;

	// --- Beam and lift ---

	/** How wide the beam is where it reaches the ground, in cm. Anything liftable inside it can be caught. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Beam", meta = (ClampMin = "50"))
	float BeamRadius = 350.f;

	/** How fast a caught target rises, in cm per second (scaled by its own LiftSpeedScale). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Beam", meta = (ClampMin = "10"))
	float LiftSpeed = 450.f;

	/** How quickly a caught target drifts into the middle of the beam. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Beam")
	float LiftCentreSpeed = 2.5f;

	/** How fast a caught target turns as it rises, in degrees per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Beam")
	float LiftSpinRate = 90.f;

	/** A target this close below the hull is taken aboard, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Beam")
	float TakeAboardDistance = 150.f;

	/** A beam press shorter than this, in seconds, counts as a pulse. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Beam")
	float PulseMaxHoldTime = 0.25f;

	/** The beam light's brightness while held, in candelas. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Beam")
	float BeamBrightness = 40000.f;

	/** How long a pulse's flash lasts, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Beam")
	float PulseFlashTime = 0.15f;

	// --- Input ---
	// Left empty, these are built in code (WASD and the left stick to fly, Space and A for the beam).
	// Assign IA_Move, IA_Beam and IMC_Default in BP_Saucer to override them.

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Saucer|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Saucer|Input")
	TObjectPtr<UInputAction> BeamAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Saucer|Input")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Saucer|Input")
	TObjectPtr<UInputMappingContext> InputMapping;

	// --- What other systems listen to ---

	/** Something reached the hull. The manifest counts it. */
	UPROPERTY(BlueprintAssignable, Category = "Saucer")
	FOnTargetLifted OnTargetLifted;

	/** The beam was tapped. The tribute checks it against the lit sector. */
	UPROPERTY(BlueprintAssignable, Category = "Saucer")
	FOnBeamPulsed OnBeamPulsed;

	/** Turns the beam on, as holding Space or A does. Also for Blueprints and console testing (`ke * StartBeam`). */
	UFUNCTION(BlueprintCallable, Category = "Saucer")
	void StartBeam();

	/** Turns the beam off, as letting go does. If it was on for less than PulseMaxHoldTime, that was a pulse. */
	UFUNCTION(BlueprintCallable, Category = "Saucer")
	void StopBeam();

	/** True while the beam is held. Power drains faster and Heat climbs while it is. */
	UFUNCTION(BlueprintPure, Category = "Saucer")
	bool IsBeamOn() const { return bBeamHeld; }

	/** True while something is rising up the beam. */
	UFUNCTION(BlueprintPure, Category = "Saucer")
	bool IsLifting() const { return LiftTarget.IsValid(); }

	/** Where the beam meets the ground. Cows and people flee from here. */
	UFUNCTION(BlueprintPure, Category = "Saucer")
	FVector GetBeamGroundPoint() const;

	// --- Hooks for BP_Saucer's effects and sounds ---

	UFUNCTION(BlueprintImplementableEvent, Category = "Saucer", meta = (DisplayName = "Beam Started"))
	void BeamStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "Saucer", meta = (DisplayName = "Beam Stopped"))
	void BeamStopped();

	UFUNCTION(BlueprintImplementableEvent, Category = "Saucer", meta = (DisplayName = "Beam Pulsed"))
	void BeamPulsed(FVector GroundPoint);

	UFUNCTION(BlueprintImplementableEvent, Category = "Saucer", meta = (DisplayName = "Lift Started"))
	void LiftStarted(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Saucer", meta = (DisplayName = "Lift Dropped"))
	void LiftDropped(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Saucer", meta = (DisplayName = "Taken Aboard"))
	void TakenAboard(AActor* Target, ETargetKind Kind);

	/** Power ran out: the hum dies and the saucer sinks to the ground. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Saucer", meta = (DisplayName = "Power Failed"))
	void PowerFailed();

	/** Pulls the camera up and back so the whole ring of pools is in view (for the tribute), or returns it. */
	UFUNCTION(BlueprintCallable, Category = "Saucer")
	void SetWideView(bool bWide) { bWideView = bWide; }

	/** The tribute's camera: high and wide enough to see the ring of pools, but under the hull (it hangs at
	 *  120 m; the camera tops out near 100 m). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Camera")
	float WideArmLength = 9500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Camera")
	float WidePitch = -70.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Camera")
	float WideFieldOfView = 85.f;

	/** The tribute was answered: rise into the mothership's hull at DockPoint. The player has no control from here. */
	UFUNCTION(BlueprintCallable, Category = "Saucer")
	void BeginDocking(FVector DockPoint);

	/** The saucer is inside the hull. The debrief comes up on this. */
	UPROPERTY(BlueprintAssignable, Category = "Saucer")
	FOnSaucerDocked OnDocked;

	/** How fast it climbs into the hull, in cm per second. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Saucer|Hover")
	float DockClimbSpeed = 700.f;

	/** True once Power has run out and the saucer is on the ground. */
	UFUNCTION(BlueprintPure, Category = "Saucer")
	bool IsDown() const { return bDown; }

private:
	UFUNCTION()
	void HandlePowerEmpty();

	void UpdatePowerHeat(float DeltaSeconds);
	void UpdateRim();
	ATransformer* FindFeedingTransformer() const;

	void BuildDefaultInput();
	void Move(const FInputActionValue& Value);
	void RequestPause();

	void UpdateHover(float DeltaSeconds);
	void UpdateHull(float DeltaSeconds);
	void UpdateBeamLight(float DeltaSeconds);
	void UpdateLift(float DeltaSeconds);

	/** The nearest liftable thing inside the beam that nothing else is already lifting, or null. */
	ULiftableComponent* FindTargetInBeam() const;
	void StartLift(ULiftableComponent* Target);
	void DropLift();
	void TakeAboard();

	/** Fits the light's cone to the beam's width at the hover height. */
	void FitBeamLight();

	bool bBeamHeld = false;
	float BeamHeldTime = 0.f;
	float PulseFlashLeft = 0.f;
	float HoverTime = 0.f;
	float HullYaw = 0.f;
	FRotator HullTilt = FRotator::ZeroRotator;
	float GroundZ = 0.f;
	bool bHasGround = false;

	TWeakObjectPtr<ULiftableComponent> LiftTarget;

	/** Every transformer in the level, found once at BeginPlay. There are only a handful. */
	TArray<TWeakObjectPtr<ATransformer>> Transformers;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RimTint;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> RimMaterial;

	bool bDown = false;
	bool bDocking = false;
	bool bWideView = false;
	float RaidArmLength = 0.f;
	float RaidPitch = 0.f;
	float RaidFieldOfView = 60.f;
	bool bDocked = false;
	FVector DockTarget = FVector::ZeroVector;
};
