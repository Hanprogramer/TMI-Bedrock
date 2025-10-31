#pragma once
#include <string>
#include <amethyst/runtime/mod/Mod.hpp>
#include <amethyst/runtime/ModContext.hpp>
#include <amethyst/runtime/events/GameEvents.hpp> // OnStartJoinGameEvent
#include <amethyst/runtime/events/RenderingEvents.hpp>
#include <mc/src-client/common/client/gui/ScreenView.hpp>
#include <mc/src-client/common/client/gui/gui/VisualTree.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-client/common/client/gui/gui/GuiData.hpp>
#include <mc/src-client/common/client/renderer/BaseActorRenderContext.hpp>
#include <mc/src-client/common/client/player/LocalPlayer.hpp>
#include <mc/src/common/world/item/registry/ItemRegistryRef.hpp>
#include <mc/src/common/world/item/registry/ItemRegistry.hpp>
#include <mc/src-client/common/client/renderer/actor/ItemRenderer.hpp>
#include <mc/src/common/world/item/ItemStack.hpp>
#include <mc/src/common/world/item/crafting/Recipes.hpp>
#include <mc/src/common/world/level/block/registry/BlockTypeRegistry.hpp>
#include <amethyst/runtime/events/InputEvents.hpp>
#include <mc/src-client/common/client/gui/screens/SceneStack.hpp>
#include <mc/src-client/common/client/gui/screens/SceneFactory.hpp>
#include <amethyst/ui/NinesliceHelper.hpp>
#include <mc/src/common/locale/I18n.hpp>
#include <mc/src-client/common/client/gui/screens/SceneCreationUtils.hpp>
#include <mc/src-client/common/client/gui/screens/models/ClientInstanceScreenModel.hpp>
#include <mc/src-client/common/client/gui/screens/controllers/ContainerScreenController.hpp>
#include <mc/src-client/common/client/gui/screens/controllers/CraftingScreenController.hpp>
#include <mc/src/common/network/packet/InventoryContentPacket.hpp>
#include "RecipeBrowserScreenController.hpp"
#include <mc/src/common/world/item/ItemInstance.hpp>
#include <mc/src/common/Minecraft.hpp>

namespace TMI {

	const char BUTTON_LEFT = 0x263b;
	const char BUTTON_RIGHT = 0x263a;

	SafetyHookInline _UIControlFactory__populateCustomRenderComponent;
	SafetyHookInline _CraftingScreenController__registerBindings;
	SafetyHookInline _CraftingScreenController_handleEvent;

	class RecipeBrowserModule {
	public:
		RecipeBrowserModule();

		ItemStack mRecipeWindowSelectedItemStack;
		ItemStack mHoveredStack;
		int mRecipeWindowCurrentPage = 0;
		int mRecipeWindowMaxPage = 0;
		int mRecipeWindowCurrentTab = 0;

		int mOverlayPage = 0;
		int mOverlayMaxPage = 0;
		int mOverlayItemPerPage = 0;

		std::vector<std::shared_ptr<Recipe>> mCraftingRecipes;
		std::vector<std::pair<ItemStack, ItemInstance>> mFurnaceRecipes;
		std::string searchQuery;
		std::map<int, ItemStack> mRecipeTabIcons;

		bool initialized = false;

		std::map<std::string, ItemStack> itemMap;
		std::vector<ItemStack> queriedItems;

		int mAnimCounter = 0;
		int mAnimCounterTime = 90;
		int mAnimIndex = 0;

		int itemCount = 0;
		int mposX = 0;
		int mposY = 0;
		boolean isMousePressed = false;
		boolean isMouseJustReleased = false;

		std::shared_ptr<RecipeBrowserScreenController> controller;

		char last_button;

		void initRecipeBrowser();

		void showRecipesWindow();
		bool setRecipesForItem(Item& item);
		bool setRecipesFromItem(Item& item);
		ItemStack getCraftingIngredient(int slot, int recipeIndex);
		int recipeCount();
		int overlayItemCount();
		ItemStack& getOverlayItem(int slotIndex);

		std::string getModNameFromNamespace(std::string mNamespace);
		std::string getItemName(ItemStack& stack);
		void drawFakeTooltip(ItemStack& stack, MinecraftUIRenderContext& ctx);
		void OnAfterRenderUI(AfterRenderUIEvent event);
		void OnBeforeRenderUI(BeforeRenderUIEvent event);
		void OnMouseInput(MouseInputEvent event);
		void setSearchQuery(std::string newQuery);

		void refreshOverlayPage();
	};


	RecipeBrowserModule* recipeMod;

	void UIControlFactory__populateCustomRenderComponent(UIControlFactory* self, const UIResolvedDef& resolved, UIControl& control) {
		std::string rendererType = resolved.getAsString("renderer");

		if (rendererType == "tmi_recipe_slot_renderer") {
			control.setComponent<CustomRenderComponent>(
				std::make_unique<CustomRenderComponent>(control)
			);

			CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
			component->setRenderer(std::make_shared<RecipeSlotRenderer>(recipeMod));

			return;
		}
		else if (rendererType == "tmi_overlay_slot_renderer") {
			control.setComponent<CustomRenderComponent>(
				std::make_unique<CustomRenderComponent>(control)
			);

			CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
			component->setRenderer(std::make_shared<OverlaySlotRenderer>(recipeMod));

			return;
		}
		else if (rendererType == "tmi_overlay_grid_sizer_renderer") {
			control.setComponent<CustomRenderComponent>(
				std::make_unique<CustomRenderComponent>(control)
			);

			CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
			component->setRenderer(std::make_shared<OverlayGridSizerRenderer>(recipeMod));

			return;
		}
		else if (rendererType == "tmi_tab_icon_renderer") {
			control.setComponent<CustomRenderComponent>(
				std::make_unique<CustomRenderComponent>(control)
			);

			CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
			component->setRenderer(std::make_shared<TabIconRenderer>(recipeMod));

			return;
		}

		_UIControlFactory__populateCustomRenderComponent.call<void, UIControlFactory*, const UIResolvedDef&, UIControl&>(self, resolved, control);
	}

	void CraftingScreenController__registerBindings(CraftingScreenController* self) {
		self->bindString("#tmi_page_text", []() {
			return std::string("{}/{}", recipeMod->mOverlayPage + 1, recipeMod->mOverlayMaxPage + 1);
			},
			[]() { return true; });

		self->registerButtonInteractedHandler(StringToNameId("tmi_prev"), [&](UIPropertyBag* props) {
			recipeMod->mOverlayPage--;
			recipeMod->refreshOverlayPage();
			return ui::ViewRequest::Refresh;
			});
		self->registerButtonInteractedHandler(StringToNameId("tmi_next"), [&](UIPropertyBag* props) {
			recipeMod->mOverlayPage++;
			recipeMod->refreshOverlayPage();
			return ui::ViewRequest::Refresh;
			});
		//self->bindFloat("#tmi_visible_slot_count", [self]() {
		//	//if (recipeMod->mOverlayPage == recipeMod->mOverlayMaxPage) {
		//	//	return 3;// recipeMod->overlayItemCount() - (recipeMod->mOverlayMaxPage * recipeMod->overlayItemCount());
		//	//}
		//	//return recipeMod->mOverlayItemPerPage;
		//	return 5;
		//	},
		//	[]() { return true; }
		//);


		self->registerButtonInteractedHandler(StringToNameId("tmi_overlay_slot_pressed"), [&](UIPropertyBag* mPropertyBag) {
			if (mPropertyBag != nullptr && !mPropertyBag->mJsonValue.isNull() && mPropertyBag->mJsonValue.isObject()) {
				auto id = mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt() + (recipeMod->mOverlayPage * recipeMod->mOverlayItemPerPage);
				if (id > -1 && id < recipeMod->overlayItemCount()) {
					ItemStack stack = recipeMod->getOverlayItem(id);
					if (recipeMod->setRecipesForItem(*stack.getItem())) {
						recipeMod->showRecipesWindow();
					}
				}
			}
			return ui::ViewRequest::Refresh;
			});

		self->registerButtonInteractedHandler(StringToNameId("tmi_overlay_slot_pressed_secondary"), [&](UIPropertyBag* mPropertyBag) {
			if (mPropertyBag != nullptr && !mPropertyBag->mJsonValue.isNull() && mPropertyBag->mJsonValue.isObject()) {
				auto id = mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt() + (recipeMod->mOverlayPage * recipeMod->mOverlayItemPerPage);
				if (id > -1 && id < recipeMod->overlayItemCount()) {
					ItemStack stack = recipeMod->getOverlayItem(id);
					if (recipeMod->setRecipesFromItem(*stack.getItem())) {
						recipeMod->showRecipesWindow();
					}
				}
			}
			return ui::ViewRequest::Refresh;
			});

		_CraftingScreenController__registerBindings.call<void, CraftingScreenController*>(self);
	}

	ui::ViewRequest CraftingScreenController_handleEvent(CraftingScreenController* self, ScreenEvent& event) {
		if (event.type == ScreenEventType::TextEditChange) {
			if (event.data.textEdit.properties != nullptr) {
				if (StringToNameId("tmi_search_box") == event.data.textEdit.id) {
					auto newText = event.data.textEdit.properties->mJsonValue.get(std::string("#item_name"), Json::Value("")).asString();
					recipeMod->setSearchQuery(newText);
					return ui::ViewRequest::ConsumeEvent;
				}
			}
		}
		return _CraftingScreenController_handleEvent.call<ui::ViewRequest, CraftingScreenController*, ScreenEvent&>(self, event);
	}


	void RegisterHooks(RecipeBrowserModule* mod)
	{
		Amethyst::HookManager& hooks = Amethyst::GetHookManager();
		recipeMod = mod;
		HOOK(UIControlFactory, _populateCustomRenderComponent);
		HOOK(CraftingScreenController, _registerBindings);
		VHOOK(CraftingScreenController, handleEvent, this);
	}
}