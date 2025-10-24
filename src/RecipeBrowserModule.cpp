#include "RecipeBrowserModule.hpp"

bool once = false;

void OnAfterRenderUI(AfterRenderUIEvent event)
{
	const std::string name = event.screen.visualTree->mRootControlName->mName;
	auto& mc = Amethyst::GetClientCtx().mClientInstance;
	auto& ctx = event.ctx;
	if (name == "inventory_screen" && !once)
	{
		once = true;
		BaseActorRenderContext renderCtxPtr = BaseActorRenderContext(*ctx.mScreenContext, *ctx.mClient, *ctx.mClient->mMinecraftGame);

		ItemRegistryRef itemRegistry = mc->getLocalPlayer()->getLevel()->getItemRegistry();
		std::shared_ptr<ItemRegistry> sharedRegistryPtr = itemRegistry._lockRegistry();
		ItemRegistry& registry = *sharedRegistryPtr;

		/**
		About recipes: Recipes->mRecipe
		keys contains: 
			smithing_table
			deprecated
			cartography_table
			stonecutter
			crafting_table

			then inside that the keys are the recipe identifier such as:
				PressurePlateSpruce_recipeId
				RepairItemRecipe
				ShieldRecipe
				Stick_bamboo_recipeId
				StoneSlab4_recipeId
				StoneSlab4_stoneBrick_recipeId
				StoneSlab_Brick_recipeId
				StoneSlab_StoneBrick_recipeId
				StoneSlab_recipeId
				TrapdoorAcacia_recipeId
				TrapdoorBirch_recipeId
				TrapdoorDarkOak_recipeId
				TrapdoorJungle_recipeId
				TrapdoorSpruce_recipeId
				Trapdoor_recipeId
				WoodButton_recipeId
				WoodPressurePlate_recipeId
				acacia_stairs_acacia_recipeId
				bed_color_0
				bed_color_1
				bed_color_10
				bed_color_11
		*/

		Recipes recipes = mc->getLocalPlayer()->getLevel()->getRecipes();
		//for (const auto& [key, inner] : recipes.mRecipes["crafting_table"]) {
		//	std::cout << key.c_str() << '\n';               // if HashedString has operator<<
		//	// or
		//	// std::cout << key.toString() << '\n';
		//	// or
		//	// std::cout << key.c_str() << '\n';
		//}

		/*for (int i = 0; i < registry.mMaxItemID; i++) {
			if (registry.mIdToItemMap[i].isNull()) continue;
			Log::Info("{}: {}", i, registry.mIdToItemMap[i].get()->getFullItemName());
		}*/
		

		// Create the itemstack
		//ItemStack stack = ItemStack();
		//const Item& item = *registry.mIdToItemMap[280]; // 8 is just random ID picked
		//stack.reinit(item, 1, 0);
		//Item& item = *mc->getLocalPlayer()->playerInventory->getSelectedItem().getItem();
		//ItemStack stack = ItemStack::EMPTY_ITEM;
		//stack.reinit(item, 1, 0);

		//// Render the item
		//if(stack)
		//	renderCtxPtr.itemRenderer->renderGuiItemNew(&renderCtxPtr, &stack, 0, 0, 0, false, false, 1.0f, 1.0f);

	}
}

void RecipeBrowserModule::Init()
{
	Amethyst::GetEventBus().AddListener<AfterRenderUIEvent>(&OnAfterRenderUI);
}