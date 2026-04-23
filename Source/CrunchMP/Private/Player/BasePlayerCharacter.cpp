// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/BasePlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "CrunchMP/CrunchMP.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/AbilitySystemTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CHeroAttributeSet.h"

#include "DebugHelper.h"

ABasePlayerCharacter::ABasePlayerCharacter()
{
	CameraBoom =  CreateDefaultSubobject<USpringArmComponent>("CameraBoom");
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->ProbeChannel = ECC_SpringArm;
	
	PlayerCam = CreateDefaultSubobject<UCameraComponent>("PlayerCamera");
	PlayerCam->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach Cam To SpringArm
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	
	HeroAttributeSet = CreateDefaultSubobject<UCHeroAttributeSet>("Hero Attribute Set");
}

void ABasePlayerCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	
	ClientSideInit();
	
	APlayerController* OwningPlayerController = GetController<APlayerController>();
	if (OwningPlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = OwningPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (Subsystem)
		{
			Subsystem->RemoveMappingContext(PlayerIMC);
			
			Subsystem->AddMappingContext(PlayerIMC, 0);
		}
	}
}

void ABasePlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EIC)
	{
		EIC->BindAction(IA_Jump, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Jump);
		EIC->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Look);
		EIC->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::Move);
		EIC->BindAction(IA_LearnAbilityLeader, ETriggerEvent::Started, this, &ABasePlayerCharacter::LearnAbilityLeaderDown);
		EIC->BindAction(IA_LearnAbilityLeader, ETriggerEvent::Completed, this, &ABasePlayerCharacter::LearnAbilityLeaderUp);
		
		for (const TPair<EAbilityInputID, UInputAction*>& InputActionPair : IA_GameplayAbility)
		{
			EIC->BindAction(InputActionPair.Value, ETriggerEvent::Triggered, this, &ABasePlayerCharacter::HandleAbilityInput, InputActionPair.Key);
		}
	}
}

void ABasePlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookVector = Value.Get<FVector2D>();
	
	AddControllerPitchInput(-LookVector.Y);
	AddControllerYawInput(LookVector.X);
}

void ABasePlayerCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MoveVector = Value.Get<FVector2D>();
	MoveVector.Normalize();
	
	if (Controller)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRot(0, Rotation.Yaw, 0);
		
		const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(RightDir, MoveVector.X);
		AddMovementInput(ForwardDir, MoveVector.Y);
		
	}
}

void ABasePlayerCharacter::LearnAbilityLeaderDown(const FInputActionValue& Value)
{
	bIsLearnAbilityLeaderDown = true;
}

void ABasePlayerCharacter::LearnAbilityLeaderUp(const FInputActionValue& Value)
{
	bIsLearnAbilityLeaderDown = false;
}

void ABasePlayerCharacter::HandleAbilityInput(const FInputActionValue& Value, EAbilityInputID InputID)
{
	bool bPressed  = Value.Get<bool>();
	
	if (bPressed && bIsLearnAbilityLeaderDown)
	{
		UpgradeAbilityWithInputId(InputID);
		return;
	}
	
	if (bPressed)
	{
		GetAbilitySystemComponent()->AbilityLocalInputPressed((int32) InputID);
	}
	else
	{
		GetAbilitySystemComponent()->AbilityLocalInputReleased((int32) InputID);
	}
	if (InputID == EAbilityInputID::BasicAttack)
	{
		FGameplayTag Tag = bPressed ? AbilityTags::BasicAttack_Pressed : AbilityTags::BasicAttack_Released;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, Tag, FGameplayEventData());
		Server_SendGameplayEventToSelf(Tag, FGameplayEventData());
	}
}

void ABasePlayerCharacter::SetInputEnabledFromPlayerController(bool bEnabled)
{
	APlayerController* PC = GetController<APlayerController>();
	if (!PC)
	{
		return;
	}
	if (bEnabled)
	{
		EnableInput(PC);
	}
	else
	{
		DisableInput(PC);
	}
}

void ABasePlayerCharacter::OnStun()
{
	SetInputEnabledFromPlayerController(false);
}

void ABasePlayerCharacter::OnRecoverFromStun()
{
	if (IsDead()) return;
	SetInputEnabledFromPlayerController(true);
}

void ABasePlayerCharacter::OnDead()
{
	SetInputEnabledFromPlayerController(false);
}

void ABasePlayerCharacter::OnRespawn()
{
	SetInputEnabledFromPlayerController(true);
}

void ABasePlayerCharacter::OnAimStateChanged(bool bIsAiming)
{
	if (bIsControlledByLocalPlayer())
		LerpCamToLocalOffSet(bIsAiming ? CamAimLocalOffSet : FVector{0.0f});
}

void ABasePlayerCharacter::LerpCamToLocalOffSet(const FVector& CamOffSet)
{
	GetWorldTimerManager().ClearTimer(CamLerpTimerHandle);
	CamLerpTimerHandle = GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ABasePlayerCharacter::TickCamLocalOffSetLerp, CamOffSet));
}

void ABasePlayerCharacter::TickCamLocalOffSetLerp(FVector CamOffSet)
{
	FVector CurrentLocalOffSet = PlayerCam->GetRelativeLocation(); 
	if (FVector::Dist(CurrentLocalOffSet, CamOffSet) < 1.f)
	{
		PlayerCam->SetRelativeLocation(CamOffSet);
		return;
	}
	
	float LearpAlpha = FMath::Clamp(GetWorld()->GetTimeSeconds() * CamLerpSpeed, 0.f, 1.f);
	FVector NewLocalOffSet = FMath::Lerp(CurrentLocalOffSet, CamOffSet, LearpAlpha);
	PlayerCam->SetRelativeLocation(NewLocalOffSet);
	
	CamLerpTimerHandle = GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &ABasePlayerCharacter::TickCamLocalOffSetLerp, CamOffSet));
}
