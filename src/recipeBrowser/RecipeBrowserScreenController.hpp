#pragma once
#include "RecipeBrowserModule.hpp"
#include <mc/src-client/common/client/gui/screens/ScreenController.hpp>
#include <mc/src-client/common/client/gui/screens/UIScene.hpp>
#include <mc/src-client/common/client/gui/screens/controllers/ContainerScreenController.hpp>
#include <mc/src/common/world/item/Item.hpp>
#include "mc/src/common/world/containers/ContainerFactory.hpp"
#include "mc/src/common/world/actor/player/Player.hpp"
#include "mc/src/common/locale/I18n.hpp"
#include "mc/src/common/world/containers/managers/models/LevelContainerManagerModel.hpp"
#include "mc/src/common/world/containers/models/LevelContainerModel.hpp"
#include "mc/src/common/world/actor/player/Inventory.hpp"
#include <mc/src/common/world/containers/models/PlayerUIContainerModel.hpp>
#include <mc/src/common/world/containers/models/InventoryContainerModel.hpp>
#include <mc/src/common/world/SimpleContainer.hpp>
#include <mc/src/common/world/Container.hpp>
#include <mc/src-client/common/client/gui/screens/SceneFactory.hpp>
#include <mc/src-client/common/client/gui/screens/SceneStack.hpp>
#include <amethyst/runtime/ModContext.hpp>
#include <vector>
#include <mc/src-client/common/client/gui/controls/renderers/MinecraftUICustomRenderer.hpp>
#include <mc/src-client/common/client/renderer/BaseActorRenderContext.hpp>
#include <mc/src/common/world/entity/components/ActorEquipmentComponent.hpp>
#include <mc/src-client/common/client/player/LocalPlayer.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-client/common/client/renderer/actor/ItemRenderer.hpp>
#include <mc/src-client/common/client/gui/controls/UIControlFactory.hpp>
#include <mc/src-client/common/client/gui/UIResolvedDef.hpp>
#include <mc/src-client/common/client/gui/controls/CustomRenderComponent.hpp>
#include <mc/src-client/common/client/gui/controls/UIPropertyBag.hpp>


namespace TMI
{
	class RecipeBrowserScreenController : public ClientInstanceScreenController {
	public:
		const Item* mItem;
		RecipeBrowserScreenController(std::shared_ptr<ClientInstanceScreenModel> model, InteractionModel interaction, const Item* item);
		void _registerBindings();

		virtual void onDelete() override;
		virtual ui::ViewRequest tryExit() override;
		virtual void leaveScreen(const std::string&) override;
		virtual void onDictationEvent(const std::string&) override;
		virtual void onLeave() override;
	};

	void RegisterOffhandHud();
}
