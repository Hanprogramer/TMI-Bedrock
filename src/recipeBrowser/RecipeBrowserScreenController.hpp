#pragma once
#include <mc/src-client/common/client/gui/screens/ScreenController.hpp>
#include <mc/src-client/common/client/gui/screens/UIScene.hpp>
#include <mc/src-client/common/client/gui/screens/controllers/ContainerScreenController.hpp>
#include <mc/src/common/world/item/Item.hpp>
#include "mc/src/common/world/containers/ContainerFactory.hpp"
#include "mc/src/common/world/actor/player/Player.hpp"
#include "mc/src/common/locale/I18n.hpp"
#include "mc/src/common/world/containers/managers/models/LevelContainerManagerModel.hpp"
#include "mc/src/common/world/containers/models/LevelContainerModel.hpp"
#include "mc/src/common/world/actor/player/Inventory.hpp"
#include <mc/src/common/world/containers/models/PlayerUIContainerModel.hpp>
#include <mc/src/common/world/containers/models/InventoryContainerModel.hpp>
#include <mc/src/common/world/SimpleContainer.hpp>
#include <mc/src/common/world/Container.hpp>
#include <mc/src-client/common/client/gui/screens/SceneFactory.hpp>
#include <mc/src-client/common/client/gui/screens/SceneStack.hpp>

class ImplementedContainer : public Container {

public:
	ImplementedContainer(ContainerType type, int size) : Container(type), mSize(size) { }
	int mSize;
	std::vector<ItemStack> mItems;

	void serverInitItemStackIds(int, int, std::function<void(int, const ItemStack&)>) override {}

	const ItemStack& getItem(int slot) const override
	{
		if (slot < 0 || slot >= mItems.size())
			return ItemStack::EMPTY_ITEM;
		return mItems[slot];
	}

	void setItem(int slot, const ItemStack& item) override
	{
		if (slot < 0 || slot >= mItems.size())
			return;
		mItems[slot] = item;
	}

	int getContainerSize() const override
	{
		return static_cast<int>(mItems.size());
	}

	int getMaxStackSize() const override
	{
		return 64;
	}

	void startOpen(Player&) override {}
};

class TMIRecipeWindowContainerModel :
	public LevelContainerModel
{
public:
	TMIRecipeWindowContainerModel(ContainerEnumName name, int size, Player& player);
	virtual Container* _getContainer() override;
	virtual int _getContainerOffset() override;
	virtual ContainerWeakRef getContainerWeakRef() override;
	Player& mPlayer;
};

class TMIRecipeWindowManagerModel :
	public LevelContainerManagerModel
{
public:
	TMIRecipeWindowManagerModel(ContainerID id, Player& player);
	virtual bool isValid();
	virtual ContainerScreenContext _postInit();

	static bool isPlayerUsingAlchemicalBag(Player& player) {
		if (player.getContainerManagerModel().expired())
			return false;
		return false;
	}
};

class AlchemicalBagManagerController :
	public ContainerManagerController
{
public:
	AlchemicalBagManagerController(std::weak_ptr<TMIRecipeWindowManagerModel> model);
	virtual void handleTakeAll(const SlotData& to, const SlotData& from);
	virtual void handlePlaceOne(const SlotData& to, const SlotData& from);
	virtual void handlePlaceAmount(const SlotData& to, int, const SlotData& from);
	virtual void handleTakeAmount(const SlotData&, int, const SlotData&);
	virtual void handleTakeHalf(const SlotData& to, const SlotData& from);
	virtual void handleSwap(const SlotData& to, const SlotData& from);
	virtual void handleSplitSingle(const SlotData& to, const SlotData& from);
	virtual void handleSplitMultiple(const SelectedSlotInfo& selectedInfo, const ItemInstance& itemInstance, const SlotData& from);
	virtual bool isAlchemicalBagItem(const SlotData& slotData);
};

namespace TMI
{
}


class RecipeBrowserScreenController : public ContainerScreenController {
public:
	const Item* mItem;
	RecipeBrowserScreenController(std::shared_ptr<ClientInstanceScreenModel> model, InteractionModel interaction, const Item* item);
	virtual void _registerCoalesceOrder() override;
	virtual void _registerAutoPlaceOrder() override;
	void _registerBindings();

	virtual void onDelete() override;
	virtual ui::ViewRequest tryExit() override;
	virtual void leaveScreen(const std::string&) override;
	virtual void onDictationEvent(const std::string&) override;
	virtual void onLeave() override;
};