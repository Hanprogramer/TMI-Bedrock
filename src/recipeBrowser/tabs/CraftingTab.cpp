#include "CraftingTab.hpp"

inline TMI::CraftingTab::CraftingTab(RecipeBrowserModule* recipeMod) : recipeMod(recipeMod)
{
    mIcon.reinit("minercaft:crafting_table", 1, 0);
}

inline TMI::CraftingTab::~CraftingTab()
{
}

inline void TMI::CraftingTab::init()
{

}

inline std::string TMI::CraftingTab::getTitle()
{
    return "Crafting Recipes";
}

inline std::string TMI::CraftingTab::getID()
{
    return "crafting_recipe";
}

inline ItemStack TMI::CraftingTab::getIcon() {
    return mIcon;
}

inline int TMI::CraftingTab::getMaxItemPerPage()
{
    return 2;
}

inline int TMI::CraftingTab::getMaxPage()
{
    if (recipeMod->mCraftingRecipes.size() == 0)
        return 0;
    return (recipeMod->mCraftingRecipes.size() / 2) - 1;
}

inline int TMI::CraftingTab::getItemCount() {
    return recipeMod->mCraftingRecipes.size();
}

inline ItemStack TMI::CraftingTab::getResult(int recipeIndex)
{
    auto& recipe = recipeMod->mCraftingRecipes.at(recipeIndex);
    auto& result = recipe->getResultItem().front();

    ItemStack stack;
    const Item& item = *result.getItem();
    stack.reinit(item, result.mCount, result.mAuxValue);
    return stack;
}

inline std::vector<ItemStack> TMI::CraftingTab::getIngredient(int slot, int recipeIndex)
{
    auto& recipe = recipeMod->mCraftingRecipes.at(recipeIndex);
    int x = 0;
    int y = 0;
    switch (slot)
    {
    case 0:
        x = 0;
        y = 0;
        break;
    case 1:
        x = 1;
        y = 0;
        break;
    case 2:
        x = 2;
        y = 0;
        break;
    case 3:
        x = 0;
        y = 1;
        break;
    case 4:
        x = 1;
        y = 1;
        break;
    case 5:
        x = 2;
        y = 1;
        break;
    case 6:
        x = 0;
        y = 2;
        break;
    case 7:
        x = 1;
        y = 2;
        break;
    case 8:
        x = 2;
        y = 2;
        break;
    }
    std::vector<ItemStack> results;
    auto& ingredientRef = recipe->getIngredient(x, y);
    if (ingredientRef.mStackSize == 0)
        return results;

    auto ingredients = ingredientRef.mImpl->getAllItems(); // all possible ingredient

    // TODO: multiple recipe ingredient / tags
    auto& desc = ingredients.front();
    ItemStack stack;
    const Item& item = *desc.mItem;
    stack.reinit(item, 1, desc.mAuxValue);
    results.push_back(stack);

    return results;
}
