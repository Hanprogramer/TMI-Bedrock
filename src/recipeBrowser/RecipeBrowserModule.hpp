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
#include <mc/src/common/world/item/ItemInstance.hpp>
#include <mc/src/common/Minecraft.hpp>
#include "customRenderers/RecipeSlotRenderer.hpp"
#include "customRenderers/OverlaySlotRenderer.hpp"
#include "customRenderers/OverlayGridSizerRenderer.hpp"
#include "customRenderers/TabIconRenderer.hpp"
#include <mc/src-client/common/client/gui/controls/UIControlFactory.hpp>
#include <mc/src-client/common/client/gui/UIResolvedDef.hpp>
#include <mc/src-client/common/client/gui/controls/UIPropertyBag.hpp>
#include <mc/src-client/common/client/gui/controls/CustomRenderComponent.hpp>
#include <mc/src-client/common/client/gui/screens/ScreenEvent.hpp>

namespace TMI {

	const char BUTTON_LEFT = 0x263b;
	const char BUTTON_RIGHT = 0x263a;

	class RecipeBrowserScreenController;
	class RecipeBrowserModule {
	public:
		static RecipeBrowserModule& getInstance() {
			static RecipeBrowserModule instance;
			return instance;
		}

		ItemStack mRecipeWindowSelectedItemStack;
		ItemStack mHoveredStack;
		std::string mHoveredText;

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

		char last_button = 'a';

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

		std::string getItemTooltipText(ItemStack& stack);
		void drawFakeTooltip(ItemStack& stack, MinecraftUIRenderContext& ctx);
		void drawFakeTextTooltip(std::string text, MinecraftUIRenderContext& ctx);
		void OnAfterRenderUI(AfterRenderUIEvent event);
		void OnBeforeRenderUI(BeforeRenderUIEvent event);
		void OnMouseInput(MouseInputEvent event);
		void setSearchQuery(std::string newQuery);

		void refreshOverlayPage();
		void cleanup();

		bool isCheatEnabled();
		bool isAddItemKeyHeld();

	private:
		// Private constructor to prevent direct instantiation
		RecipeBrowserModule() { }

		// Delete copy constructor and assignment operator
		RecipeBrowserModule(const RecipeBrowserModule&) = delete;
		RecipeBrowserModule& operator=(const RecipeBrowserModule&) = delete;

		// Optionally delete move constructor and assignment operator
		RecipeBrowserModule(RecipeBrowserModule&&) = delete;
		RecipeBrowserModule& operator=(RecipeBrowserModule&&) = delete;
	};

	void UIControlFactory__populateCustomRenderComponent(UIControlFactory* self, const UIResolvedDef& resolved, UIControl& control);

	void CraftingScreenController__registerBindings(CraftingScreenController* self);

	ui::ViewRequest CraftingScreenController_handleEvent(CraftingScreenController* self, ScreenEvent& event);


	void RegisterHooks(RecipeBrowserModule* mod);
}