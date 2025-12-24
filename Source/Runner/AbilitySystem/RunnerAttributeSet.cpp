// Fill out your copyright notice in the Description page of Project Settings.


#include "RunnerAttributeSet.h"

#include "GameplayEffectExtension.h"
#include "Runner/RunnerGameplayTags.h"
#include "Runner/Interfaces/CombatInterface.h"

void URunnerAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.f);
	}
}

void URunnerAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	// Meta Attribute를 통한 데미지 처리
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalDamage = GetIncomingDamage();
		SetIncomingDamage(0.f); // 리셋

		if (LocalDamage > 0.f)
		{
			const float NewHealth = GetHealth() - LocalDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
			
			bool bIsCombatInterface = GetOwningActor()->Implements<UCombatInterface>();
			if (bIsCombatInterface)
			{
				bool bIsCriticalHit = Data.EffectSpec.GetDynamicAssetTags().HasTag(TAG_Data_Critical);
				ICombatInterface::Execute_OnDamageApplied(GetOwningActor(),LocalDamage, bIsCriticalHit);
			}

			// 사망 처리
			if (GetHealth() <= 0.f)
			{
				if (bIsCombatInterface && !ICombatInterface::Execute_IsDead(GetOwningActor()))
				{
					ICombatInterface::Execute_Die(GetOwningActor());
				}
			}
		}
	}

	// MaxHealth 변경 시 현재 Health 비율 유지
	if (Data.EvaluatedData.Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
	}
}
