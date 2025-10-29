#include "RecipeBrowserModule.hpp"

namespace TMI {
	ItemStack selectedItemStack;
	ItemStack currentHoveredStack;
	int mode = 0;
	int currentPage = 0;
	int maxPage = 0;
	std::vector<std::shared_ptr<Recipe>> recipes;

	bool initialized = false;

	std::map<std::string, ItemStack> itemMap;
	int itemCount = 0;
	int mposX = 0;
	int mposY = 0;
	boolean isMousePressed = false;
	boolean isMouseJustReleased = false;

	const char BUTTON_LEFT = 0x263b;
	const char BUTTON_RIGHT = 0x263a;

	char last_button;

	std::string getModNameFromNamespace(std::string mNamespace) {
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

	std::string getItemName(ItemStack& stack) {
		auto& item = *stack.getItem();
		return item.buildDescriptionName(stack);
	}

	void drawFakeTooltip(ItemStack& stack, MinecraftUIRenderContext& ctx) {
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

	void OnAfterRenderUI(AfterRenderUIEvent event)
	{
		const std::string name = event.screen.visualTree->mRootControlName->mName;
		auto& mc = Amethyst::GetClientCtx().mClientInstance;
		auto& ctx = event.ctx;
		if (name == "inventory_screen")
		{
			if (!initialized)
			{

				ItemRegistryRef itemRegistryRef = mc->getLocalPlayer()->getLevel()->getItemRegistry();
				std::shared_ptr<ItemRegistry> sharedRegistryPtr = itemRegistryRef._lockRegistry();
				ItemRegistry& itemRegistry = *sharedRegistryPtr;

				for (int i = 0; i < itemRegistry.mMaxItemID; i++) {
					auto itemRef = itemRegistry.mIdToItemMap[i];
					if (!itemRef.isNull()) {
						Item& item = *itemRef;
						ItemStack stack;
						stack.reinit(item, 1, 0);
						itemMap.insert(std::pair<std::string, ItemStack>(item.getFullItemName(), stack));
						itemCount++;
					}
				}

				initialized = true;

			}
			BaseActorRenderContext renderCtxPtr = BaseActorRenderContext(*ctx.mScreenContext, *ctx.mClient, *ctx.mClient->mMinecraftGame);
			int i = 0;
			int y = 0;
			int x = 0;
			const int width = 20;
			const int height = 20;
			int mposXX = mposX / mc->mGuiData->mGuiScale;
			int mposYY = mposY / mc->mGuiData->mGuiScale;
			int hovered = -1;
			std::string hoveredID = "";
			for (const auto& [key, stack] : itemMap) {
				int xx = x * width;
				int yy = y * height;
				if (mposXX > xx && mposXX < xx + width && mposYY > yy && mposYY < yy + height) {
					// Draw hover bg
					ctx.fillRectangle(RectangleArea(xx, xx + width, yy, yy + height), mce::Color(1.0f, 1.0f, 1.0f, 0.5f), 0.5f);
					hovered = i;
					hoveredID = key;
					currentHoveredStack = stack;
					if (isMouseJustReleased) {
						// Open the screen
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
						if (TMI::setRecipesForItem(item)) {
							ItemStack clone = ItemStack(stack);
							ItemStack& cloneAddr = clone;
							auto controller = std::make_shared<RecipeBrowserScreenController>(model, interactionModel, cloneAddr);

							auto scene = factory.createUIScene(game, clientInstance, "tmi.recipe_screen", controller);
							auto screen = factory._createScreen(scene);
							factory.getCurrentSceneStack()->pushScreen(screen, false);
						}
					}
				}
				renderCtxPtr.itemRenderer->renderGuiItemNew(&renderCtxPtr, &stack, 0, xx + 2, yy + 2, false, 1.0f, 1.0f, 0.95f);
				x++;
				if (x > 5) {
					x = 0;
					y += 1;
				}
				i++;
				if (i > 200) break;
			}
			ctx.flushImages(mce::Color::WHITE, 1.0f, "ui_flush");
		}
		isMouseJustReleased = false;

		// Render custom tooltip renderer
		if (!currentHoveredStack.isNull())
			drawFakeTooltip(currentHoveredStack, ctx);
		currentHoveredStack = ItemStack::EMPTY_ITEM;
	}

	void OnBeforeRenderUI(BeforeRenderUIEvent event) {
	}

	void OnMouseInput(MouseInputEvent event) {
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

	void initRecipeBrowser()
	{
		Amethyst::GetEventBus().AddListener<AfterRenderUIEvent>(&OnAfterRenderUI);
		Amethyst::GetEventBus().AddListener<BeforeRenderUIEvent>(&OnBeforeRenderUI);
		Amethyst::GetEventBus().AddListener<MouseInputEvent>(&OnMouseInput);
		RegisterCustomUIRenderers();
	}

	bool setRecipesForItem(Item& item)
	{
		ItemStack stack;
		stack.reinit(item, 1, 0);
		selectedItemStack = stack;
		recipes.clear();

		Recipes& lrecipes = Amethyst::GetClientCtx().mClientInstance->getLocalPlayer()->getLevel()->getRecipes();
		for (const auto& [recipeId, recipe] : lrecipes.mRecipes["crafting_table"]) {
			const std::vector<ItemInstance>& results = recipe->getResultItem();
			if (results.empty()) continue;
			const ItemInstance& result = results.front();
			if (result == NULL || result.isNull()) {
				Log::Info("Recipe '{}' has no result. Result NULL", recipe->mRecipeId);
				continue;
			}
			if (result.getItem()->mId == item.mId) {
				recipes.push_back(recipe);
				Log::Info("Recipe '{}' match the result type", recipe->mRecipeId);
			}
		}

		currentPage = 0;
		maxPage = std::max((int)(recipes.size() / 2.0) - 1, 0);

		return recipes.size() > 0;
	}

	bool setRecipesFromItem(Item& item)
	{
		ItemStack stack;
		stack.reinit(item, 1, 0);
		selectedItemStack = stack;
		recipes.clear();

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
						recipes.push_back(recipe);
						goto finish;
					}
				}
			}
		finish: {}
		}

		currentPage = 0;
		maxPage = std::max((int)(recipes.size() / 2.0) - 1, 0);

		return recipes.size() > 0;
	}

	ItemStack getCraftingIngredient(int slot, int recipeIndex)
	{
		auto recipe = recipes.at(recipeIndex);
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
		auto ingredient = ingredients.front();

		ItemStack stack;
		const Item& item = *ingredient.mItem;

		if (ingredient.hasAux())
			stack.reinit(item, 1, ingredient.mAuxValue);
		else
			stack.reinit(item, 1, 0);
		return stack;
	}
	ItemStack getResult(int recipeIndex)
	{
		auto& recipe = recipes.at(recipeIndex);
		auto& result = recipe->getResultItem().front();

		ItemStack stack;
		const Item& item = *result.getItem();
		stack.reinit(item, result.mCount, result.mAuxValue);
		return stack;
	}
	int recipeCount() {
		return recipes.size();
	}
}