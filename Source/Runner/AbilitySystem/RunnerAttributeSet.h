// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#include "RunnerAttributeSet.generated.h"


#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS(BlueprintType, Blueprintable)
class RUNNER_API URunnerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

public:
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, Health)
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet,MaxHealth)
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData AttackSpeed;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, AttackSpeed)

	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData MoveSpeedMultiplier;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, MoveSpeedMultiplier)
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData CriticalChance;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, CriticalChance)
	
	UPROPERTY(BlueprintReadOnly)
	FGameplayAttributeData CriticalMultiplier;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, CriticalMultiplier)
	
	UPROPERTY(BlueprintReadOnly, Category = "Meta")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(URunnerAttributeSet, IncomingDamage)
};