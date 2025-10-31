#include "FurnaceTab.hpp"

TMI::FurnaceTab::FurnaceTab(RecipeBrowserModule* recipeMod) : mRecipe(recipeMod)
{
	mIcon.reinit("minecraft:furnace", 1, 0);
}

TMI::FurnaceTab::~FurnaceTab()
{
}

inline std::string TMI::FurnaceTab::getTitle()
{
	return "Smelting";
}

inline std::string TMI::FurnaceTab::getID()
{
	return "furnace_recipe";
}

inline ItemStack TMI::FurnaceTab::getIcon() {
	return mIcon;
}

inline int TMI::FurnaceTab::getItemPerPage()
{
	return 4;
}

inline int TMI::FurnaceTab::getItemCount() {
	return mRecipe->mFurnaceRecipes.size();
}

inline ItemStack TMI::FurnaceTab::getResult(int recipeIndex)
{
	auto& instance = mRecipe->mFurnaceRecipes[recipeIndex].second;

	ItemStack stack;
	const Item& item = *instance.getItem();
	stack.reinit(item, instance.mCount, instance.mAuxValue);
	return stack;
}

inline std::vector<ItemStack> TMI::FurnaceTab::getIngredient(int slot, int recipeIndex)
{
	// TODO: Optimize this to not fetch all the time
	std::vector<ItemStack> result;
	result.push_back(mRecipe->mFurnaceRecipes[recipeIndex].first);
	return result;
}
