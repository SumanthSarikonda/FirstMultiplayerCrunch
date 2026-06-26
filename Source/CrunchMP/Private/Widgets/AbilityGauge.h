// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayEffectTypes.h"
#include "AbilityGauge.generated.h"

class UAbilitySystemComponent;
struct FGameplayAbilitySpec;

USTRUCT(BlueprintType)
struct FAbilityWidgetData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class UGameplayAbility> AbilityClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName AbilityName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;
	
};
/**
 * 
 */
UCLASS()
class UAbilityGauge : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	void ConfigureWithWidgetData(const FAbilityWidgetData* WidgetData);
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Cooldown")
	float CooldownUpdatedInterval = 0.1f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName IconMaterialParamName = "Icon";
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName CooldownPercentParamName = "Percent";
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName AbilityLevelParamName = "Level";
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName CanCastAbilityParamName = "CanCast";
	
	UPROPERTY(EditDefaultsOnly, Category = "Visual")
	FName UpgradePointAvailableParamName = "UpgradeAvailable";
	
	UPROPERTY(EditDefaultsOnly, Category = "ToolTip")
	TSubclassOf<class UAbilityToolTip> AbilityToolTipClass;
	
	void CreateToolTipWidget(const FAbilityWidgetData* AbilityWidgetData);
	
	UPROPERTY(meta=(BindWidget))
	class UImage* Icon;
	
	UPROPERTY(meta=(BindWidget))
	class UImage* LevelGauge;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownCountText;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CooldownDurationText;
	
	UPROPERTY(meta=(BindWidget))
	class UTextBlock* CostText;
	
	UPROPERTY()
	class UGameplayAbility* AbilityCDO;
	
	void AbilityComitted(UGameplayAbility* Ability);
	
	void StartCooldown(float CooldownTimeRemaining, float CooldownDuration);
	
	float CachedCooldownDuration;
	float CachedCooldownTimeRemaining;
	
	FTimerHandle CooldownTimerHandle;
	FTimerHandle CooldownTimerUpdateHandle;
	
	FNumberFormattingOptions WholeNumberFormattingOptions;
	FNumberFormattingOptions TwoDigitNumberFormattingOptions;
	
	void CooldownFinished();
	void UpdateCooldown();
	
	const UAbilitySystemComponent* OwnerAbilitySystemComponent;
	FGameplayAbilitySpecHandle CachedAbilitySpecHandle;
	
	const FGameplayAbilitySpec* GetAbilitySpec();
	
	bool bIsAbilityLearned = false;
	
	void AbilitySpecUpdated(const FGameplayAbilitySpec& AbilitySpec);
	void UpdateCanCast();
	void UpgradePointUpdated(const FOnAttributeChangeData& Data);
	void ManaUpdated(const FOnAttributeChangeData& Data);
	
};
