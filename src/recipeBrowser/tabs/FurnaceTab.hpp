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

		FurnaceTab(RecipeBrowserModule* recipeMod);
		~FurnaceTab();

		std::string getTitle();
		std::string getID();
		ItemStack getIcon();


		int getItemPerPage();
		int getItemCount();
		ItemStack getResult(int recipeIndex);
		std::vector<ItemStack> getIngredient(int slot, int recipeIndex);
	};
}
