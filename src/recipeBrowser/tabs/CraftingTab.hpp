#pragma once
#include "TMITab.hpp"
#include "recipeBrowser/RecipeBrowserModule.hpp"
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

        std::string getTitle() override;
        std::string getID() override;
        ItemStack getIcon() override;

        int getItemPerPage() override;
        int getItemCount() override;

        ItemStack getResult(int recipeIndex) override;
        std::vector<ItemStack> getIngredient(int slot, int recipeIndex) override;
    };
}