// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "InputActionValue.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "BasePlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class ABasePlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	
public:
	ABasePlayerCharacter();
	virtual void PawnClientRestart() override; // First Time Character is Spawned in Client Side.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

private:
	
#pragma region Components
	UPROPERTY(VisibleDefaultsOnly, Category = "Camera")
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleDefaultsOnly, Category = "Camera")
	class UCameraComponent* PlayerCam;
#pragma endregion
	
//Gameplay Ability
	virtual void OnAimStateChanged(bool bIsAiming) override;
	UPROPERTY()
	class UCHeroAttributeSet* HeroAttributeSet;
	
#pragma region Inputs
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Jump;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Look;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Move;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_LearnAbilityLeader;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_UseInventoryItem;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TMap<EAbilityInputID, class UInputAction*> IA_GameplayAbility;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* PlayerIMC;
	
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void LearnAbilityLeaderDown(const FInputActionValue& Value);
	void LearnAbilityLeaderUp(const FInputActionValue& Value);
	void UseInventoryItem(const FInputActionValue& Value);
	bool bIsLearnAbilityLeaderDown = false;
	
	void HandleAbilityInput(const FInputActionValue& Value, EAbilityInputID InputID);
	void SetInputEnabledFromPlayerController(bool bEnabled);
	
#pragma endregion
	
//Stun
virtual void OnStun() override;
virtual void OnRecoverFromStun() override;
	
//Death and Respawn
virtual void OnDead() override;
virtual void OnRespawn() override;
	
//Camera View
private:
	UPROPERTY(EditDefaultsOnly, Category = "View")
	FVector CamAimLocalOffSet;
	
	UPROPERTY(EditDefaultsOnly, Category = "View")
	float CamLerpSpeed = 20.f;
	
	FTimerHandle CamLerpTimerHandle;
	
	void LerpCamToLocalOffSet(const FVector& CamOffSet);
	void TickCamLocalOffSetLerp(FVector CamOffSet);
	
//Inventory
private:
	class UInventoryComponent* InventoryComponent;
};
