// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/BaseAbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GAS/BaseAttributeSet.h"
#include "GAS/AbilitySystemTags.h"
#include "GAS/CHeroAttributeSet.h"
#include "GAS/PA_AbilitySystemGenerics.h"

UBaseAbilitySystemComponent::UBaseAbilitySystemComponent()
{
	GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetHealthAttribute()).AddUObject(this, &UBaseAbilitySystemComponent::HealthUpdate);
	GetGameplayAttributeValueChangeDelegate(UBaseAttributeSet::GetManaAttribute()).AddUObject(this, &UBaseAbilitySystemComponent::ManaUpdate);
	GetGameplayAttributeValueChangeDelegate(UCHeroAttributeSet::GetExperienceAttribute()).AddUObject(this, &UBaseAbilitySystemComponent::ExpUpdate);
	GenericConfirmInputID = (int32)EAbilityInputID::Confirm;
	GenericCancelInputID = (int32)EAbilityInputID::Cancel;
}

void UBaseAbilitySystemComponent::InitializeBaseAttributes()
{
	if (!AbilitySystemGenerics || ! AbilitySystemGenerics->GetBaseStatsDT() || !GetOwner())
	{
		return;
	}
		
	const UDataTable* BaseStatsDT = AbilitySystemGenerics->GetBaseStatsDT();
	const FHeroBaseStats* BaseStats = nullptr;
	
	for (const TPair<FName, uint8*>& DataPair : BaseStatsDT->GetRowMap())
	{
		BaseStats = BaseStatsDT->FindRow<FHeroBaseStats>(DataPair.Key,"");
		if (BaseStats && BaseStats->Class == GetOwner()->GetClass())
		{
			break;
		}
	}
	
	if (BaseStats)
	{
		SetNumericAttributeBase(UBaseAttributeSet::GetMaxHealthAttribute(), BaseStats->BaseMaxHealth);
		SetNumericAttributeBase(UBaseAttributeSet::GetMaxManaAttribute(), BaseStats->BaseMaxMana);
		SetNumericAttributeBase(UBaseAttributeSet::GetAttackDamageAttribute(), BaseStats->BaseAttackDamage);
		SetNumericAttributeBase(UBaseAttributeSet::GetArmorAttribute(), BaseStats->BaseArmor);
		SetNumericAttributeBase(UBaseAttributeSet::GetMoveSpeedAttribute(), BaseStats->BaseMoveSpeed);
		
		SetNumericAttributeBase(UCHeroAttributeSet::GetStrengthAttribute(), BaseStats->Strength);
		SetNumericAttributeBase(UCHeroAttributeSet::GetStrengthGrowthRateAttribute(), BaseStats->StrengthGrowthRate);
		SetNumericAttributeBase(UCHeroAttributeSet::GetIntelligenceAttribute(), BaseStats->Intelligence);
		SetNumericAttributeBase(UCHeroAttributeSet::GetIntelligenceGrowthRateAttribute(), BaseStats->IntelligenceGrowthRate);
	}
	
	const FRealCurve* ExpCurve = AbilitySystemGenerics->GetExpCurve();
	if (ExpCurve)
	{
		int MaxLevel = ExpCurve->GetNumKeys();
		SetNumericAttributeBase(UCHeroAttributeSet::GetMaxLevelAttribute(), MaxLevel);
		
		float MaxExp = ExpCurve->GetKeyValue(ExpCurve->GetLastKeyHandle());
		SetNumericAttributeBase(UCHeroAttributeSet::GetMaxLevelExpAttribute(), MaxExp);
		
	}
	
	ExpUpdate(FOnAttributeChangeData());
}

void UBaseAbilitySystemComponent::ServerSideInit()
{
	InitializeBaseAttributes();
	ApplyStartingEffects();
	GrantInitialAbilities();
}

void UBaseAbilitySystemComponent::Client_AbilitySpecLevelUpdated_Implementation(FGameplayAbilitySpecHandle Handle,
	int NewLevel)
{
	FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(Handle);
	if (Spec)
	{
		Spec->Level = NewLevel;
		AbilitySpecDirtiedCallbacks.Broadcast(*Spec);
	}
}

void UBaseAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
	for (FGameplayAbilitySpec& Spec : ActivatableAbilities.Items)
	{
		AbilitySpecDirtiedCallbacks.Broadcast(Spec);
	}
}

void UBaseAbilitySystemComponent::ApplyStartingEffects()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	if (!AbilitySystemGenerics)
		return;
	
	for (const TSubclassOf<UGameplayEffect>& EffectClass : AbilitySystemGenerics->GetStartingEffect())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(EffectClass, 1, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UBaseAbilitySystemComponent::GrantInitialAbilities()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	for (const TPair<EAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : Abilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 0 , (int32)AbilityPair.Key, nullptr));
	}
	
	for (const TPair<EAbilityInputID, TSubclassOf<UGameplayAbility>>& AbilityPair : BasicAbilities)
	{
		GiveAbility(FGameplayAbilitySpec(AbilityPair.Value, 1 , (int32)AbilityPair.Key, nullptr));
	}
	
	if (!AbilitySystemGenerics)
		return;
	
	for (const TSubclassOf<UGameplayAbility>& PassiveAbility : AbilitySystemGenerics->GetPassiveAbilities())
	{
		GiveAbility(FGameplayAbilitySpec(PassiveAbility, 1, -1, nullptr));
	}
}

void UBaseAbilitySystemComponent::ApplyAllStats()
{
	if (!AbilitySystemGenerics)
		return;
	AuthApplyGameplayEffect(AbilitySystemGenerics->GetFullStatEffect());
}

const TMap<EAbilityInputID, TSubclassOf<UGameplayAbility>>& UBaseAbilitySystemComponent::GetAbilities() const
{
	return Abilities;
}

bool UBaseAbilitySystemComponent::IsAtMaxLevel() const
{
	bool bFound;
	float CurrentLevel = GetGameplayAttributeValue(UCHeroAttributeSet::GetLevelAttribute(), bFound);
	float MaxLevel = GetGameplayAttributeValue(UCHeroAttributeSet::GetMaxLevelAttribute(), bFound);
	return CurrentLevel >= MaxLevel;
}

void UBaseAbilitySystemComponent::Server_UpgradeAbilityWithInputId_Implementation(EAbilityInputID InputID)
{
	bool bFound = false;
	float UpgradePoint = GetGameplayAttributeValue(UCHeroAttributeSet::GetUpgradePointAttribute(), bFound);
	if (!bFound || UpgradePoint <= 0)
		return;
	
	FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromInputID((int32)InputID);
	if (!AbilitySpec || AbilityTags::IsAbilityAtMaxLevel(*AbilitySpec))
		return;
	
	SetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute(), UpgradePoint - 1);
	AbilitySpec->Level += 1;
	MarkAbilitySpecDirty(*AbilitySpec);
	Client_AbilitySpecLevelUpdated(AbilitySpec->Handle, AbilitySpec->Level);
}

bool UBaseAbilitySystemComponent::Server_UpgradeAbilityWithInputId_Validate(EAbilityInputID InputID)
{
	return true;
}

void UBaseAbilitySystemComponent::AuthApplyGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffect, int32 Level)
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingSpec(GameplayEffect, Level, MakeEffectContext());
		ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
	}
}

void UBaseAbilitySystemComponent::HealthUpdate(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	bool bFound = false;
	float MaxHealth = GetGameplayAttributeValue(UBaseAttributeSet::GetMaxHealthAttribute(), bFound);
	if (bFound && ChangeData.NewValue >= MaxHealth)
	{
		if (!HasMatchingGameplayTag(AbilityTags::Status_HealthFull))
		{
			//Done Locally
			AddLooseGameplayTag(AbilityTags::Status_HealthFull);
		}
	}
	else
	{
		RemoveLooseGameplayTag(AbilityTags::Status_HealthFull);
	}
	
	if (ChangeData.NewValue <= 0 )
	{
		if (!HasMatchingGameplayTag(AbilityTags::Status_HealthEmpty))
		{
			AddLooseGameplayTag(AbilityTags::Status_HealthEmpty);
			
			if (AbilitySystemGenerics && AbilitySystemGenerics->GetDeathEffect())
				AuthApplyGameplayEffect(AbilitySystemGenerics->GetDeathEffect());
			
			FGameplayEventData DeadAbilityEventData;
			if (ChangeData.GEModData)
				DeadAbilityEventData.ContextHandle = ChangeData.GEModData->EffectSpec.GetContext();
			
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), AbilityTags::Status_Dead, DeadAbilityEventData);
			
		}
	}
	else
	{
		RemoveLooseGameplayTag(AbilityTags::Status_HealthEmpty);
	}
}

void UBaseAbilitySystemComponent::ManaUpdate(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	
	bool bFound = false;
	float MaxMana = GetGameplayAttributeValue(UBaseAttributeSet::GetMaxManaAttribute(), bFound);
	if (bFound && ChangeData.NewValue >= MaxMana)
	{
		if (!HasMatchingGameplayTag(AbilityTags::Status_ManaFull))
		{
			//Done Locally
			AddLooseGameplayTag(AbilityTags::Status_ManaFull);
		}
	}
	else
	{
		RemoveLooseGameplayTag(AbilityTags::Status_ManaFull);
	}
	
	if (ChangeData.NewValue <= 0 )
	{
		if (!HasMatchingGameplayTag(AbilityTags::Status_ManaEmpty))
		{
			AddLooseGameplayTag(AbilityTags::Status_ManaEmpty);
		}
	}
	else
	{
		RemoveLooseGameplayTag(AbilityTags::Status_ManaEmpty);
	}
}

void UBaseAbilitySystemComponent::ExpUpdate(const FOnAttributeChangeData& ChangeData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
		return;
	
	if (IsAtMaxLevel())
		return;
	
	if (!AbilitySystemGenerics)
		return;
	
	float CurrentExp = ChangeData.NewValue;
	
	const FRealCurve* ExpCurve = AbilitySystemGenerics->GetExpCurve();
	if (!ExpCurve)
	{
		UE_LOG(LogTemp, Error, TEXT("ExpCurve is NULL"));
		return;
	}
	
	float PreLevelExp = 0;
	float NextLevelExp = 0;
	float NewLevel = 1;
	
	for (auto Iter = ExpCurve->GetKeyHandleIterator(); Iter; ++Iter)
	{
		float ExpToReachLevel = ExpCurve->GetKeyValue(*Iter);
		if (CurrentExp < ExpToReachLevel)
		{
			NextLevelExp = ExpToReachLevel;
			break;
		}
		
		PreLevelExp = ExpToReachLevel;
		NewLevel = Iter.GetIndex() + 1;
	}
	
	float CurrentLevel = GetNumericAttributeBase(UCHeroAttributeSet::GetLevelAttribute());
	float CurrentUpgradePoint = GetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute());
	
	float LevelUpgraded = NewLevel - CurrentLevel;
	float NewUpgradePoint = CurrentUpgradePoint + LevelUpgraded;
	
	SetNumericAttributeBase(UCHeroAttributeSet::GetLevelAttribute(), NewLevel);
	SetNumericAttributeBase(UCHeroAttributeSet::GetPrevLevelExperienceAttribute(), PreLevelExp);
	SetNumericAttributeBase(UCHeroAttributeSet::GetNextLevelExperienceAttribute(), NextLevelExp);
	SetNumericAttributeBase(UCHeroAttributeSet::GetUpgradePointAttribute(), NewUpgradePoint);
}
