// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseRunner.h"
#include "AbilitySystemComponent.h"
#include "AutoTargetSpringArm.h"
#include "RunnerAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"


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

}

// Called every frame
void ABaseRunner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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





