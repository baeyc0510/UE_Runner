#include "RogueliteSubsystem.h"
#include "RogueliteActionData.h"
#include "RoguelitePoolPreset.h"
#include "RogueliteQueryFilter.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

/*~ USubsystem Interface ~*/

void URogueliteSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void URogueliteSubsystem::Deinitialize()
{
	if (RunState.bActive)
	{
		EndRun(false);
	}

	AllActions.Empty();
	TagIndex.Empty();
	PreAcquireChecks.Empty();

	Super::Deinitialize();
}

/*~ Static Access ~*/

URogueliteSubsystem* URogueliteSubsystem::Get(const UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	if (!IsValid(World))
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!IsValid(GameInstance))
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<URogueliteSubsystem>();
}

/*~ ActionDB ~*/

void URogueliteSubsystem::RegisterAction(URogueliteActionData* Action)
{
	if (!IsValid(Action))
	{
		return;
	}

	if (AllActions.Contains(Action))
	{
		return;
	}

	AllActions.Add(Action);

	// 태그 인덱스 업데이트
	for (const FGameplayTag& Tag : Action->Tags)
	{
		TagIndex.FindOrAdd(Tag).Add(Action);
	}
}

void URogueliteSubsystem::UnregisterAction(URogueliteActionData* Action)
{
	if (!IsValid(Action))
	{
		return;
	}

	if (!AllActions.Contains(Action))
	{
		return;
	}

	AllActions.Remove(Action);

	// 태그 인덱스에서 제거
	for (const FGameplayTag& Tag : Action->Tags)
	{
		if (TSet<URogueliteActionData*>* Set = TagIndex.Find(Tag))
		{
			Set->Remove(Action);
		}
	}
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetAllActions() const
{
	return AllActions.Array();
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetActionsByTag(FGameplayTag Tag) const
{
	if (const TSet<URogueliteActionData*>* Set = TagIndex.Find(Tag))
	{
		return Set->Array();
	}
	return TArray<URogueliteActionData*>();
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetActionsByTags(const FGameplayTagContainer& Tags, bool bRequireAll) const
{
	TArray<URogueliteActionData*> Result;

	if (bRequireAll)
	{
		for (URogueliteActionData* Action : AllActions)
		{
			if (IsValid(Action) && Action->HasAllTags(Tags))
			{
				Result.Add(Action);
			}
		}
	}
	else
	{
		TSet<URogueliteActionData*> ResultSet;
		for (const FGameplayTag& Tag : Tags)
		{
			if (const TSet<URogueliteActionData*>* Set = TagIndex.Find(Tag))
			{
				ResultSet.Append(*Set);
			}
		}
		Result = ResultSet.Array();
	}

	return Result;
}

/*~ Run Management ~*/

void URogueliteSubsystem::StartRun()
{
	if (RunState.bActive)
	{
		EndRun(false);
	}

	RunState.Reset();
	RunState.bActive = true;

	OnRunStarted.Broadcast();
}

void URogueliteSubsystem::EndRun(bool bCompleted)
{
	if (!RunState.bActive)
	{
		return;
	}

	RunState.bActive = false;

	OnRunEnded.Broadcast(bCompleted);
}

bool URogueliteSubsystem::IsRunActive() const
{
	return RunState.bActive;
}

FRogueliteRunState& URogueliteSubsystem::GetRunState()
{
	return RunState;
}

const FRogueliteRunState& URogueliteSubsystem::GetRunStateConst() const
{
	return RunState;
}

/*~ Query ~*/

TArray<URogueliteActionData*> URogueliteSubsystem::ExecuteQuery(const FRogueliteQuery& InQuery)
{
	TArray<URogueliteActionData*> Candidates;

	// 풀 태그 결정
	FGameplayTagContainer EffectivePoolTags = InQuery.PoolTags;
	FGameplayTagContainer EffectiveRequireTags = InQuery.RequireTags;
	FGameplayTagContainer EffectiveExcludeTags = InQuery.ExcludeTags;
	ERogueliteQueryMode EffectiveMode = InQuery.Mode;
	bool bEffectiveExcludeMaxStacked = InQuery.bExcludeMaxStacked;
	URogueliteQueryFilter* EffectiveCustomFilter = InQuery.CustomFilter;

	// 프리셋 적용
	if (IsValid(InQuery.PoolPreset))
	{
		URoguelitePoolPreset* Preset = InQuery.PoolPreset;
		EffectivePoolTags.AppendTags(Preset->PoolTags);
		EffectiveRequireTags.AppendTags(Preset->RequireTags);
		EffectiveExcludeTags.AppendTags(Preset->ExcludeTags);

		if (InQuery.Mode == ERogueliteQueryMode::All)
		{
			EffectiveMode = Preset->DefaultMode;
		}
		bEffectiveExcludeMaxStacked = bEffectiveExcludeMaxStacked || Preset->bExcludeMaxStacked;

		if (!IsValid(EffectiveCustomFilter))
		{
			EffectiveCustomFilter = Preset->AdditionalFilter;
		}
	}

	// 후보 수집
	if (EffectivePoolTags.IsEmpty())
	{
		Candidates = AllActions.Array();
	}
	else
	{
		TSet<URogueliteActionData*> CandidateSet;
		for (const FGameplayTag& Tag : EffectivePoolTags)
		{
			if (const TSet<URogueliteActionData*>* Set = TagIndex.Find(Tag))
			{
				CandidateSet.Append(*Set);
			}
		}
		Candidates = CandidateSet.Array();
	}

	// 필터링
	TArray<URogueliteActionData*> Filtered;
	for (URogueliteActionData* Action : Candidates)
	{
		if (!IsValid(Action))
		{
			continue;
		}

		// RequireTags 체크
		if (!EffectiveRequireTags.IsEmpty() && !Action->HasAllTags(EffectiveRequireTags))
		{
			continue;
		}

		// ExcludeTags 체크
		if (!EffectiveExcludeTags.IsEmpty() && Action->HasAnyTags(EffectiveExcludeTags))
		{
			continue;
		}

		// 조건 체크 (RequiredTags, BlockedByTags)
		if (!Action->MeetsConditions(RunState.ActiveTags))
		{
			continue;
		}

		// MaxStacked 체크
		if (bEffectiveExcludeMaxStacked)
		{
			int32 CurrentStacks = RunState.GetStacks(Action);
			if (Action->IsMaxStacked(CurrentStacks))
			{
				continue;
			}
		}

		// 모드 체크
		if (!PassesQueryMode(Action, EffectiveMode))
		{
			continue;
		}

		// 커스텀 필터 체크
		if (IsValid(EffectiveCustomFilter) && !EffectiveCustomFilter->PassesFilter(Action, RunState))
		{
			continue;
		}

		Filtered.Add(Action);
	}

	// 가중치 기반 선택
	TArray<URogueliteActionData*> Results = WeightedSelect(Filtered, InQuery);

	// 이벤트 발생
	OnQueryComplete.Broadcast(InQuery, Results);

	return Results;
}

TArray<URogueliteActionData*> URogueliteSubsystem::QuerySimple(URoguelitePoolPreset* Preset, int32 Count)
{
	FRogueliteQuery QueryStruct;
	QueryStruct.PoolPreset = Preset;
	QueryStruct.Count = Count;
	return ExecuteQuery(QueryStruct);
}

TArray<URogueliteActionData*> URogueliteSubsystem::QueryByTag(FGameplayTag PoolTag, int32 Count)
{
	FRogueliteQuery QueryStruct;
	QueryStruct.PoolTags.AddTag(PoolTag);
	QueryStruct.Count = Count;
	QueryStruct.Mode = ERogueliteQueryMode::NewOrAcquired;
	return ExecuteQuery(QueryStruct);
}

bool URogueliteSubsystem::PassesQueryMode(URogueliteActionData* Action, ERogueliteQueryMode Mode) const
{
	bool bAcquired = RunState.HasAction(Action);
	int32 CurrentStacks = RunState.GetStacks(Action);
	bool bMaxStacked = Action->IsMaxStacked(CurrentStacks);

	switch (Mode)
	{
	case ERogueliteQueryMode::All:
		return true;

	case ERogueliteQueryMode::OnlyNew:
		return !bAcquired;

	case ERogueliteQueryMode::OnlyAcquired:
		return bAcquired;

	case ERogueliteQueryMode::NewOrAcquired:
		return !bAcquired || (bAcquired && !bMaxStacked);

	case ERogueliteQueryMode::Custom:
		return true;
	}

	return true;
}

TArray<URogueliteActionData*> URogueliteSubsystem::WeightedSelect(const TArray<URogueliteActionData*>& Candidates, const FRogueliteQuery& InQuery)
{
	if (Candidates.Num() == 0 || InQuery.Count <= 0)
	{
		return TArray<URogueliteActionData*>();
	}

	if (Candidates.Num() <= InQuery.Count)
	{
		return Candidates;
	}

	// 랜덤 스트림 설정
	FRandomStream RandomStream;
	if (InQuery.RandomSeed != 0)
	{
		RandomStream.Initialize(InQuery.RandomSeed);
	}
	else
	{
		RandomStream.GenerateNewSeed();
	}

	// 가중치 계산
	TArray<float> Weights;
	Weights.Reserve(Candidates.Num());

	for (URogueliteActionData* Action : Candidates)
	{
		float Weight = Action->BaseWeight;

		// 가중치 배율 적용
		for (const auto& Modifier : InQuery.WeightModifiers)
		{
			if (Action->HasTag(Modifier.Key))
			{
				Weight *= Modifier.Value;
			}
		}

		Weights.Add(FMath::Max(Weight, 0.f));
	}

	// 가중치 기반 선택
	TArray<URogueliteActionData*> Results;
	TArray<int32> AvailableIndices;
	for (int32 i = 0; i < Candidates.Num(); ++i)
	{
		AvailableIndices.Add(i);
	}

	for (int32 i = 0; i < InQuery.Count && AvailableIndices.Num() > 0; ++i)
	{
		float TotalWeight = 0.f;
		for (int32 Idx : AvailableIndices)
		{
			TotalWeight += Weights[Idx];
		}

		if (TotalWeight <= 0.f)
		{
			// 모든 가중치가 0이면 균등 확률
			int32 RandomIdx = RandomStream.RandRange(0, AvailableIndices.Num() - 1);
			Results.Add(Candidates[AvailableIndices[RandomIdx]]);
			AvailableIndices.RemoveAt(RandomIdx);
		}
		else
		{
			float Random = RandomStream.FRandRange(0.f, TotalWeight);
			float Cumulative = 0.f;

			for (int32 j = 0; j < AvailableIndices.Num(); ++j)
			{
				int32 Idx = AvailableIndices[j];
				Cumulative += Weights[Idx];

				if (Random <= Cumulative)
				{
					Results.Add(Candidates[Idx]);
					AvailableIndices.RemoveAt(j);
					break;
				}
			}
		}
	}

	return Results;
}

/*~ Action Management ~*/

bool URogueliteSubsystem::AcquireAction(URogueliteActionData* Action, int32 StacksToAdd)
{
	FString FailReason;
	return TryAcquireAction(Action, FailReason, StacksToAdd);
}

bool URogueliteSubsystem::TryAcquireAction(URogueliteActionData* Action, FString& OutFailReason, int32 StacksToAdd)
{
	if (!IsValid(Action))
	{
		OutFailReason = TEXT("Invalid action");
		return false;
	}

	if (!RunState.bActive)
	{
		OutFailReason = TEXT("Run not active");
		return false;
	}

	if (StacksToAdd <= 0)
	{
		OutFailReason = TEXT("Invalid stack count");
		return false;
	}

	// 획득 전 체크
	for (const FRoguelitePreAcquireCheckSignature& Check : PreAcquireChecks)
	{
		if (Check.IsBound() && !Check.Execute(Action, RunState))
		{
			OutFailReason = TEXT("Pre-acquire check failed");
			return false;
		}
	}

	int32 OldStacks = RunState.GetStacks(Action);

	// 최대 스택 체크
	int32 NewStacks = OldStacks + StacksToAdd;
	if (Action->MaxStacks > 0)
	{
		NewStacks = FMath::Min(NewStacks, Action->MaxStacks);
	}

	if (NewStacks == OldStacks)
	{
		OutFailReason = TEXT("Already at max stacks");
		return false;
	}

	int32 ActualStacksAdded = NewStacks - OldStacks;

	// 상태 업데이트
	FRogueliteAcquiredInfo& Info = RunState.AcquiredActions.FindOrAdd(Action);
	if (OldStacks == 0)
	{
		Info.AcquiredTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	}
	Info.Stacks = NewStacks;

	// 자동 효과 적용
	ApplyAutoEffects(Action, ActualStacksAdded);

	// 이벤트 발생
	OnActionAcquired.Broadcast(Action, OldStacks, NewStacks);
	OnStackChanged.Broadcast(Action, OldStacks, NewStacks);

	return true;
}

bool URogueliteSubsystem::RemoveAction(URogueliteActionData* Action, int32 StacksToRemove, bool bRemoveAll)
{
	if (!IsValid(Action))
	{
		return false;
	}

	if (!RunState.HasAction(Action))
	{
		return false;
	}

	int32 OldStacks = RunState.GetStacks(Action);
	int32 NewStacks = bRemoveAll ? 0 : FMath::Max(0, OldStacks - StacksToRemove);
	int32 ActualStacksRemoved = OldStacks - NewStacks;

	if (ActualStacksRemoved <= 0)
	{
		return false;
	}

	// 자동 효과 제거
	RemoveAutoEffects(Action, ActualStacksRemoved);

	if (NewStacks == 0)
	{
		RunState.AcquiredActions.Remove(Action);
	}
	else
	{
		RunState.AcquiredActions[Action].Stacks = NewStacks;
	}

	// 이벤트 발생
	OnActionRemoved.Broadcast(Action, OldStacks, NewStacks);
	OnStackChanged.Broadcast(Action, OldStacks, NewStacks);

	return true;
}

bool URogueliteSubsystem::HasAction(URogueliteActionData* Action) const
{
	return RunState.HasAction(Action);
}

int32 URogueliteSubsystem::GetActionStacks(URogueliteActionData* Action) const
{
	return RunState.GetStacks(Action);
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetAllAcquired() const
{
	TArray<URogueliteActionData*> Result;
	RunState.AcquiredActions.GetKeys(Result);
	return Result;
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetAcquiredWithTag(FGameplayTag Tag) const
{
	TArray<URogueliteActionData*> Result;
	for (const auto& Pair : RunState.AcquiredActions)
	{
		if (IsValid(Pair.Key) && Pair.Key->HasTag(Tag))
		{
			Result.Add(Pair.Key);
		}
	}
	return Result;
}

/*~ Tags ~*/

void URogueliteSubsystem::AddTagToSystem(FGameplayTag Tag)
{
	RunState.ActiveTags.AddTag(Tag);
}

void URogueliteSubsystem::RemoveTagFromSystem(FGameplayTag Tag)
{
	RunState.ActiveTags.RemoveTag(Tag);
}

bool URogueliteSubsystem::HasTagInSystem(FGameplayTag Tag) const
{
	return RunState.ActiveTags.HasTag(Tag);
}

FGameplayTagContainer URogueliteSubsystem::GetAllTags() const
{
	return RunState.ActiveTags;
}

/*~ Numeric Data ~*/

void URogueliteSubsystem::SetRunStateValue(FGameplayTag Key, float Value)
{
	float OldValue = RunState.GetNumericValue(Key);
	if (!FMath::IsNearlyEqual(OldValue, Value))
	{
		RunState.SetNumericValue(Key, Value);
		OnRunStateValueChanged.Broadcast(Key, OldValue, Value);
	}
}

float URogueliteSubsystem::GetRunStateValue(FGameplayTag Key, float DefaultValue) const
{
	return RunState.GetNumericValue(Key, DefaultValue);
}

float URogueliteSubsystem::AddRunStateValue(FGameplayTag Key, float Delta)
{
	float OldValue = RunState.GetNumericValue(Key);
	float NewValue = OldValue + Delta;
	if (!FMath::IsNearlyEqual(OldValue, NewValue))
	{
		RunState.SetNumericValue(Key, NewValue);
		OnRunStateValueChanged.Broadcast(Key, OldValue, NewValue);
	}
	return NewValue;
}

TMap<FGameplayTag, float> URogueliteSubsystem::GetAllRunStateValues() const
{
	return RunState.NumericData;
}

/*~ Slots ~*/

bool URogueliteSubsystem::EquipActionToSlot(URogueliteActionData* Action, FGameplayTag SlotTag)
{
	if (!IsValid(Action) || !SlotTag.IsValid())
	{
		return false;
	}

	if (!RunState.HasAction(Action))
	{
		return false;
	}

	FRogueliteSlotArray& SlotData = RunState.Slots.FindOrAdd(SlotTag);
	if (SlotData.Actions.Contains(Action))
	{
		return false;
	}

	SlotData.Actions.Add(Action);
	return true;
}

void URogueliteSubsystem::UnequipActionFromSlot(URogueliteActionData* Action, FGameplayTag SlotTag)
{
	if (!IsValid(Action) || !SlotTag.IsValid())
	{
		return;
	}

	if (FRogueliteSlotArray* SlotData = RunState.Slots.Find(SlotTag))
	{
		SlotData->Actions.Remove(Action);
	}
}

TArray<URogueliteActionData*> URogueliteSubsystem::GetSlotContents(FGameplayTag SlotTag) const
{
	if (const FRogueliteSlotArray* SlotData = RunState.Slots.Find(SlotTag))
	{
		return SlotData->Actions;
	}
	return TArray<URogueliteActionData*>();
}

int32 URogueliteSubsystem::GetSlotCount(FGameplayTag SlotTag) const
{
	if (const FRogueliteSlotArray* SlotData = RunState.Slots.Find(SlotTag))
	{
		return SlotData->Actions.Num();
	}
	return 0;
}

bool URogueliteSubsystem::IsSlotFull(FGameplayTag SlotTag, int32 MaxCount) const
{
	return GetSlotCount(SlotTag) >= MaxCount;
}

/*~ Save/Load ~*/

FRogueliteRunSaveData URogueliteSubsystem::CreateRunSaveData() const
{
	FRogueliteRunSaveData SaveData;

	for (const auto& Pair : RunState.AcquiredActions)
	{
		if (IsValid(Pair.Key))
		{
			FSoftObjectPath Path(Pair.Key);
			SaveData.AcquiredActions.Add(Path, Pair.Value.Stacks);
		}
	}

	for (const auto& SlotPair : RunState.Slots)
	{
		FRogueliteSlotSaveArray& SaveSlot = SaveData.Slots.FindOrAdd(SlotPair.Key);
		for (URogueliteActionData* Action : SlotPair.Value.Actions)
		{
			if (IsValid(Action))
			{
				SaveSlot.ActionPaths.Add(FSoftObjectPath(Action));
			}
		}
	}

	SaveData.ActiveTags = RunState.ActiveTags;
	SaveData.NumericData = RunState.NumericData;

	return SaveData;
}

void URogueliteSubsystem::RestoreRunFromSaveData(const FRogueliteRunSaveData& SaveData)
{
	RunState.Reset();
	RunState.bActive = true;

	for (const auto& Pair : SaveData.AcquiredActions)
	{
		if (UObject* Obj = Pair.Key.TryLoad())
		{
			if (URogueliteActionData* Action = Cast<URogueliteActionData>(Obj))
			{
				FRogueliteAcquiredInfo Info;
				Info.Stacks = Pair.Value;
				RunState.AcquiredActions.Add(Action, Info);
			}
		}
	}

	for (const auto& SlotPair : SaveData.Slots)
	{
		FRogueliteSlotArray& SlotData = RunState.Slots.FindOrAdd(SlotPair.Key);
		for (const FSoftObjectPath& Path : SlotPair.Value.ActionPaths)
		{
			if (UObject* Obj = Path.TryLoad())
			{
				if (URogueliteActionData* Action = Cast<URogueliteActionData>(Obj))
				{
					SlotData.Actions.Add(Action);
				}
			}
		}
	}

	RunState.ActiveTags = SaveData.ActiveTags;
	RunState.NumericData = SaveData.NumericData;
}

/*~ Pre-Acquire Check ~*/

void URogueliteSubsystem::RegisterPreAcquireCheck(FRoguelitePreAcquireCheckSignature CheckDelegate)
{
	PreAcquireChecks.Add(CheckDelegate);
}

void URogueliteSubsystem::UnregisterPreAcquireCheck(FRoguelitePreAcquireCheckSignature CheckDelegate)
{
	PreAcquireChecks.Remove(CheckDelegate);
}

/*~ Auto Effects ~*/

void URogueliteSubsystem::ApplyAutoEffects(URogueliteActionData* Action, int32 Stacks)
{
	if (!IsValid(Action))
	{
		return;
	}

	if (Action->bAutoApplyToRunState)
	{
		for (const FRogueliteValueEntry& Entry : Action->Values)
		{
			for (int32 i = 0; i < Stacks; ++i)
			{
				float OldValue = RunState.GetNumericValue(Entry.Key);
				float NewValue = RunState.ApplyValue(Entry.Key, Entry.Value, Entry.ApplyMode);
				if (!FMath::IsNearlyEqual(OldValue, NewValue))
				{
					OnRunStateValueChanged.Broadcast(Entry.Key, OldValue, NewValue);
				}
			}
		}
	}

	if (Action->bAutoGrantTags)
	{
		RunState.ActiveTags.AppendTags(Action->Tags);
	}
}

void URogueliteSubsystem::RemoveAutoEffects(URogueliteActionData* Action, int32 Stacks)
{
	if (!IsValid(Action))
	{
		return;
	}

	if (Action->bAutoApplyToRunState)
	{
		for (const FRogueliteValueEntry& Entry : Action->Values)
		{
			for (int32 i = 0; i < Stacks; ++i)
			{
				float OldValue = RunState.GetNumericValue(Entry.Key);
				float NewValue = OldValue;

				// Add의 역연산
				if (Entry.ApplyMode == ERogueliteApplyMode::Add)
				{
					NewValue = RunState.ApplyValue(Entry.Key, -Entry.Value, ERogueliteApplyMode::Add);
				}
				// Multiply의 역연산
				else if (Entry.ApplyMode == ERogueliteApplyMode::Multiply && Entry.Value != 0.f)
				{
					NewValue = RunState.ApplyValue(Entry.Key, 1.f / Entry.Value, ERogueliteApplyMode::Multiply);
				}
				// Set, Max, Min은 역연산 불가 (상태가 보존되지 않음)

				if (!FMath::IsNearlyEqual(OldValue, NewValue))
				{
					OnRunStateValueChanged.Broadcast(Entry.Key, OldValue, NewValue);
				}
			}
		}
	}

	// 태그 제거는 다른 액션이 같은 태그를 부여했을 수 있어 처리 안 함
}
