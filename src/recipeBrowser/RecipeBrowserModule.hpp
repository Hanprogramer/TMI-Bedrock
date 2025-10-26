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

namespace RecipeBrowserModule {
    void Init();
}