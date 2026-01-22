#include "AdventureInventoryComponent.h"

UAdventureInventoryComponent::UAdventureInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAdventureInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

int32 UAdventureInventoryComponent::FindIndex(FName ItemId) const
{
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].ItemId == ItemId)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

bool UAdventureInventoryComponent::HasItem(FName ItemId, int32 MinCount) const
{
	return GetCount(ItemId) >= MinCount;
}

int32 UAdventureInventoryComponent::GetCount(FName ItemId) const
{
	const int32 Idx = FindIndex(ItemId);
	return (Idx != INDEX_NONE) ? Items[Idx].Count : 0;
}

void UAdventureInventoryComponent::AddItem(const FAdventureInventoryItem& Item)
{
	if (Item.ItemId.IsNone()) return;
	if (Item.Count <= 0) return;

	const int32 Idx = FindIndex(Item.ItemId);
	if (Idx != INDEX_NONE)
	{
		Items[Idx].Count += Item.Count;
		if (!Item.DisplayName.IsEmpty()) Items[Idx].DisplayName = Item.DisplayName;
		Items[Idx].bIsKeyItem = Items[Idx].bIsKeyItem || Item.bIsKeyItem;
		Broadcast();
		return;
	}

	if (Items.Num() >= MaxDistinctItems)
	{
		// inventory full, just like your emotional bandwidth
		return;
	}

	Items.Add(Item);
	Broadcast();
}

bool UAdventureInventoryComponent::RemoveItem(FName ItemId, int32 Count)
{
	if (ItemId.IsNone()) return false;
	if (Count <= 0) return true;

	const int32 Idx = FindIndex(ItemId);
	if (Idx == INDEX_NONE) return false;

	Items[Idx].Count -= Count;
	if (Items[Idx].Count <= 0)
	{
		Items.RemoveAt(Idx);
	}
	Broadcast();
	return true;
}

void UAdventureInventoryComponent::Clear()
{
	Items.Reset();
	Broadcast();
}

void UAdventureInventoryComponent::Broadcast()
{
	OnInventoryChanged.Broadcast(Items);
}
