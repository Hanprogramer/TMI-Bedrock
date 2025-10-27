#pragma once
#include <string>
#include <amethyst/runtime/mod/Mod.hpp>
#include <amethyst/runtime/ModContext.hpp>
#include <amethyst/runtime/events/GameEvents.hpp> // OnStartJoinGameEvent
#include <amethyst/runtime/events/RenderingEvents.hpp>
#include <mc/src-client/common/client/gui/ScreenView.hpp>
#include <mc/src-client/common/client/gui/gui/VisualTree.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-client/common/client/gui/gui/GuiData.hpp>
#include <mc/src-client/common/client/renderer/BaseActorRenderContext.hpp>
#include <mc/src-client/common/client/player/LocalPlayer.hpp>
#include <mc/src/common/world/item/registry/ItemRegistryRef.hpp>
#include <mc/src/common/world/item/registry/ItemRegistry.hpp>
#include <mc/src-client/common/client/renderer/actor/ItemRenderer.hpp>
#include <mc/src/common/world/item/ItemStack.hpp>
#include <mc/src/common/world/item/crafting/Recipes.hpp>
#include <mc/src/common/world/level/block/registry/BlockTypeRegistry.hpp>
#include <amethyst/runtime/events/InputEvents.hpp>
#include <mc/src-client/common/client/gui/screens/SceneStack.hpp>
#include <mc/src-client/common/client/gui/screens/SceneFactory.hpp>
#include <amethyst/ui/NinesliceHelper.hpp>
#include <mc/src/common/locale/I18n.hpp>
#include <mc/src-client/common/client/gui/screens/SceneCreationUtils.hpp>
#include <mc/src-client/common/client/gui/screens/models/ClientInstanceScreenModel.hpp>
#include <mc/src-client/common/client/gui/screens/controllers/ContainerScreenController.hpp>
#include <mc/src/common/network/packet/InventoryContentPacket.hpp>
#include "RecipeBrowserScreenController.hpp"
#include <mc/src/common/world/item/ItemInstance.hpp>
#include <mc/src/common/Minecraft.hpp>
namespace TMI {
    extern ItemStack selectedItemStack;
    extern ItemStack currentHoveredStack;
    extern int mode; // 0 - from, 1 - of
    extern int currentPage;
    extern int maxPage;
    extern std::vector<std::shared_ptr<Recipe>> recipes;

    void initRecipeBrowser();

    bool setRecipesForItem(Item& item);
    bool setRecipesFromItem(Item& item);
    ItemStack getCraftingIngredient(int slot, int recipeIndex);
    ItemStack getResult(int recipeIndex);
    int recipeCount();

    std::string getItemName(ItemStack& stack);
    void drawFakeTooltip(ItemStack& stack, MinecraftUIRenderContext& ctx);
    void OnAfterRenderUI(AfterRenderUIEvent event);
    void OnBeforeRenderUI(BeforeRenderUIEvent event);
}