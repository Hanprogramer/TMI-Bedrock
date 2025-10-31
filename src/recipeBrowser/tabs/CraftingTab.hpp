#pragma once
#include "TMITab.hpp"
namespace TMI
{
    class CraftingTab : public TMITab
    {
    private:
        ItemStack mIcon;
        RecipeBrowserModule* recipeMod;
    public:
        CraftingTab(RecipeBrowserModule* recipeMod);
        ~CraftingTab();

        void init();

        std::string getTitle();
        std::string getID();

        ItemStack getIcon();

        int getMaxItemPerPage();

        int getMaxPage();

        int getItemCount();

        ItemStack getResult(int recipeIndex);

        std::vector<ItemStack> getIngredient(int slot, int recipeIndex);
    };
}