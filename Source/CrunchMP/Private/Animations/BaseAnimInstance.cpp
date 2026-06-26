// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/BaseAnimInstance.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/AbilitySystemTags.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBaseAnimInstance::NativeInitializeAnimation()
{
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter)
	{
		OwnerMovementComp = OwnerCharacter->GetCharacterMovement();
	}
	
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TryGetPawnOwner());
	if (OwnerASC)
	{
		OwnerASC->RegisterGameplayTagEvent(AbilityTags::Status_Aim).AddUObject(this, &UBaseAnimInstance::OwnerAimTagChanged);
	}
}

void UBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (OwnerCharacter)
	{
		FVector Velocity = OwnerCharacter->GetVelocity();
		Speed = Velocity.Length();
		FRotator BodyRot = OwnerCharacter->GetActorRotation();
		FRotator BodyRotDelta = UKismetMathLibrary::NormalizedDeltaRotator(BodyRot, BodyPreviousRotation);
		BodyPreviousRotation = BodyRot;
		
		YawSpeed = BodyRotDelta.Yaw / DeltaSeconds;
		float YawLerpSpeed = YawSpeedSmoothLerpSpeed;
		if (YawLerpSpeed == 0)
		{
			YawLerpSpeed = YawSpeedLerpToZeroSpeed;
		}
		SmoothedYawSpeed = UKismetMathLibrary::FInterpTo(SmoothedYawSpeed, YawSpeed, DeltaSeconds, YawLerpSpeed); //SmootherLean
		FRotator ControlRot = OwnerCharacter->GetBaseAimRotation();
		LookRotOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRot, BodyRot);
		
		ForwardSpeed = Velocity.Dot(ControlRot.Vector());
		RightSpeed = -Velocity.Dot(ControlRot.Vector().Cross(FVector::UpVector));
	}
	if (OwnerCharacter)
	{
		bISJumping = OwnerMovementComp->IsFalling();
	}
}

void UBaseAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	
}

bool UBaseAnimInstance::ShouldDoFullBody() const
{
	return (GetSpeed() <= 0) && !(GetIsAiming());
}

void UBaseAnimInstance::OwnerAimTagChanged(const FGameplayTag Tag, const int32 NewCount)
{
	bIsAiming = NewCount != 0;
}
