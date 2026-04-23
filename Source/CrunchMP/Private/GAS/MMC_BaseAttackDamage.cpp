// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/MMC_BaseAttackDamage.h"
#include "GAS/BaseAttributeSet.h"

UMMC_BaseAttackDamage::UMMC_BaseAttackDamage()
{
	DamageCaptureDef.AttributeToCapture = UBaseAttributeSet::GetAttackDamageAttribute();
	DamageCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Source;
	
	ArmorCaptureDef.AttributeToCapture = UBaseAttributeSet::GetArmorAttribute();
	ArmorCaptureDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	
	RelevantAttributesToCapture.Add(DamageCaptureDef);
	RelevantAttributesToCapture.Add(ArmorCaptureDef);
}

float UMMC_BaseAttackDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	float AttackDamage = 0.f;
	GetCapturedAttributeMagnitude(DamageCaptureDef, Spec, EvalParams, AttackDamage);
	
	float Armor = 0.f;
	GetCapturedAttributeMagnitude(ArmorCaptureDef, Spec, EvalParams, Armor);
	
	float Damage = AttackDamage * (1 - Armor / (Armor + 100));
	
	return -Damage;
}
