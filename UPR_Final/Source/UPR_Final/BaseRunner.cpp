// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseRunner.h"
#include "AbilitySystemComponent.h"
#include "AutoTargetSpringArm.h"
#include "RunnerAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Curves/CurveFloat.h"
#include "Components/SplineComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Abilities/GameplayAbility.h"
#include "Babylon.h"


// Sets default values
ABaseRunner::ABaseRunner()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//Setup GASComponent
	GASComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("GameplayAbilitySystemComponent"));

	//Setup CameraBoom
	/*AutoTargetCameraBoom = CreateDefaultSubobject<UAutoTargetSpringArm>(TEXT("Camera Boom"));
	AutoTargetCameraBoom->TargetArmLength = 375.0f;
	AutoTargetCameraBoom->bAutoActivate = true;
	AutoTargetCameraBoom->SocketOffset = FVector(0.0f,0.0f,110.0f);*/
}

// Called when the game starts or when spawned
void ABaseRunner::BeginPlay()
{
	if (IsValid(GASComponent))
	{
		RunnerAttributeSet = GASComponent->GetSet<URunnerAttributeSet>();
	}

	Super::BeginPlay();

	//Init Gameplay Ability System : chaque ability du moveset est donnée avec son index comme InputID
	if (IsValid(RunnerAttributeSet) && IsValid(GASComponent))
	{
		for (int32 AbilityIndex = 0; AbilityIndex < RunnerMoveset.Num(); ++AbilityIndex)
		{
			GASComponent->GiveAbility(FGameplayAbilitySpec(RunnerMoveset[AbilityIndex], 0, AbilityIndex));
		}
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, UKismetSystemLibrary::GetDisplayName(this) + TEXT("GAS COMPONENT NULL REFERENCE "),
			true, true, FLinearColor(1.0f, 0.0f, 0.039216f), 10.0f);
	}

	//Cache Base Variables : vitesse, camera et FOV du palier de depart
	auto GetCapValue = [](const TArray<float>& Caps, int32 Index)
	{
		return Caps.IsValidIndex(Index) ? Caps[Index] : 0.f;
	};
	CurrentSpeedCap = GetCapValue(SpeedCapsArray, StartingIndex);
	GetCharacterMovement()->MaxWalkSpeed = CurrentSpeedCap;
	MovementMode = ERunnerMovementMode::Idle;
	//Les composants camera sont ajoutés dans le Blueprint
	if (USpringArmComponent* CameraBoom = FindComponentByClass<UAutoTargetSpringArm>())
	{
		CameraBoom->TargetArmLength = GetCapValue(TargetArmLengthArray, StartingIndex);
	}
	if (UCameraComponent* Camera = FindComponentByClass<UCameraComponent>())
	{
		Camera->SetFieldOfView(GetCapValue(CameraFOVCaps, StartingIndex));
	}
	OnSpeedCapPassed(1);
	UpdateAdrenaline(0.0f);

	//Recopie les combos de ComboMap
	ComboMap.GenerateKeyArray(SpeedCapUnloccked);
	TArray<int32> ComboValues;
	ComboMap.GenerateValueArray(ComboValues);
	//Equivalent du noeud Set Array Elem avec Size To Fit
	auto SetSizeToFit = [this](int32 Index, int32 Item)
	{
		if (Index < 0) { return; }
		if (Index >= SpeedValueToNextCap.Num()) { SpeedValueToNextCap.SetNum(Index + 1); }
		SpeedValueToNextCap[Index] = Item;
	};
	for (int32 ComboIndex = 0; ComboIndex < ComboValues.Num(); ++ComboIndex)
	{
		if (ComboValues.IsValidIndex(ComboIndex - 1))
		{
			SetSizeToFit(ComboValues[ComboIndex] + ComboValues[ComboIndex - 1], ComboIndex);
		}
		else
		{
			SetSizeToFit(ComboIndex, ComboValues[ComboIndex]);
		}
	}
	InitPostProcess();

	//Event dispatchers et compte a rebours
	BeginPlayAssignEventDispatcher();
	OnStartCountDown();
	//MaxSpeedSound est un composant ajouté dans le Blueprint, retrouvé par son nom
	TArray<UAudioComponent*> AudioComponents;
	GetComponents<UAudioComponent>(AudioComponents);
	for (UAudioComponent* AudioComponent : AudioComponents)
	{
		if (AudioComponent->GetFName() == TEXT("MaxSpeedSound"))
		{
			AudioComponent->Stop();
		}
	}

	//Les traces de debug ne sont dessinées que dans l'editeur
	DEBUGDrawType = UBabylon::IsInEditor() ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
}

void ABaseRunner::OnSpeedCapPassed_Implementation(int32 NewSpeedCapIndex)
{
}

void ABaseRunner::UpdateAdrenaline_Implementation(float DeltaAdrenaline)
{
}

void ABaseRunner::BeginPlayAssignEventDispatcher_Implementation()
{
}

void ABaseRunner::OnStartCountDown_Implementation()
{
}

// Called every frame
void ABaseRunner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UCharacterMovementComponent* RunnerMovement = GetCharacterMovement();

	//Etat au sol / en l'air
	if (RunnerMovement->IsMovingOnGround())
	{
		//Au sol : Running ou Idle selon la vitesse horizontale
		MovementMode = FVector2D(RunnerMovement->Velocity.X, RunnerMovement->Velocity.Y).Size() > 5.0
			? ERunnerMovementMode::Running
			: ERunnerMovementMode::Idle;
	}
	else
	{
		TickInAir(RunnerMovement);
	}

	//Wallrun : le saut et l'air dash restent possibles, et le wallrun est limité dans le temps
	if (IsWallRunning)
	{
		JumpPossible = true;
		CanAirDash = true;
		WallRunCurrentTime -= UGameplayStatics::GetWorldDeltaSeconds(this);
		if (WallRunCurrentTime <= 0.f)
		{
			CanWallRun = false;
		}
	}
	if (IsGrinding)
	{
		PerformGrind();
	}

	//Diminution de la vitesse accumulée
	SpeedValue = FMath::Clamp(SpeedValue - UGameplayStatics::GetWorldDeltaSeconds(this) * SpeedDecayMultiplayer, 0.0f, 99999.0f);
	SpeedValueUpdated.Broadcast(SpeedValue);

	//Suspension (REVOIR SUSPENDED SYSTEM)
	if (IsSuspended)
	{
		RunnerMovement->GravityScale = 0.0f;
		RunnerMovement->Velocity = FVector::ZeroVector;
		CanMove = false;
	}
	//blend cam 
	if (IsCameraBlending)
	{
		SpeedCapCamera();
	}

	//Boussole : objectif a GoalLocationOffset devant le runner sur la spline de course (derriere si IsRetour)
	if (RaceManagerRef)
	{
		if (USplineComponent* RaceSpline = RaceManagerRef->FindComponentByClass<USplineComponent>())
		{
			const float GoalDistance = IsRetour ? DistanceAlongRace - GoalLocationOffset : DistanceAlongRace + GoalLocationOffset;
			GoalLocation = RaceSpline->GetLocationAtDistanceAlongSpline(FMath::Clamp(GoalDistance, 0.0f, RaceSpline->GetSplineLength()), ESplineCoordinateSpace::World);
			LookAtGoal = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), GoalLocation);
			BoussoleUpdated.Broadcast();
		}
	}

	UpdatePostProcess();

	//Rotation du mesh : en course, il s'oriente selon l'input, sinon il revient a -90
	USkeletalMeshComponent* RunnerMesh = GetMesh();
	if (MovementMode == ERunnerMovementMode::Running)
	{
		const bool bInputTurning = PlayerMoveInput.Y <= 0.1 && !PlayerMoveInput.IsZero();
		MeshRotationTarget = bInputTurning ? PlayerMoveInput.X * -90.0f + 90.0f : -90.0f;

		const FRotator MeshRelativeRotation = RunnerMesh->GetRelativeRotation();
		if (FMath::IsNearlyEqual(MeshRotationTarget, MeshRelativeRotation.Yaw, 0.1f))
		{
			MeshRotationAlpha = 0.0f;
		}
		else
		{
			//Plus le mesh est deja aligné sur la cible, plus l'alpha monte vite
			const float Alignment = FVector::DotProduct(FRotator(0.0f, MeshRotationTarget, 0.0f).Vector(), MeshRelativeRotation.Vector());
			const float AlphaStep = FMath::Clamp(UGameplayStatics::GetWorldDeltaSeconds(this) * MeshRotationSpeed + Alignment * 0.01f, 0.01f, 1.0f);
			MeshRotationAlpha = FMath::Clamp(AlphaStep + MeshRotationAlpha, 0.0f, 1.0f);
		}

		const FRotator NewMeshRotation = UKismetMathLibrary::RLerp(FRotator(0.0f, MeshRelativeRotation.Yaw, 0.0f), FRotator(0.0f, MeshRotationTarget, 0.0f), MeshRotationAlpha, true);
		RunnerMesh->SetRelativeRotation(NewMeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		RunnerMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void ABaseRunner::TickInAir(UCharacterMovementComponent* RunnerMovement)
{
	JumpPossible = false;

	if ((MovementMode == ERunnerMovementMode::Running || MovementMode == ERunnerMovementMode::Idle) && MovementMode != ERunnerMovementMode::Grinding)
	{
		MovementMode = ERunnerMovementMode::Falling;
	}

	if (WasGroundLastFrame)
	{
		LastGroundPosition = RunnerMovement->GetLastUpdateLocation();
		WasGroundLastFrame = false;
	}

	if (RunnerMovement->Velocity.Z < 0.0)
	{
		IsFalling = true;
	}

	if (IsFalling)
	{
		//Pas d'acceleration de la gravité tant que le composant de flottement flotte. IsFloating est une variable Blueprint, lue par réflexion
		const UActorComponent* FloatingComponent = FloatingComponentClass ? FindComponentByClass(FloatingComponentClass) : nullptr;
		const FBoolProperty* IsFloatingProperty = FloatingComponent ? FindFProperty<FBoolProperty>(FloatingComponent->GetClass(), TEXT("IsFloating")) : nullptr;
		const bool IsFloating = IsFloatingProperty && IsFloatingProperty->GetPropertyValue_InContainer(FloatingComponent);
		if (!IsFloating)
		{
			GravityAcceleration();
		}
	}

	FVector ClimbTarget;
	const EClimbHeight ClimbResult = TryClimb(ClimbTarget);
	if (ClimbResult != EClimbHeight::NoTraversal)
	{
		PerformClimb(ClimbTarget, ClimbResult);
	}
	else if (CanWallRun && !IsWallRunning)
	{
		TryWallRun();
	}
	else
	{
		EndWallRun();
	}
}

// Called to bind functionality to input
void ABaseRunner::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
void ABaseRunner::CancelSpecific(ECapacityTypeInternal CapacityToCancel)
{
	OnCancelSpecificAbilityDelegate.Broadcast(CapacityToCancel);
}

EClimbHeight ABaseRunner::TryClimb(FVector& OutPos)
{
	OutPos = FVector::ZeroVector;

	int32 LastHitIndex = 0;
	FVector LastHitLocation = FVector::ZeroVector;
	int32 NoHitCombIndex = 0;
	TArray<FHitResult> RaycastCombHit;
	const int32 HighClimbThreshold = 5;
	bool LastHitSet = false;

	if (IsWallRunning || IsGrinding || !CanTraversalAction)
	{
		return EClimbHeight::NoTraversal;
	}
	CanTraversalAction = false;

	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	const FVector ActorLocation = GetActorLocation();
	const FRotator ActorRotation = GetActorRotation();
	const FVector CapsuleForward = Capsule->GetForwardVector();
	const TArray<AActor*> ActorsToIgnore;

	//Launch RayCast Comb and Cache RayCastHitInfo
	for (int32 Index = 0; Index < TraversalDetectionVerticalPositions.Num(); ++Index)
	{
		const FVector RotatedPosition = ActorRotation.RotateVector(TraversalDetectionVerticalPositions[Index]);
		const FVector Start = RotatedPosition + ActorLocation;
		const FVector End = RotatedPosition + CapsuleForward * TraversalDetectionLength + ActorLocation;
		const FLinearColor TraceColor = Index >= HighClimbThreshold ? FLinearColor(1.f, 0.f, 0.103594f) : FLinearColor(1.f, 0.529412f, 0.f);

		FHitResult Hit;
		UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
			DEBUGDrawType, Hit, true, TraceColor, FLinearColor::Green, 5.f);
		RaycastCombHit.Add(Hit);
	}

	for (int32 Index = 0; Index < RaycastCombHit.Num(); ++Index)
	{
		const FHitResult& Hit = RaycastCombHit[Index];
		if (Hit.bBlockingHit)
		{
			//Cache la position plus basse qui hit un obstacle
			if (Index > LastHitIndex)
			{
				LastHitSet = true;
				LastHitIndex = Index;
				LastHitLocation = Hit.Location;
			}
		}
		//Cache l'index du ray qui n'a rien touché le plus bas
		else if (Index > NoHitCombIndex && Index > LastHitIndex && LastHitSet)
		{
			NoHitCombIndex = Index;
			break;
		}
	}

	if (NoHitCombIndex >= RaycastCombHit.Num() - 1 || NoHitCombIndex <= 1)
	{
		CanTraversalAction = true;
		return EClimbHeight::NoTraversal;
	}

	if (DEBUGCanPrint)
	{
		const EClimbHeight DebugClimbHeight = LastHitIndex >= HighClimbThreshold ? EClimbHeight::High : EClimbHeight::Low;
		const FString DebugString = FString::Printf(TEXT("\r\nComb Last Index is%d\r\nLast NoHitIndex is  %d\r\nPlayer moved to : %s\r\nClimb Height is : %s"),
			RaycastCombHit.Num() - 1,
			LastHitIndex,
			*(LastHitLocation + CapsuleForward * 100.f).ToString(),
			*StaticEnum<EClimbHeight>()->GetNameStringByValue(static_cast<int64>(DebugClimbHeight)));
		UKismetSystemLibrary::PrintString(this, DebugString, true, true, FLinearColor(1.f, 0.529412f, 0.f), 2.f);
		UKismetSystemLibrary::DrawDebugArrow(this, ActorLocation, LastHitLocation, 0.f, FLinearColor(1.f, 0.52549f, 0.f), 5.f, 0.f);
	}

	//Trace check if place vertical
	const FVector HasEnoughPlaceVertical = TraversalDetectionVerticalPositions[LastHitIndex] + ActorLocation
		+ FVector(0.f, 0.f, Capsule->GetScaledCapsuleHalfHeight() + 30.f);
	const FVector HasEnoughPlaceHorizontal = CapsuleForward * 100.f;
	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	const FLinearColor CapsuleTraceColor(1.f, 0.52549f, 0.f);

	FHitResult VerticalHit;
	const bool VerticalBlocked = UKismetSystemLibrary::CapsuleTraceSingle(this, ActorLocation, HasEnoughPlaceVertical,
		CapsuleRadius, CapsuleHalfHeight, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
		EDrawDebugTrace::ForDuration, VerticalHit, true, CapsuleTraceColor, FLinearColor::Green, 5.f);

	FHitResult HorizontalHit;
	const bool HorizontalBlocked = UKismetSystemLibrary::CapsuleTraceSingle(this, HasEnoughPlaceVertical, HasEnoughPlaceVertical + HasEnoughPlaceHorizontal,
		CapsuleRadius, CapsuleHalfHeight, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
		EDrawDebugTrace::ForDuration, HorizontalHit, true, CapsuleTraceColor, FLinearColor::Green, 5.f);

	if (HorizontalBlocked || VerticalBlocked)
	{
		return EClimbHeight::NoTraversal;
	}

	MovementMode = ERunnerMovementMode::TraversalAction;
	ClimbHeight = LastHitIndex >= HighClimbThreshold ? EClimbHeight::High : EClimbHeight::Low;
	OutPos = HasEnoughPlaceVertical + HasEnoughPlaceHorizontal;
	return ClimbHeight;
}

void ABaseRunner::PerformClimb(FVector LocationTarget, EClimbHeight InClimbHeight)
{
	switch (InClimbHeight)
	{
	case EClimbHeight::Low:
		MaxTraversalActionTimer = 0.4;
		break;
	case EClimbHeight::High:
		MaxTraversalActionTimer = 0.6;
		break;
	default:
		MaxTraversalActionTimer = 0.0;
		break;
	}
	TraversalTarget = LocationTarget;
	CanWallRun = false;

	if (TraversalActionTimer > MaxTraversalActionTimer)
	{
		TraversalActionTimer = 0.0;
		SetActorLocation(TraversalTarget, false, nullptr, ETeleportType::TeleportPhysics);
		CanMove = true;
		CanWallRun = true;
		CanTraversalAction = true;
		MovementMode = ERunnerMovementMode::Idle;
		return;
	}

	TraversalActionTimer += GetWorld()->GetDeltaSeconds();
	UCharacterMovementComponent* RunnerMovement = GetCharacterMovement();
	RunnerMovement->Velocity = FVector::ZeroVector;
	RunnerMovement->GravityScale = 0.f;
	CanMove = false;

	GetWorldTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &ABaseRunner::PerformClimb, TraversalTarget, InClimbHeight));
}

void ABaseRunner::PerformWallRun(EWallRunSide ConnectedSide, FHitResult InHit)
{
	UCharacterMovementComponent* RunnerMovement = GetCharacterMovement();

	//Sortie du wallrun en chute, le wallrun est bloqué 0.5s
	auto StopWallRunFalling = [this]()
	{
		MovementMode = ERunnerMovementMode::Falling;
		IsWallRunning = false;
		IsFalling = true;
		CanWallRun = false;
		//Comme le node Delay, un appel pendant le compte a rebours est ignoré
		if (!GetWorldTimerManager().IsTimerActive(WallRunCooldownHandle))
		{
			GetWorldTimerManager().SetTimer(WallRunCooldownHandle,
				FTimerDelegate::CreateWeakLambda(this, [this]() { CanWallRun = true; }), 0.5f, false);
		}
	};

	if (JumpInputPressed)
	{
		StopWallRunFalling();
		return;
	}

	if (RunnerMovement->IsMovingOnGround())
	{
		MovementMode = ERunnerMovementMode::Running;
		IsWallRunning = false;
		return;
	}

	WallConnectedSide = ConnectedSide;

	const FVector UpNormal = UKismetMathLibrary::Normal(GetActorUpVector());
	const FVector HeightOffset(0.f, 0.f, 20.f);

	//Oriente le runner le long du mur. Les points de detection utilisent la rotation mise a jour
	switch (WallConnectedSide)
	{
	case EWallRunSide::Left:
		//Left Side hit
		SetActorRotation(UKismetMathLibrary::MakeRotFromYZ(FVector(InHit.ImpactNormal.X, InHit.ImpactNormal.Y, 0.f), UpNormal), ETeleportType::TeleportPhysics);
		DetectBehind = GetActorLocation() + GetActorRightVector() * CapsuleXYOffset * -1.f + HeightOffset;
		DetectForwards = GetActorLocation() + GetActorForwardVector() * 75.f
			+ GetActorRightVector() * CapsuleXYOffset * -1.f * WallRunForwardDetectOffest + HeightOffset;
		break;
	case EWallRunSide::Right:
		//Right Side Hit
		SetActorRotation(UKismetMathLibrary::MakeRotFromYZ(FVector(-InHit.ImpactNormal.X, -InHit.ImpactNormal.Y, 0.f), UpNormal), ETeleportType::TeleportPhysics);
		DetectBehind = GetActorLocation() + GetActorRightVector() * CapsuleXYOffset + HeightOffset;
		DetectForwards = GetActorLocation() + GetActorForwardVector() * 75.f
			+ GetActorRightVector() * CapsuleXYOffset * WallRunForwardDetectOffest + HeightOffset;
		break;
	default:
		//None et Both ne sont pas branchés dans le Blueprint
		return;
	}

	const FLinearColor TraceColor = WallConnectedSide == EWallRunSide::Left ? FLinearColor(0.072288f, 1.f, 0.f) : FLinearColor(1.f, 0.f, 0.375776f);
	const FLinearColor TraceHitColor(0.f, 0.152941f, 1.f);
	const TArray<AActor*> ActorsToIgnore;

	FHitResult BehindHit;
	const bool BehindBlocked = UKismetSystemLibrary::CapsuleTraceSingle(this, GetActorLocation(), DetectBehind,
		WallRunCapsuleRadius, WallRunCapsuleHalfHeight, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
		DEBUGDrawType, BehindHit, true, TraceColor, TraceHitColor, 5.f);

	FHitResult ForwardHit;
	const bool ForwardBlocked = UKismetSystemLibrary::CapsuleTraceSingle(this, GetActorLocation(), DetectForwards,
		WallRunCapsuleRadius, WallRunCapsuleHalfHeight, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
		DEBUGDrawType, ForwardHit, true, TraceColor, TraceHitColor, 5.f);

	const double HorizontalSpeed = FVector2D(RunnerMovement->Velocity.X, RunnerMovement->Velocity.Y).Size();
	if (!((ForwardBlocked || BehindBlocked) && HorizontalSpeed > 100.0))
	{
		StopWallRunFalling();
		return;
	}

	RunnerMovement->Velocity = HorizontalSpeed * GetActorForwardVector();
	RunnerMovement->GravityScale = 0.f;
	IsWallRunning = true;
	JumpPossible = true;

	//Le hit arriere est prioritaire sur le hit avant
	WallRunInfo = BehindBlocked ? BehindHit : ForwardHit;
	GetWorldTimerManager().SetTimerForNextTick(
		FTimerDelegate::CreateUObject(this, &ABaseRunner::PerformWallRun, WallConnectedSide, WallRunInfo));
}

void ABaseRunner::EndWallRun()
{
	GetCharacterMovement()->GravityScale = DefaultGravityScale;
	IsWallRunning = false;
	JumpPossible = false;
}

void ABaseRunner::TryWallRun()
{
	if (IsGrinding || !CanWallRun)
	{
		IsWallRunning = false;
		if (DEBUGCanPrint)
		{
			UKismetSystemLibrary::PrintString(this, TEXT("Wallrun return NULL"), true, true, FLinearColor(0.329412f, 1.f, 0.f), 2.f);
		}
		return;
	}

	//Cache Values
	const FVector ZOffset(0.f, 0.f, CapsuleZOffset);
	const FVector RightCapsulePos = GetActorLocation() + GetActorRightVector() * CapsuleXYOffset + ZOffset;
	const FVector LeftCapsulePos = GetActorLocation() + GetActorRightVector() * CapsuleXYOffset * -1.f + ZOffset;

	//Trace Left and Right from player
	const FLinearColor TraceHitColor(0.f, 0.152941f, 1.f);
	const TArray<AActor*> ActorsToIgnore;

	FHitResult RightHit;
	const bool RightWallDetect = UKismetSystemLibrary::CapsuleTraceSingle(this, GetActorLocation(), RightCapsulePos,
		WallRunCapsuleRadius, WallRunCapsuleHalfHeight, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
		DEBUGDrawType, RightHit, true, FLinearColor(1.f, 0.023529f, 0.607843f), TraceHitColor, 5.f);

	FHitResult LeftHit;
	const bool LeftWallDetect = UKismetSystemLibrary::CapsuleTraceSingle(this, GetActorLocation(), LeftCapsulePos,
		WallRunCapsuleRadius, WallRunCapsuleHalfHeight, ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
		DEBUGDrawType, LeftHit, true, FLinearColor(0.329412f, 1.f, 0.f), TraceHitColor, 5.f);

	EWallRunSide DetectEnum;
	if (LeftWallDetect && RightWallDetect)
	{
		DetectEnum = EWallRunSide::Both;
	}
	else if (LeftWallDetect)
	{
		DetectEnum = EWallRunSide::Left;
	}
	else if (RightWallDetect)
	{
		DetectEnum = EWallRunSide::Right;
	}
	else
	{
		DetectEnum = EWallRunSide::None;
	}

	if (DetectEnum == EWallRunSide::None || DetectEnum == EWallRunSide::Both)
	{
		IsFalling = true;
		IsWallRunning = false;
		return;
	}

	GetCharacterMovement()->AddImpulse(GetActorForwardVector() * WallJumpImpulse, true);
	UpdateComboValues(5.f, 2.f);
	PerformWallRun(DetectEnum, DetectEnum == EWallRunSide::Left ? LeftHit : RightHit);
	WallConnectedSide = DetectEnum;
	MovementMode = ERunnerMovementMode::WallRunning;
	IsWallRunning = true;
	ResetCam();
}

void ABaseRunner::ResetCam()
{
	if (!(IsGrinding || IsWallRunning))
	{
		ResetCameraRotationAlpha = 0.f;
		CanMoveCamera = true;
		return;
	}

	CanMoveCamera = false;
	ResetCameraRotationAlpha = FMath::Clamp(UGameplayStatics::GetWorldDeltaSeconds(this) * ResetCamSpeed + ResetCameraRotationAlpha, 0.f, 1.f);

	//Cible : seulement le yaw du mesh
	const USkeletalMeshComponent* RunnerMesh = GetMesh();
	const float MeshYaw = UKismetMathLibrary::MakeRotFromZX(RunnerMesh->GetUpVector(), RunnerMesh->GetRightVector()).Yaw;
	const FRotator TargetRotation(0.f, MeshYaw, 0.f);
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, PlayerIndex))
	{
		PlayerController->SetControlRotation(UKismetMathLibrary::RLerp(GetControlRotation(), TargetRotation, ResetCameraRotationAlpha, true));
	}

	if (ResetCameraRotationAlpha >= 1.f)
	{
		ResetCameraRotationAlpha = 0.f;
		CanMoveCamera = true;
		return;
	}

	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ABaseRunner::ResetCam));
}

void ABaseRunner::SpeedCapCamera()
{
	//Comme le node Get du Blueprint : un index invalide renvoie 0
	auto GetCapValue = [](const TArray<float>& Caps, int32 Index)
	{
		return Caps.IsValidIndex(Index) ? Caps[Index] : 0.f;
	};

	//Les composants camera sont ajoutés dans le Blueprint
	if (USpringArmComponent* CameraBoom = FindComponentByClass<UAutoTargetSpringArm>())
	{
		CameraBoom->TargetArmLength = FMath::Lerp(GetCapValue(TargetArmLengthArray, OldSpeedCapIndex),
			GetCapValue(TargetArmLengthArray, CurrentSpeedCapIndex), CurrentCameraBlendTime);
	}
	if (UCameraComponent* Camera = FindComponentByClass<UCameraComponent>())
	{
		Camera->SetFieldOfView(FMath::Lerp(GetCapValue(CameraFOVCaps, OldSpeedCapIndex),
			GetCapValue(CameraFOVCaps, CurrentSpeedCapIndex), CurrentCameraBlendTime));
	}

	CurrentCameraBlendTime += UGameplayStatics::GetWorldDeltaSeconds(this);
	if (CurrentCameraBlendTime >= MaxCameraBlendTime)
	{
		IsCameraBlending = false;
		CurrentCameraBlendTime = 0.f;
	}
}

void ABaseRunner::GravityAcceleration()
{
	FallingTime += UGameplayStatics::GetWorldDeltaSeconds(this);
	//Comme dans le Blueprint, une courbe non assignée donne une gravité de 0
	GetCharacterMovement()->GravityScale = IsValid(GravityCurve) ? GravityCurve->GetFloatValue(FallingTime) : 0.f;
}

void ABaseRunner::EndGrind_Implementation()
{
	if (!IsValid(RailToGrind))
	{
		return;
	}

	UCharacterMovementComponent* RunnerMovement = GetCharacterMovement();
	RunnerMovement->Velocity = FVector::ZeroVector;

	RunnerMovement->AddImpulse(GetCapsuleComponent()->GetForwardVector() * GrindSpeed, true);
	CanAirDash = true;
	CanWallRun = true;
	RunnerMovement->bUseControllerDesiredRotation = true;
	IsGrinding = false;


	OnGrindEnded.Broadcast(RailToGrind);
	RailToGrind = nullptr;
}

void ABaseRunner::PerformGrind_Implementation()
{
	USplineComponent* RailSpline = IsValid(RailToGrind) ? RailToGrind->FindComponentByClass<USplineComponent>() : nullptr;
	if (!RailSpline)
	{
		MovementMode = ERunnerMovementMode::Falling;
		EndGrind();
		return;
	}

	MovementMode = ERunnerMovementMode::Grinding;
	//Comme dans le Blueprint, une courbe non assignée donne une vitesse de 0
	GrindSpeed = IsValid(GrindSpeedCurve) ? GrindSpeedCurve->GetFloatValue(CurrentSpeedCap) : 0.f;

	//Update Runner Location
	const FVector RailLocation = RailSpline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	const FVector RailUp = RailSpline->GetUpVectorAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World);
	SetActorLocation(RailLocation + RailUp * GrindZOffset, true);

	//Update Distance Along Grind et Update Rotation
	const float DeltaSeconds = UGameplayStatics::GetWorldDeltaSeconds(this);
	if (IsFolllowingSplineDirection)
	{
		DistanceAlongSpline += DeltaSeconds * GrindSpeed;
		SetActorRotation(RailSpline->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World));
	}
	else
	{
		DistanceAlongSpline += DeltaSeconds * -1.f * GrindSpeed;
		FVector RailX, RailY, RailZ;
		UKismetMathLibrary::BreakRotIntoAxes(RailSpline->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World), RailX, RailY, RailZ);
		//Demi-tour : le runner regarde dans le sens inverse de la spline
		SetActorRotation(UKismetMathLibrary::MakeRotFromXY(-RailX, -RailY));
	}

	//End Grind
	if (DistanceAlongSpline > RailSpline->GetSplineLength() || DistanceAlongSpline < 0.5f)
	{
		MovementMode = ERunnerMovementMode::Falling;
		EndGrind();
	}
}

void ABaseRunner::TryGrind_Implementation(AActor* InRailToGrind)
{
	USplineComponent* RailSpline = IsValid(InRailToGrind) ? InRailToGrind->FindComponentByClass<USplineComponent>() : nullptr;
	if (!RailSpline)
	{
		EndGrind();
		return;
	}

	//Place le runner sur la spline
	RailToGrind = InRailToGrind;
	RunnerSplineLocation = RailSpline->FindLocationClosestToWorldLocation(GetActorLocation(), ESplineCoordinateSpace::World);
	//Comme dans le Blueprint : rotation au debut de la spline, en espace local
	SplineRotationAtRunnerLocation = RailSpline->GetRotationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::Local);
	DistanceAlongSpline = RailSpline->GetDistanceAlongSplineAtLocation(RunnerSplineLocation, ESplineCoordinateSpace::World);
	MovementMode = ERunnerMovementMode::Grinding;

	AttachToActor(RailToGrind, FAttachmentTransformRules::KeepWorldTransform);

	//Le sens du grind suit la direction de la camera par rapport a la spline
	const USceneComponent* CameraBoom = FindComponentByClass<UAutoTargetSpringArm>();
	const FVector CameraDirection = CameraBoom ? CameraBoom->GetComponentRotation().Vector() : FVector::ZeroVector;
	const FVector SplineDirection = RailSpline->GetRotationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World).Vector();
	//Comme dans le Blueprint, divisé par la somme des normes et non leur produit. Seul le signe est utilisé
	const double PlayerRotationDotProduct = FVector::DotProduct(CameraDirection, SplineDirection) / (CameraDirection.Size() + SplineDirection.Size());
	IsFolllowingSplineDirection = PlayerRotationDotProduct >= 0.0;

	CanWallRun = false;
	GrindCamBlendAmount = 0.f;
	IsGrinding = true;
	UpdateComboValues(5.f, 0.f);

	//IsVertical est une variable de BP_GrindRailBase, lue par réflexion
	const FBoolProperty* IsVerticalProperty = FindFProperty<FBoolProperty>(RailToGrind->GetClass(), TEXT("IsVertical"));
	if (IsVerticalProperty && IsVerticalProperty->GetPropertyValue_InContainer(RailToGrind))
	{
		IsFolllowingSplineDirection = true;
	}
	else
	{
		ResetCam();
	}
}

void ABaseRunner::IncrementCombo()
{
	++TrickCombo;
	IsTricking = false;
	ComboUpdated.Broadcast(TrickCombo);
}

void ABaseRunner::UpdateComboValues(float speedAdded, float adrenalineAdded)
{
	IncrementCombo();

	//Update Expected Speed
	CurrentComboExpectedSpeedValue = speedAdded * SpeedComboScaling + CurrentComboExpectedSpeedValue;
	ExpectedSpeedUpdated.Broadcast(CurrentComboExpectedSpeedValue);

	//Update Expected Adrenaline
	CurrentComboExpectedAdrenaline = adrenalineAdded * AdrenalineComboScaling + CurrentComboExpectedAdrenaline;
	ExpectedAdrenalineUpdated.Broadcast(CurrentComboExpectedAdrenaline);
}

void ABaseRunner::OnOutOfBounds()
{
	OnOutOfBoundsDelegate.Broadcast();
}

void ABaseRunner::OnAbilityLaunched(ECapacityTypeInternal LaunchedAbility)
{
	OnAbilityLaunchedDelegate.Broadcast(LaunchedAbility, this);  
}

void ABaseRunner::CancelAllAbility()
{
	OnCancelAllAbilitiesDelegate.Broadcast();
}

void ABaseRunner::UpdatePostProcess_Implementation()
{
}

void ABaseRunner::InitPostProcess_Implementation()
{
}





