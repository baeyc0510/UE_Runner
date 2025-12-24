#include "RogueliteActionData.h"

FPrimaryAssetId URogueliteActionData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("RogueliteAction"), GetFName());
}

float URogueliteActionData::GetValue(FGameplayTag Key, float DefaultValue) const
{
	for (const FRogueliteValueEntry& Entry : Values)
	{
		if (Entry.Key == Key)
		{
			return Entry.Value;
		}
	}
	return DefaultValue;
}

bool URogueliteActionData::HasTag(FGameplayTag Tag) const
{
	return ActionTags.HasTag(Tag);
}

bool URogueliteActionData::HasAnyTags(const FGameplayTagContainer& InTags) const
{
	return ActionTags.HasAny(InTags);
}

bool URogueliteActionData::HasAllTags(const FGameplayTagContainer& InTags) const
{
	return ActionTags.HasAll(InTags);
}

bool URogueliteActionData::IsMaxStacked(int32 CurrentStacks) const
{
	if (MaxStacks <= 0)
	{
		return false;
	}
	return CurrentStacks >= MaxStacks;
}

bool URogueliteActionData::MeetsConditions(const FGameplayTagContainer& ActiveTags) const
{
	// RequiredTags가 있으면 모두 보유해야 함
	if (!RequiredTags.IsEmpty() && !ActiveTags.HasAll(RequiredTags))
	{
		return false;
	}

	// BlockedByTags 중 하나라도 있으면 제외
	if (!BlockedByTags.IsEmpty() && ActiveTags.HasAny(BlockedByTags))
	{
		return false;
	}

	return true;
}
