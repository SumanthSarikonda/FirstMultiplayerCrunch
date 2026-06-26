// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemInterface.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GenericTeamAgentInterface.h"
#include "Widgets/RenderActorTargetInterace.h"
#include "BaseCharacter.generated.h"

UCLASS()
class ABaseCharacter : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface, public IRenderActorTargetInterace
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();
	void ServerSideInit();
	void ClientSideInit();
	bool bIsControlledByLocalPlayer() const;
	virtual void PossessedBy(AController* NewController) override; // Only Called on the Server Side
	const TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>>& GetAbilities() const;
	virtual FVector GetCaptureLocalPos() const override;
	virtual FRotator GetCaptureLocalRot() const override;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Capture")
	FVector HeadShotCaptureLocalPos;
	
	UPROPERTY(EditDefaultsOnly, Category = "Capture")
	FRotator HeadShotCaptureLocalRot;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	/************************************/
	/*       GamePlay Ability           */
	/************************************/
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendGameplayEventToSelf(const FGameplayTag& EventTag, const FGameplayEventData& EventData);
	FORCEINLINE bool GetIsInFocusMode() const {return bIsInFocusMode;}
protected:
	
	void UpgradeAbilityWithInputId(EAbilityInputID InputID);
	
private:
	void BindGASChangeDelegates();
	void DeathTagUpdated(const FGameplayTag, int32 NewCount);
	void StunTagUpdated(const FGameplayTag, int32 NewCount);
	void AimTagUpdated(const FGameplayTag, int32 NewCount);
	void FocusTagUpdated(const FGameplayTag, int32 NewCount);
	
	bool bIsInFocusMode = false;
	
	void SetIsAiming(bool bIsAiming); 
	virtual void OnAimStateChanged(bool bIsAiming);
	void MoveSpeedUpdated(const FOnAttributeChangeData& Data);
	void MaxHealthUpdated(const FOnAttributeChangeData& Data);
	void MaxManaUpdated(const FOnAttributeChangeData& Data);
	
	UPROPERTY(VisibleAnywhere, Category = "Gameplay Ability")
	class UBaseAbilitySystemComponent* BaseAbilitySystemComponent;
	
	UPROPERTY()
	class UBaseAttributeSet* BaseAttributeSet;
	
	/************************************/
	/*               UI                 */
	/************************************/
private:
	UPROPERTY(VisibleAnywhere, Category = "UI")
	class UWidgetComponent* OverHeadWidgetComponent;
	void ConfigureOverHeadStatusWidget();
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityCheckUpdateGap = 1.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	float HeadStatGaugeVisibilityRangeSquared = 10000000.f;
	
	FTimerHandle HeadStatGaugeVisibilityTimerHandle;
	
	void UpdateHeadStatGaugeVisibility();
	void SetStatusGuageEnabled(bool bIsEnable);
	
	/************************************/
	/*              Stun                */
	/************************************/
private:
	UPROPERTY(EditDefaultsOnly, Category = "Stun")
	UAnimMontage* StunMontage;
	
	virtual void OnStun();
	virtual void OnRecoverFromStun();
	
	/************************************/
	/*       Death And Respawn          */
	/************************************/
public:
	bool IsDead() const;
	void RespawnImmediately();
	
private:
	FTransform MeshRelativeTransform;
	
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float DeathMontageTimeShift = -0.8f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	UAnimMontage* DeathMontage;
	
	FTimerHandle DeathMontageTimerHandle;
	
	void DeathMontageFinished();
	void SetRagdollEnabled(bool bIsEnable);
	
	void PlayDeathAnim();
	
	void StartDeathSequence();
	void Respawn();
	
	virtual void OnDead();
	virtual void OnRespawn();
	
	/************************************/
	/*               Team               */
	/************************************/
public:
	
	/** Assigns Team Agent to given TeamID */
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	
	/** Retrieve team identifier in form of FGenericTeamId */
	virtual FGenericTeamId GetGenericTeamId() const override;
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
	
private:
	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamId;
	
	UFUNCTION()
	virtual void OnRep_TeamID();
	
	/************************************/
	/*                AI                */
	/************************************/
private:
	
	void SetAiPerceptionStimuliSourceEnabled(bool bIsEnabled);
	
	UPROPERTY()
	class UAIPerceptionStimuliSourceComponent* PerceptionStimuliSourceComponent;
};
