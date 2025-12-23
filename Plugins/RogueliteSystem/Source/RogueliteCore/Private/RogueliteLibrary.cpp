#include "RogueliteLibrary.h"
#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"

/*~ Subsystem Access ~*/

URogueliteSubsystem* URogueliteLibrary::GetSubsystem(const UObject* WorldContextObject)
{
	return URogueliteSubsystem::Get(WorldContextObject);
}

/*~ DB ~*/

void URogueliteLibrary::RegisterAction(const UObject* WorldContextObject, URogueliteActionData* Action)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->RegisterAction(Action);
	}
}

void URogueliteLibrary::UnregisterAction(const UObject* WorldContextObject, URogueliteActionData* Action)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->UnregisterAction(Action);
	}
}

TArray<URogueliteActionData*> URogueliteLibrary::GetAllRegisteredActions(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetAllActions();
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteLibrary::GetActionsByTag(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetActionsByTag(Tag);
	}
	return TArray<URogueliteActionData*>();
}

/*~ Run ~*/

void URogueliteLibrary::StartRun(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->StartRun();
	}
}

void URogueliteLibrary::EndRun(const UObject* WorldContextObject, bool bCompleted)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->EndRun(bCompleted);
	}
}

bool URogueliteLibrary::IsRunActive(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->IsRunActive();
	}
	return false;
}

FRogueliteRunState URogueliteLibrary::GetRunState(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetRunStateConst();
	}
	return FRogueliteRunState();
}

/*~ Query ~*/

TArray<URogueliteActionData*> URogueliteLibrary::QuerySimple(const UObject* WorldContextObject, URoguelitePoolPreset* Preset, int32 Count)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->QuerySimple(Preset, Count);
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteLibrary::QueryByTag(const UObject* WorldContextObject, FGameplayTag PoolTag, int32 Count)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->QueryByTag(PoolTag, Count);
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteLibrary::ExecuteQuery(const UObject* WorldContextObject, const FRogueliteQuery& QueryStruct)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->ExecuteQuery(QueryStruct);
	}
	return TArray<URogueliteActionData*>();
}

/*~ Action ~*/

bool URogueliteLibrary::AcquireAction(const UObject* WorldContextObject, URogueliteActionData* Action, int32 StacksToAdd)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->AcquireAction(Action, StacksToAdd);
	}
	return false;
}

bool URogueliteLibrary::TryAcquireAction(const UObject* WorldContextObject, URogueliteActionData* Action, FString& OutFailReason, int32 StacksToAdd)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->TryAcquireAction(Action, OutFailReason, StacksToAdd);
	}
	OutFailReason = TEXT("Subsystem not found");
	return false;
}

bool URogueliteLibrary::RemoveAction(const UObject* WorldContextObject, URogueliteActionData* Action, int32 StacksToRemove, bool bRemoveAll)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->RemoveAction(Action, StacksToRemove, bRemoveAll);
	}
	return false;
}

int32 URogueliteLibrary::GetActionStacks(const UObject* WorldContextObject, URogueliteActionData* Action)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetActionStacks(Action);
	}
	return 0;
}

bool URogueliteLibrary::HasAction(const UObject* WorldContextObject, URogueliteActionData* Action)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->HasAction(Action);
	}
	return false;
}

TArray<URogueliteActionData*> URogueliteLibrary::GetAllAcquired(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetAllAcquired();
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteLibrary::GetAcquiredWithTag(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetAcquiredWithTag(Tag);
	}
	return TArray<URogueliteActionData*>();
}

/*~ Tags ~*/

void URogueliteLibrary::AddTagToSystem(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->AddTagToSystem(Tag);
	}
}

void URogueliteLibrary::RemoveTagFromSystem(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->RemoveTagFromSystem(Tag);
	}
}

bool URogueliteLibrary::HasTagInSystem(const UObject* WorldContextObject, FGameplayTag Tag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->HasTagInSystem(Tag);
	}
	return false;
}

FGameplayTagContainer URogueliteLibrary::GetAllTags(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetAllTags();
	}
	return FGameplayTagContainer();
}

/*~ Numeric ~*/

void URogueliteLibrary::SetRunStateValue(const UObject* WorldContextObject, FGameplayTag Key, float Value)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->SetRunStateValue(Key, Value);
	}
}

float URogueliteLibrary::GetRunStateValue(const UObject* WorldContextObject, FGameplayTag Key, float DefaultValue)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetRunStateValue(Key, DefaultValue);
	}
	return DefaultValue;
}

float URogueliteLibrary::AddRunStateValue(const UObject* WorldContextObject, FGameplayTag Key, float Delta)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->AddRunStateValue(Key, Delta);
	}
	return Delta;
}

TMap<FGameplayTag, float> URogueliteLibrary::GetAllRunStateValues(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetAllRunStateValues();
	}
	return TMap<FGameplayTag, float>();
}

/*~ Slots ~*/

bool URogueliteLibrary::EquipActionToSlot(const UObject* WorldContextObject, URogueliteActionData* Action, FGameplayTag SlotTag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->EquipActionToSlot(Action, SlotTag);
	}
	return false;
}

void URogueliteLibrary::UnequipActionFromSlot(const UObject* WorldContextObject, URogueliteActionData* Action, FGameplayTag SlotTag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->UnequipActionFromSlot(Action, SlotTag);
	}
}

TArray<URogueliteActionData*> URogueliteLibrary::GetSlotContents(const UObject* WorldContextObject, FGameplayTag SlotTag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetSlotContents(SlotTag);
	}
	return TArray<URogueliteActionData*>();
}

int32 URogueliteLibrary::GetSlotCount(const UObject* WorldContextObject, FGameplayTag SlotTag)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->GetSlotCount(SlotTag);
	}
	return 0;
}

bool URogueliteLibrary::IsSlotFull(const UObject* WorldContextObject, FGameplayTag SlotTag, int32 MaxCount)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->IsSlotFull(SlotTag, MaxCount);
	}
	return false;
}

/*~ Save/Load ~*/

FRogueliteRunSaveData URogueliteLibrary::CreateRunSaveData(const UObject* WorldContextObject)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		return Subsystem->CreateRunSaveData();
	}
	return FRogueliteRunSaveData();
}

void URogueliteLibrary::RestoreRunFromSaveData(const UObject* WorldContextObject, const FRogueliteRunSaveData& SaveData)
{
	if (URogueliteSubsystem* Subsystem = GetSubsystem(WorldContextObject))
	{
		Subsystem->RestoreRunFromSaveData(SaveData);
	}
}
