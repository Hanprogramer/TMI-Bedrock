#pragma once
#include <Windows.h>
#include <amethyst/runtime/mod/Mod.hpp>
#include <amethyst/runtime/ModContext.hpp>
#include <amethyst/runtime/events/GameEvents.hpp> // OnStartJoinGameEvent
#include <string>
#include <amethyst/runtime/events/RenderingEvents.hpp>
#include <amethyst/ui/NinesliceHelper.hpp>
#include <mc/src-client/common/client/gui/ScreenView.hpp>
#include <mc/src-client/common/client/gui/gui/VisualTree.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-client/common/client/gui/gui/GuiData.hpp>
#include <mc/src-client/common/client/game/ClientInstance.hpp>
#include <mc/src-client/common/client/gui/gui/FontHandle.hpp>
#include <mc/src-deps/core/utility/NonOwnerPointer.hpp>
#include <mc/src-client/common/client/player/LocalPlayer.hpp>
#include <mc/src/common/world/level/BlockSource.hpp>
#include <mc/src/common/CommonTypes.hpp>
#include <mc/src/common/world/level/block/registry/BlockTypeRegistry.hpp>
#include <mc/src/common/locale/I18n.hpp>
#include <mc/src-deps/core/string/StringHash.hpp>
#define ModFunction extern "C" __declspec(dllexport)



BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

void OnAfterRenderUI(AfterRenderUIEvent& event);