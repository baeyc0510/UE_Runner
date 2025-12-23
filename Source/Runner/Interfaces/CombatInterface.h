// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

struct FGameplayEffectSpecHandle;
// This class does not need to be modified.
UINTERFACE(BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RUNNER_API ICombatInterface
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ApplyDamageEffect(const FGameplayEffectSpecHandle& InEffectHandle);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnDamageApplied(float DamageAmount, bool bIsCriticalHit);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Die();
};