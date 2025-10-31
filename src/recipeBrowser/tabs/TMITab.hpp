#pragma once
#include <mc/src/common/world/item/ItemStack.hpp>
#include "recipeBrowser/RecipeBrowserModule.hpp"

namespace TMI
{
    class TMITab
    {
    public:
        virtual std::string getTitle() = 0;
        virtual std::string getID() = 0;
        virtual ItemStack getIcon() = 0;

        virtual int getItemPerPage() = 0;

        int getMaxPage()
        {
            if (getItemCount() == 0)
                return 0;
            return ((getItemCount() + getItemPerPage() - 1) / getItemPerPage()) - 1;
        }

        virtual int getItemCount() = 0;

        virtual ItemStack getResult(int recipeIndex) = 0;
        virtual std::vector<ItemStack> getIngredient(int slot, int recipeIndex) = 0;
    };
}