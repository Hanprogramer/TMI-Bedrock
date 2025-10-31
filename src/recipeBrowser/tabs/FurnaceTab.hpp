#pragma once
#include "TMITab.hpp"
#include "recipeBrowser/RecipeBrowserModule.hpp"

namespace TMI
{
	class FurnaceTab : public TMITab
	{
	private:
		ItemStack mIcon;
		RecipeBrowserModule* mRecipe;
	public:

		FurnaceTab(RecipeBrowserModule* recipeMod): mRecipe(recipeMod)
		{
			mIcon.reinit("minercaft:furnace", 1, 0);
		}
		~FurnaceTab()
		{
		}

		void init()
		{

		}

		std::string getTitle()
		{
			return "Smelting";
		}
		std::string getID()
		{
			return "furnace_recipe";
		}
		ItemStack getIcon() {
			return mIcon;
		}


		int getMaxItemPerPage()
		{
			return 2;
		}

		int getMaxPage()
		{
			if (mRecipe->mFurnaceRecipes.size() == 0)
				return 0;
			return (mRecipe->mFurnaceRecipes.size() / 2) - 1;
		}

		int getItemCount() {
			return mRecipe->mFurnaceRecipes.size();
		}

		ItemStack getResult(int recipeIndex)
		{
			auto& instance = mRecipe->mFurnaceRecipes[recipeIndex].second;

			ItemStack stack;
			const Item& item = *instance.getItem();
			stack.reinit(item, instance.mCount, instance.mAuxValue);
			return stack;
		}

		std::vector<ItemStack> getIngredient(int slot, int recipeIndex)
		{
			// TODO: Optimize this to not fetch all the time
			std::vector<ItemStack> result;
			result.push_back(mRecipe->mFurnaceRecipes[recipeIndex].first);
			return result;
		}
	};
}
