// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC_LevelBased.h"
#include "AbilitySystemComponent.h"
#include "CHeroAttributeSet.h"

UMMC_LevelBased::UMMC_LevelBased()
{
	LevelCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	LevelCaptureDef.AttributeToCapture = UCHeroAttributeSet::GetLevelAttribute();
	
	RelevantAttributesToCapture.Add(LevelCaptureDef);
}

float UMMC_LevelBased::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	UAbilitySystemComponent* ASC = Spec.GetContext().GetInstigatorAbilitySystemComponent();
	if (!ASC)
		return 0.0f;
	
	float Level = 0;
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	GetCapturedAttributeMagnitude(LevelCaptureDef, Spec, EvalParams, Level);
	
	bool bFound;
	float RateAttaributeVal = ASC->GetGameplayAttributeValue(RateAttribute, bFound);
	if (!bFound)
		return 0.0f;
	
	return (Level - 1) * RateAttaributeVal;
}
