#include "RecipeBrowserScreenController.hpp"
#include <amethyst/runtime/ModContext.hpp>
#include <vector>

TMIRecipeWindowContainerModel::TMIRecipeWindowContainerModel(ContainerEnumName name, int size, Player& player) :
	LevelContainerModel(name, size, player, BlockActorType::Undefined, { 0, 0, 0 }, ContainerCategory::Default), mPlayer(player)
{
}

Container* TMIRecipeWindowContainerModel::_getContainer()
{
	//auto* component = mPlayer.tryGetComponent<AlchemicalBagContainerComponent>();
	//Assert(component, "AlchemicalBagContainerComponent not found on player");
	//return component->mContainers[0].get();
	//return new FillingContainer(&mPlayer, 9, ContainerType::CONTAINER);
	return new ImplementedContainer(ContainerType::CONTAINER, 12);
}

int TMIRecipeWindowContainerModel::_getContainerOffset()
{
	return 104;
}

ContainerWeakRef TMIRecipeWindowContainerModel::getContainerWeakRef()
{
	auto* container = _getContainer();
	return ContainerWeakRef{
		mPlayer.getUniqueID(),
		ActorContainerType::ContainerComponent,
		BlockPos{ 0, 0, 0 },
		container->mContainerRuntimeId
	};
}

TMIRecipeWindowManagerModel::TMIRecipeWindowManagerModel(ContainerID id, Player& player) :
	LevelContainerManagerModel(id, player, player.getUniqueID())
{
	setContainerType(ContainerType::CONTAINER);
}

bool TMIRecipeWindowManagerModel::isValid() {
	return true;
}

ContainerScreenContext TMIRecipeWindowManagerModel::_postInit()
{
	auto& supplies = mPlayer.getSupplies();
	auto hotbarSize = 9;
	auto containerSize = supplies.mInventory->getContainerSize();
	_addContainer(std::make_shared<TMIRecipeWindowContainerModel>(ContainerEnumName::ContainerItems, 104, mPlayer));
	_addContainer(std::make_shared<PlayerUIContainerModel>(ContainerEnumName::CursorContainer, mPlayer));
	_addContainer(std::make_shared<InventoryContainerModel>(ContainerEnumName::HotbarContainer, hotbarSize, mPlayer));
	_addContainer(std::make_shared<InventoryContainerModel>(ContainerEnumName::InventoryContainer, containerSize - hotbarSize, mPlayer));
	_addContainer(std::make_shared<InventoryContainerModel>(ContainerEnumName::CombinedHotbarAndInventoryContainer, containerSize, mPlayer));
	for (auto& [name, container] : mContainers) {
		container->postInit();
	}
	return LevelContainerManagerModel::_postInit();
}

AlchemicalBagManagerController::AlchemicalBagManagerController(std::weak_ptr<TMIRecipeWindowManagerModel> model)
	: ContainerManagerController(model)
{
}

void AlchemicalBagManagerController::handleTakeAll(const SlotData& to, const SlotData& from)
{
	if (to.mCollectionName == "cursor_items" && isAlchemicalBagItem(from))
		return; // prevent taking out the bag onto the cursor
	ContainerManagerController::handleTakeAll(to, from);
}

void AlchemicalBagManagerController::handlePlaceOne(const SlotData& to, const SlotData& from)
{
	if (to.mCollectionName == "cursor_items" && isAlchemicalBagItem(from))
		return; // prevent taking out the bag onto the cursor
	ContainerManagerController::handlePlaceOne(to, from);
}

void AlchemicalBagManagerController::handlePlaceAmount(const SlotData& to, int amount, const SlotData& from)
{
	if (to.mCollectionName == "cursor_items" && isAlchemicalBagItem(from))
		return; // prevent taking out the bag onto the cursor
	ContainerManagerController::handlePlaceAmount(to, amount, from);
}

void AlchemicalBagManagerController::handleTakeAmount(const SlotData& to, int amount, const SlotData& from)
{
	if (to.mCollectionName == "cursor_items" && isAlchemicalBagItem(from))
		return; // prevent taking out the bag onto the cursor
	ContainerManagerController::handleTakeAmount(to, amount, from);
}

void AlchemicalBagManagerController::handleTakeHalf(const SlotData& to, const SlotData& from)
{
	if (to.mCollectionName == "cursor_items" && isAlchemicalBagItem(from))
		return; // prevent taking out the bag onto the cursor
	ContainerManagerController::handleTakeHalf(to, from);
}

void AlchemicalBagManagerController::handleSwap(const SlotData& to, const SlotData& from)
{
	if (to.mCollectionName == "cursor_items" && isAlchemicalBagItem(from))
		return; // prevent taking out the bag onto the cursor
	ContainerManagerController::handleSwap(to, from);
}

void AlchemicalBagManagerController::handleSplitSingle(const SlotData& to, const SlotData& from)
{
	if (to.mCollectionName == "cursor_items" && isAlchemicalBagItem(from))
		return; // prevent taking out the bag onto the cursor
	ContainerManagerController::handleSplitSingle(to, from);
}

void AlchemicalBagManagerController::handleSplitMultiple(const SelectedSlotInfo& selectedInfo, const ItemInstance& itemInstance, const SlotData& to)
{
	if (to.mCollectionName == "cursor_items" && isAlchemicalBagItem(selectedInfo.mSlot))
		return; // prevent taking out the bag onto the cursor
	ContainerManagerController::handleSplitMultiple(selectedInfo, itemInstance, to);
}

bool AlchemicalBagManagerController::isAlchemicalBagItem(const SlotData& slotData)
{
	auto* containerModel = mContainerManagerModel.lock()->getContainerByName(slotData.mCollectionName);
	if (!containerModel)
		return false;
	auto* fromItem = containerModel->getItemStack(slotData.mCollectionIndex).getItem();
	if (!fromItem)
		return false;
	return fromItem->mFullName.getString().starts_with("ee2:") && fromItem->mFullName.getString().ends_with("_alchemical_bag");
}

RecipeBrowserScreenController::RecipeBrowserScreenController(std::shared_ptr<ClientInstanceScreenModel> model, InteractionModel interaction, const Item* item) : ContainerScreenController(model, interaction), mItem(item)
{
	auto& player = *model->getPlayer();
	//setAssociatedEntityUniqueID(player.getUniqueID());
	auto managerModel = ContainerFactory::clientCreateContainerManagerModel<TMIRecipeWindowManagerModel>(player, ContainerID::CONTAINER_ID_NONE, player);
	mContainerManagerController = std::make_shared<AlchemicalBagManagerController>(managerModel);
	mContainerManagerController->registerContainerCallbacks();
	mContainerManagerController->postInit(mContainerManagerController);
	_registerBindings();
	_registerCoalesceOrder();
	_registerAutoPlaceOrder();
}

void RecipeBrowserScreenController::_registerCoalesceOrder()
{
}

void RecipeBrowserScreenController::_registerAutoPlaceOrder()
{
}

void RecipeBrowserScreenController::_registerBindings()
{
	std::string itemName = std::format("item.{}.name", mItem->mFullName.getString());
	itemName = getI18n().get(itemName, nullptr);

	bindString("#title_text", [itemName]() {
		return itemName;
		}, []() { return true; });
	

	auto self = this;
	this->registerButtonInteractedHandler(StringToNameId("tmi_close_modal"), [self](UIPropertyBag* props) {
		auto& clientInstance = *Amethyst::GetClientCtx().mClientInstance;
		auto& game = *clientInstance.mMinecraftGame;
		auto& factory = *clientInstance.mSceneFactory;
		factory.getCurrentSceneStack()->schedulePopScreen(1);
		return ui::ViewRequest::Exit;
	});
}

void RecipeBrowserScreenController::onDelete()
{
	Log::Info("Deleting");
}

ui::ViewRequest RecipeBrowserScreenController::tryExit()
{
	Log::Info("Try exiting");
	return ui::ViewRequest::None;
}

void RecipeBrowserScreenController::leaveScreen(const std::string& arg1)
{
	Log::Info("Leaving {}", arg1);
}

void RecipeBrowserScreenController::onDictationEvent(const std::string& arg1)
{
	Log::Info("OnDictation {}", arg1);
}


void RecipeBrowserScreenController::onLeave()
{
	// DO nothing
	Log::Info("is leaving");
}