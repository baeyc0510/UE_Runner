// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Runner/Interfaces/CombatInterface.h"
#include "EnemyBase.generated.h"

class URunnerAttributeSet;

UCLASS()
class RUNNER_API AEnemyBase : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void ApplyDamageEffect_Implementation(const FGameplayEffectSpecHandle& InEffectHandle) override;
	virtual void OnDamageApplied_Implementation(float DamageAmount, bool bIsCriticalHit) override {}
	virtual void Die_Implementation() override {}

protected:
	UPROPERTY(BlueprintReadOnly)
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly)
	URunnerAttributeSet* RunnerAttributeSet;
};
