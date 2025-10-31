#include "RecipeBrowserModule.hpp"
#include "RecipeBrowserScreenController.hpp"

namespace TMI {

	SafetyHookInline _UIControlFactory__populateCustomRenderComponent;
	SafetyHookInline _CraftingScreenController__registerBindings;
	SafetyHookInline _CraftingScreenController_handleEvent;
	RecipeBrowserModule* recipeMod;


	std::string RecipeBrowserModule::getModNameFromNamespace(std::string mNamespace) {
		std::string itemNamespace = mNamespace;
		std::string modName;
		std::weak_ptr<const Amethyst::Mod> mod;
		if (itemNamespace.empty() || itemNamespace == "minecraft")
		{
			modName = "Minecraft";
		}
		else if (!(mod = Amethyst::GetContext().mModLoader->GetModByNamespace(itemNamespace)).expired()) {
			modName = mod.lock()->mInfo->FriendlyName;
		}
		else {
			modName.reserve(itemNamespace.size());
			bool cap = true;
			for (char c : itemNamespace) {
				if (c == '_') {
					modName.push_back(' ');
					cap = true;
				}
				else if (cap) {
					modName.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
					cap = false;
				}
				else {
					modName.push_back(c);
				}
			}
		}
		return modName;
	}

	std::string RecipeBrowserModule::getItemName(ItemStack& stack) {
		auto& item = *stack.getItem();
		return item.buildDescriptionName(stack);
	}

	void RecipeBrowserModule::drawFakeTooltip(ItemStack& stack, MinecraftUIRenderContext& ctx) {
		if (stack.isNull()) return;
		// Draw the background
		ClientInstance& mc = *Amethyst::GetClientCtx().mClientInstance;
		Vec2 screenSize = mc.mGuiData->clientUIScreenSize;
		Font& font = *ctx.mDebugTextFontHandle.mDefaultFont.get();
		auto fontHandlePtr = Bedrock::NonOwnerPointer<const FontHandle>(&ctx.mDebugTextFontHandle);

		Item& item = *stack.getItem();
		const int padding = 4;
		const int maxWidth = 300;
		const int maxHeight = 100;
		std::string text;
		Level& level = *mc.mMinecraft->getLevel();
		item.appendFormattedHovertext(stack, level, text, false);
		text = text.substr(0, text.size() - 3); // remove the last \n

		TextMeasureData tmd = TextMeasureData{
				1,  // fontSize
				0, //linepadding
				true, // renderShadow
				false, // showColorSymbol
				false, // hideHyphen
		};
		CaretMeasureData cmd = CaretMeasureData{
				0, false
		};

		UIMeasureStrategy& meassurer = ctx.mMeasureStrategy;

		MeasureResult result = meassurer.measureText(fontHandlePtr, text, maxWidth, maxHeight, tmd, cmd);

		float height = result.mSize.y;
		float width = std::max(result.mSize.x, 0.0f);
		float mGuiScale = mc.mGuiData->mGuiScale;
		float x = mposX / mGuiScale + 10;
		float y = mposY / mGuiScale + 10;


		// Draw the background
		Amethyst::NinesliceHelper backgroundSlice(16, 16, 5, 5);
		mce::TexturePtr texture = ctx.getTexture("textures/ui/purpleBorder", true);
		RectangleArea bgRect = RectangleArea(
			x,
			x + width + padding * 2,
			y,
			y + height + padding * 2);
		backgroundSlice.Draw(bgRect, &texture, &ctx);
		ctx.flushImages(mce::Color::WHITE, 1.0f, "ui_flush");

		// Draw the text
		ctx.drawText(font,
			RectangleArea(
				x + padding,
				x + width + padding,
				y + padding,
				height), text, mce::Color::WHITE, 1.0f, ui::TextAlignment::Center,
			tmd,
			cmd);
		ctx.flushText(1.0f);
	}

	void RecipeBrowserModule::OnAfterRenderUI(AfterRenderUIEvent event)
	{
		const std::string name = event.screen.visualTree->mRootControlName->mName;
		auto& mc = Amethyst::GetClientCtx().mClientInstance;
		auto& ctx = event.ctx;

		if (mc->mClientState != ClientInstanceState::Playing) return;
		if (!initialized)
		{
			// Regisster blocks
			auto blockRegistryRef = mc->getLocalPlayer()->getLevel()->getBlockRegistry();
			if (auto blockRegistry = blockRegistryRef.lock()) {
				for (const auto& [id, block] : blockRegistry->mBlockLookupMap) {
					auto stack = ItemStack();
					stack.reinit(*block, 1);
					if (stack.getItem() == nullptr) continue; // Skip blocks that can't be turned into item
					itemMap.insert(std::pair<std::string, ItemStack>(id.getString(), stack));
				}
			}

			// Register the items
			ItemRegistryRef itemRegistryRef = mc->getLocalPlayer()->getLevel()->getItemRegistry();
			std::shared_ptr<ItemRegistry> sharedRegistryPtr = itemRegistryRef._lockRegistry();
			ItemRegistry& itemRegistry = *sharedRegistryPtr;

			for (int i = 0; i < itemRegistry.mMaxItemID; i++) {
				auto& itemRef = itemRegistry.mIdToItemMap[i];
				if (!itemRef.isNull()) {
					Item& item = *itemRef;
					ItemStack stack;
					stack.reinit(item, 1, 0);
					itemMap.insert(std::pair<std::string, ItemStack>(item.getFullItemName(), stack));
					itemCount++;
				}
			}

			setSearchQuery("");
			initialized = true;
		}

		// Render custom tooltip renderer
		if (!mHoveredStack.isNull())
			drawFakeTooltip(mHoveredStack, ctx);
		mHoveredStack = ItemStack::EMPTY_ITEM;
	}

	void RecipeBrowserModule::OnBeforeRenderUI(BeforeRenderUIEvent event) {
		mAnimCounter++;
		if (mAnimCounter > mAnimCounterTime) {
			mAnimIndex++;
			mAnimCounter = 0;
		}
	}

	void RecipeBrowserModule::setSearchQuery(std::string newQuery)
	{
		searchQuery = newQuery;
		queriedItems.clear();

		if (newQuery.empty()) {
			for (const auto& [idName, stack] : itemMap) {
				queriedItems.push_back(stack);
			}
		}
		else {
			for (const auto& [idName, stack] : itemMap) {
				if (stack.isNull()) continue;
				if (idName.contains(newQuery)) {
					queriedItems.push_back(stack);
					continue;
				}
				auto item = stack.getItem();
				if (item != nullptr) {
					if (item->mFullName.getString().contains(newQuery)) {
						queriedItems.push_back(stack);
						continue;
					}
				}
			}
		}
		if (mOverlayItemPerPage > 0)
			mOverlayMaxPage = queriedItems.size() / mOverlayItemPerPage;
	}

	void RecipeBrowserModule::refreshOverlayPage()
	{
		if (mOverlayPage < 0)
			mOverlayPage = mOverlayMaxPage;
		if (mOverlayPage > mOverlayMaxPage)
			mOverlayPage = 0;

	}

	void RecipeBrowserModule::OnMouseInput(MouseInputEvent event) {
		mposX = event.x;
		mposY = event.y;
		boolean isPressed = !event.mButtonData == char();

		if (!isPressed && isMousePressed) {
			isMouseJustReleased = true;
		}

		last_button = event.mActionButtonId;
		isMousePressed = isPressed;

		//Log::Info("buttonData: {}, actionButtonId: {}", event.mButtonData, event.mActionButtonId);
	}

	void RecipeBrowserModule::initRecipeBrowser()
	{
		Amethyst::GetEventBus().AddListener<AfterRenderUIEvent>([&](AfterRenderUIEvent ev) { OnAfterRenderUI(ev); });
		Amethyst::GetEventBus().AddListener<BeforeRenderUIEvent>([&](BeforeRenderUIEvent ev) { OnBeforeRenderUI(ev); });
		Amethyst::GetEventBus().AddListener<MouseInputEvent>([&](MouseInputEvent ev) { OnMouseInput(ev); });
		RegisterHooks(this);
	}

	void RecipeBrowserModule::showRecipesWindow()
	{
		auto& stack = mRecipeWindowSelectedItemStack;
		auto& clientInstance = *Amethyst::GetClientCtx().mClientInstance;
		auto& game = *clientInstance.mMinecraftGame;
		auto& factory = *clientInstance.mSceneFactory;
		auto model = SceneCreationUtils::_createModel<ClientInstanceScreenModel>(
			factory,
			game,
			clientInstance,
			factory.mAdvancedGraphicOptions
		);
		auto interactionModel = ContainerScreenController::interactionModelFromUIProfile(model->getUIProfile());
		auto& item = *stack.getItem();
		auto controller = std::make_shared<RecipeBrowserScreenController>(this, model, interactionModel, mRecipeWindowSelectedItemStack);
		this->controller = controller;
		auto scene = factory.createUIScene(game, clientInstance, "tmi.recipe_screen", controller);
		auto screen = factory._createScreen(scene);
		factory.getCurrentSceneStack()->pushScreen(screen, false);
	}

	bool RecipeBrowserModule::setRecipesForItem(Item& item)
	{
		// Crafting recipes
		ItemStack stack;
		stack.reinit(item, 1, 0);
		std::vector<std::shared_ptr<Recipe>> resultRecipes;
		Recipes& lrecipes = Amethyst::GetClientCtx().mClientInstance->getLocalPlayer()->getLevel()->getRecipes();
		for (const auto& [recipeId, recipe] : lrecipes.mRecipes["crafting_table"]) {
			const std::vector<ItemInstance>& results = recipe->getResultItem();
			if (results.empty()) continue;
			const ItemInstance& result = results.front();
			if (result == NULL || result.isNull()) {
				continue;
			}
			if (result.getItem()->mId == item.mId) {
				resultRecipes.push_back(recipe);
			}
		}

		// Furnace recipes
		auto& furnaceRecipes = lrecipes.mFurnaceRecipes;
		auto& itemReg = *Amethyst::GetClientCtx().mClientInstance->getLocalPlayer()->getLevel()->getItemRegistry()._lockRegistry();
		std::vector<std::pair<ItemStack, ItemInstance>> resultFurnaceRecipes;
		int size = furnaceRecipes.size();
		for (auto& [key, result] : furnaceRecipes) {
			if (key.mTag.getString() == "furnace") {
				auto id = key.mID >> 16;
				if (result.mItem->mId != item.mId) continue;
				auto aux = static_cast<std::uint16_t>(key.mID);

				if (itemReg.mIdToItemMap.contains(id)) {
					auto& item = *itemReg.mIdToItemMap[id];
					ItemStack stack;
					stack.reinit(item, 1, aux);
					resultFurnaceRecipes.push_back(std::make_pair(stack, result));
				}
			}
		}

		if (resultRecipes.size() > 0 || resultFurnaceRecipes.size() > 0) {
			mCraftingRecipes = resultRecipes;
			mFurnaceRecipes = resultFurnaceRecipes;
			mRecipeWindowSelectedItemStack = stack;

			mRecipeWindowCurrentTab = 0;
			mRecipeWindowCurrentPage = 0;
			mRecipeWindowMaxPage = std::max((int)(mCraftingRecipes.size() / 2.0) - 1, 0);
			return true;
		}

		return false;
	}

	bool RecipeBrowserModule::setRecipesFromItem(Item& item)
	{
		ItemStack stack;
		stack.reinit(item, 1, 0);
		mRecipeWindowSelectedItemStack = stack;
		std::vector<std::shared_ptr<Recipe>> resultRecipes;

		Recipes& lrecipes = Amethyst::GetClientCtx().mClientInstance->getLocalPlayer()->getLevel()->getRecipes();
		for (const auto& [recipeId, recipe] : lrecipes.mRecipes["crafting_table"]) {
			for (int i = 0; i < 8; i++) {
				int x = 0;
				int y = 0;
				switch (i) {
				case 0:
					x = 0; y = 0;
					break;
				case 1:
					x = 1; y = 0;
					break;
				case 2:
					x = 2; y = 0;
					break;
				case 3:
					x = 0; y = 1;
					break;
				case 4:
					x = 1; y = 1;
					break;
				case 5:
					x = 2; y = 1;
					break;
				case 6:
					x = 0; y = 2;
					break;
				case 7:
					x = 1; y = 2;
					break;
				case 8:
					x = 2; y = 2;
					break;
				}
				auto& ingredients = recipe->getIngredient(x, y);
				if (ingredients.mStackSize == 0) continue;
				auto items = ingredients.mImpl->getAllItems();
				for (int j = 0; j < items.size(); j++) {
					if (items[j].mItem->mId == item.mId) {
						resultRecipes.push_back(recipe);
						goto finish;
					}
				}
			}
		finish: {}
		}

		// Furnace recipes
		auto& furnaceRecipes = lrecipes.mFurnaceRecipes;
		auto& itemReg = *Amethyst::GetClientCtx().mClientInstance->getLocalPlayer()->getLevel()->getItemRegistry()._lockRegistry();
		std::vector<std::pair<ItemStack, ItemInstance>> resultFurnaceRecipes;
		int size = furnaceRecipes.size();
		for (auto& [key, result] : furnaceRecipes) {
			if (key.mTag.getString() == "furnace") {
				auto id = key.mID >> 16;
				if (id != item.mId) continue;
				auto aux = static_cast<std::uint16_t>(key.mID);

				if (itemReg.mIdToItemMap.contains(id)) {
					auto& item = *itemReg.mIdToItemMap[id];
					ItemStack stack;
					stack.reinit(item, 1, aux);
					resultFurnaceRecipes.push_back(std::make_pair(stack, result));
				}
			}
		}

		if (resultRecipes.size() > 0 || resultFurnaceRecipes.size() > 0) {
			mCraftingRecipes = resultRecipes;
			mFurnaceRecipes = resultFurnaceRecipes;
			mRecipeWindowCurrentPage = 0;
			mRecipeWindowMaxPage = std::max((int)(mCraftingRecipes.size() / 2.0) - 1, 0);
			return true;
		}

		return false;
	}

	ItemStack RecipeBrowserModule::getCraftingIngredient(int slot, int recipeIndex)
	{
		auto& recipe = mCraftingRecipes.at(recipeIndex);
		int x = 0;
		int y = 0;
		switch (slot) {
		case 0:
			x = 0; y = 0;
			break;
		case 1:
			x = 1; y = 0;
			break;
		case 2:
			x = 2; y = 0;
			break;
		case 3:
			x = 0; y = 1;
			break;
		case 4:
			x = 1; y = 1;
			break;
		case 5:
			x = 2; y = 1;
			break;
		case 6:
			x = 0; y = 2;
			break;
		case 7:
			x = 1; y = 2;
			break;
		case 8:
			x = 2; y = 2;
			break;
		}
		auto& ingredientRef = recipe->getIngredient(x, y);
		if (ingredientRef.mStackSize == 0) return ItemStack::EMPTY_ITEM;

		auto ingredients = ingredientRef.mImpl->getAllItems(); // all possible ingredient

		auto& desc = ingredients.front();
		ItemStack stack;
		const Item& item = *desc.mItem;
		stack.reinit(item, 1, desc.mAuxValue);
		return stack;
	}

	int RecipeBrowserModule::recipeCount() {
		return mCraftingRecipes.size();
	}
	int RecipeBrowserModule::overlayItemCount() {
		return queriedItems.size();
	}
	ItemStack& RecipeBrowserModule::getOverlayItem(int index)
	{
		return queriedItems.at(index);
	}
	void UIControlFactory__populateCustomRenderComponent(UIControlFactory* self, const UIResolvedDef& resolved, UIControl& control) {
		std::string rendererType = resolved.getAsString("renderer");
		
		if (rendererType == "tmi_recipe_slot_renderer") {
		control.setComponent<CustomRenderComponent>(
		std::make_unique<CustomRenderComponent>(control)
		);

		CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
		component->setRenderer(std::make_shared<RecipeSlotRenderer>());

		return;
		}
		else if (rendererType == "tmi_overlay_slot_renderer") {
		control.setComponent<CustomRenderComponent>(
		std::make_unique<CustomRenderComponent>(control)
		);

		CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
		component->setRenderer(std::make_shared<OverlaySlotRenderer>());

		return;
		}
		else if (rendererType == "tmi_overlay_grid_sizer_renderer") {
		control.setComponent<CustomRenderComponent>(
		std::make_unique<CustomRenderComponent>(control)
		);

		CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
		component->setRenderer(std::make_shared<OverlayGridSizerRenderer>());

		return;
		}
		else if (rendererType == "tmi_tab_icon_renderer") {
		control.setComponent<CustomRenderComponent>(
		std::make_unique<CustomRenderComponent>(control)
		);

		CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
		component->setRenderer(std::make_shared<TabIconRenderer>());

		return;
		}

		_UIControlFactory__populateCustomRenderComponent.call<void, UIControlFactory*, const UIResolvedDef&, UIControl&>(self, resolved, control);
	}
	void CraftingScreenController__registerBindings(CraftingScreenController* self) {
		self->bindString("#tmi_page_text", []() {
			return std::format("{}/{}", recipeMod->mOverlayPage + 1, recipeMod->mOverlayMaxPage + 1);
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