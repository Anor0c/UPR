// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoTargetSpringArm.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

void UAutoTargetSpringArm::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bShouldTarget)
	{
		TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), CameraTarget);
		this->SetRelativeRotation(TargetRotation, false , nullptr, ETeleportType::None); 
		/*SetRelativeRotation();
		Cast<ACharacter>(GetOwner())->GetController()->SetControlRotation(TargetRotation); */
	}
}
