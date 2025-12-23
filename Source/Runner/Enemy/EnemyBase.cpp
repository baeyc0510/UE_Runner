// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"

#include "AbilitySystemComponent.h"
#include "Runner/AbilitySystem/RunnerAttributeSet.h"


AEnemyBase::AEnemyBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AbilitySystemComponent =  CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	RunnerAttributeSet = CreateDefaultSubobject<URunnerAttributeSet>(TEXT("RunnerAttributeSet"));
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemyBase::ApplyDamageEffect_Implementation(const FGameplayEffectSpecHandle& InEffectHandle)
{
	UE_LOG(LogTemp,Warning,TEXT("Damage Applied"))
	AbilitySystemComponent->BP_ApplyGameplayEffectSpecToSelf(InEffectHandle);
}
