// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageExecutionCalculation.h"

#include "RunnerAttributeSet.h"
#include "Runner/RunnerGameplayTags.h"

struct FDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalMultiplier);
	DECLARE_ATTRIBUTE_CAPTUREDEF(IncomingDamage);

	FDamageStatics()
	{
		// Source (공격자) 어트리뷰트
		DEFINE_ATTRIBUTE_CAPTUREDEF(URunnerAttributeSet, AttackPower, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URunnerAttributeSet, CriticalChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(URunnerAttributeSet, CriticalMultiplier, Source, false);
        
		// Target (피격자) 어트리뷰트
		DEFINE_ATTRIBUTE_CAPTUREDEF(URunnerAttributeSet, IncomingDamage, Target, false);
	}
};

static const FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}

UDamageExecutionCalculation::UDamageExecutionCalculation(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalMultiplierDef);
	RelevantAttributesToCapture.Add(DamageStatics().IncomingDamageDef);
}

void UDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	Super::Execute_Implementation(ExecutionParams, OutExecutionOutput);
	
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
    UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

    AActor* SourceActor = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
    AActor* TargetActor = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    
    FAggregatorEvaluateParameters EvalParams;
    EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    // 어트리뷰트 값 가져오기
    float AttackPower = 0.f;
    float CritChance = 0.f;
    float CritMultiplier = 1.2f;

    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().AttackPowerDef, EvalParams, AttackPower);
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().CriticalChanceDef, EvalParams, CritChance);
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().CriticalMultiplierDef, EvalParams, CritMultiplier);

	
	float BaseDamage = AttackPower;
	
    // 크리티컬 판정
    bool bIsCritical = FMath::FRand() < CritChance;
    if (bIsCritical)
    {
        BaseDamage *= CritMultiplier;
    }
	
	// TODO: 방어력 적용
	float FinalDamage = BaseDamage; 
	
	// Spec에 정보 저장
    FGameplayEffectSpec* MutableSpec = ExecutionParams.GetOwningSpecForPreExecuteMod();
    if (MutableSpec)
    {
    	if (bIsCritical)
    	{
    		MutableSpec->AddDynamicAssetTag(TAG_Data_Critical);	
    	}
    }

    // 데미지 적용
    if (FinalDamage > 0.f)
    {
        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(
                DamageStatics().IncomingDamageProperty,
                EGameplayModOp::Additive,
                FinalDamage));
    }
}
