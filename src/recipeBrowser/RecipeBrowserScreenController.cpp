#include "RecipeBrowserScreenController.hpp"
#include <mc/src-client/common/client/gui/screens/controllers/CraftingScreenController.hpp>
#include "customRenderers/RecipeSlotRenderer.hpp"
#include "customRenderers/OverlaySlotRenderer.hpp"
#include "customRenderers/OverlayGridSizerRenderer.hpp"

namespace TMI {
	
	SafetyHookInline _UIControlFactory__populateCustomRenderComponent;
	SafetyHookInline _CraftingScreenController__registerBindings;
	SafetyHookInline _CraftingScreenController_handleEvent;

	void UIControlFactory__populateCustomRenderComponent(UIControlFactory* self, const UIResolvedDef& resolved, UIControl& control) {
		std::string rendererType = resolved.getAsString("renderer");

		if (rendererType == "tmi_recipe_slot_renderer") {
			control.setComponent<CustomRenderComponent>(
				std::make_unique<CustomRenderComponent>(control)
			);

			CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
			component->setRenderer(std::make_shared<RecipeSlotRenderer>());

			return;
		}
		else if (rendererType == "tmi_overlay_slot_renderer") {
			control.setComponent<CustomRenderComponent>(
				std::make_unique<CustomRenderComponent>(control)
			);

			CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
			component->setRenderer(std::make_shared<OverlaySlotRenderer>());

			return;
		}
		else if (rendererType == "tmi_overlay_grid_sizer_renderer") {
			control.setComponent<CustomRenderComponent>(
				std::make_unique<CustomRenderComponent>(control)
			);

			CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
			component->setRenderer(std::make_shared<OverlayGridSizerRenderer>());

			return;
		}

		_UIControlFactory__populateCustomRenderComponent.call<void, UIControlFactory*, const UIResolvedDef&, UIControl&>(self, resolved, control);
	}

	void CraftingScreenController__registerBindings(CraftingScreenController* self) {
		self->bindString("#tmi_page_text", []() { return std::format("{}/{}", TMI::mOverlayPage + 1, TMI::mOverlayMaxPage + 1); }, []() { return true; });

		self->registerButtonInteractedHandler(StringToNameId("tmi_prev"), [self](UIPropertyBag* props) {
			TMI::mOverlayPage--;
			refreshPage();
			return ui::ViewRequest::Refresh;
			});
		self->registerButtonInteractedHandler(StringToNameId("tmi_next"), [self](UIPropertyBag* props) {
			TMI::mOverlayPage++;
			refreshPage();
			return ui::ViewRequest::Refresh;
			});
		self->bindFloat("#tmi_visible_slot_count", [self]() {
			if (TMI::mOverlayPage == TMI::mOverlayMaxPage) {
				return 3;// TMI::overlayItemCount() - (TMI::mOverlayMaxPage * TMI::overlayItemCount());
			}
			return TMI::mOverlayItemPerPage;
			},
			[]() { return false; }
		);


		self->registerButtonInteractedHandler(StringToNameId("tmi_overlay_slot_pressed"), [self](UIPropertyBag* mPropertyBag) {
			if (mPropertyBag != nullptr && !mPropertyBag->mJsonValue.isNull() && mPropertyBag->mJsonValue.isObject()) {
				auto id = mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt() + (TMI::mOverlayPage * TMI::mOverlayItemPerPage);
				if (id > -1 && id < TMI::overlayItemCount()) {
					ItemStack stack = TMI::getOverlayItem(id);
					if (TMI::setRecipesForItem(*stack.getItem())) {
						TMI::showRecipesWindow();
					}
				}
			}
			return ui::ViewRequest::Refresh;
			});
		self->registerButtonInteractedHandler(StringToNameId("tmi_overlay_slot_pressed_secondary"), [self](UIPropertyBag* mPropertyBag) {
			if (mPropertyBag != nullptr && !mPropertyBag->mJsonValue.isNull() && mPropertyBag->mJsonValue.isObject()) {
				auto id = mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt() + (TMI::mOverlayPage * TMI::mOverlayItemPerPage);
				if (id > -1 && id < TMI::overlayItemCount()) {
					ItemStack stack = TMI::getOverlayItem(id);
					if (TMI::setRecipesFromItem(*stack.getItem())) {
						TMI::showRecipesWindow();
					}
				}
			}
			return ui::ViewRequest::Refresh;
			});

		_CraftingScreenController__registerBindings.call<void, CraftingScreenController*>(self);
	}

	ui::ViewRequest CraftingScreenController_handleEvent(CraftingScreenController* self, ScreenEvent& event) {
		if (event.type == ScreenEventType::TextEditChange) {
			if (event.data.textEdit.properties != nullptr) {
				if (StringToNameId("tmi_search_box") == event.data.textEdit.id) {
					auto newText = event.data.textEdit.properties->mJsonValue.get(std::string("#item_name"), Json::Value("")).asString();
					TMI::setSearchQuery(newText);
					return ui::ViewRequest::ConsumeEvent;
				}
			}
		}
		return _CraftingScreenController_handleEvent.call<ui::ViewRequest, CraftingScreenController*, ScreenEvent&>(self, event);
	}


	void RegisterHooks()
	{
		Amethyst::HookManager& hooks = Amethyst::GetHookManager();
		HOOK(UIControlFactory, _populateCustomRenderComponent);
		HOOK(CraftingScreenController, _registerBindings);
		VHOOK(CraftingScreenController, handleEvent, this);
	}

	RecipeBrowserScreenController::RecipeBrowserScreenController(std::shared_ptr<ClientInstanceScreenModel> model, InteractionModel interaction, ItemStack& itemStack) : ClientInstanceScreenController(model), mItemStack(itemStack)
	{
		auto& player = *model->getPlayer();
		_registerBindings();
	}

	void RecipeBrowserScreenController::_registerBindings()
	{

		bindString("#title_text", [this]() {
			if (mItemStack.isNull()) return std::string("Crafting Recipes");
			Item& item = *mItemStack.getItem();
			return item.buildDescriptionName(mItemStack);
			}, []() { return true; });

		bindString("#page_text", [this]() {
			return std::format("{}/{}", TMI::mRecipeWindowCurrentPage + 1, TMI::mRecipeWindowMaxPage + 1);
			}, []() { return true; });

		bindBool("#recipe2visible", [this]() {
			if (TMI::mRecipeWindowCurrentPage == TMI::mRecipeWindowMaxPage && TMI::recipeCount() % 2 != 0) {
				return false;
			}
			return true;
			}, []() {return true; });

		bindBool("#prevBtnVisible", [this]() {
			if (TMI::mRecipeWindowCurrentPage == 0 || TMI::mRecipeWindowMaxPage == 0) {
				return false;
			}
			return true;
			}, []() {return true; });

		bindBool("#nextBtnVisible", [this]() {
			if (TMI::mRecipeWindowCurrentPage == TMI::mRecipeWindowMaxPage || TMI::mRecipeWindowMaxPage == 0) {
				return false;
			}
			return true;
			}, []() {return true; });

		this->registerButtonInteractedHandler(StringToNameId("tmi_close_modal"), [this](UIPropertyBag* props) {
			auto& clientInstance = *Amethyst::GetClientCtx().mClientInstance;
			auto& game = *clientInstance.mMinecraftGame;
			auto& factory = *clientInstance.mSceneFactory;

			factory.getCurrentSceneStack()->schedulePopScreen(1);
			return ui::ViewRequest::Exit;
			});

		this->registerButtonInteractedHandler(StringToNameId("tmi_slot_pressed"), [this](UIPropertyBag* mPropertyBag) {
			if (mPropertyBag != nullptr && !mPropertyBag->mJsonValue.isNull() && mPropertyBag->mJsonValue.isObject()) {
				auto id = mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt();
				auto recipe_index = mPropertyBag->mJsonValue.get("#tmi_recipe_index", Json::Value(-1)).asInt() + (TMI::mRecipeWindowCurrentPage * 2) - 1;
				auto isResultSlot = mPropertyBag->mJsonValue.get("#tmi_is_result_slot", Json::Value(false)).asBool();

				if (id > -1 && recipe_index > -1 && recipe_index < TMI::recipeCount()) {
					ItemStack stack;
					if (isResultSlot) {
						stack = TMI::getResult(recipe_index);
					}
					else {
						stack = TMI::getCraftingIngredient(id, recipe_index);
					}

					TMI::setRecipesForItem(*stack.getItem());
				}
			}
			return ui::ViewRequest::Refresh;
			});

		this->registerButtonInteractedHandler(StringToNameId("tmi_slot_pressed_secondary"), [this](UIPropertyBag* mPropertyBag) {
			if (mPropertyBag != nullptr && !mPropertyBag->mJsonValue.isNull() && mPropertyBag->mJsonValue.isObject()) {
				auto id = mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt();
				auto recipe_index = mPropertyBag->mJsonValue.get("#tmi_recipe_index", Json::Value(-1)).asInt() + (TMI::mRecipeWindowCurrentPage * 2) - 1;
				auto isResultSlot = mPropertyBag->mJsonValue.get("#tmi_is_result_slot", Json::Value(false)).asBool();

				if (id > -1 && recipe_index > -1 && recipe_index < TMI::recipeCount()) {
					ItemStack stack;
					if (isResultSlot) {
						stack = TMI::getResult(recipe_index);
					}
					else {
						stack = TMI::getCraftingIngredient(id, recipe_index);
					}

					TMI::setRecipesFromItem(*stack.getItem());
				}
			}
			return ui::ViewRequest::Refresh;
			});

		this->registerButtonInteractedHandler(StringToNameId("tmi_prev"), [this](UIPropertyBag* props) {
			TMI::mRecipeWindowCurrentPage--;
			refreshPage();
			return ui::ViewRequest::Refresh;
			});
		this->registerButtonInteractedHandler(StringToNameId("tmi_next"), [this](UIPropertyBag* props) {
			TMI::mRecipeWindowCurrentPage++;
			refreshPage();
			return ui::ViewRequest::Refresh;
			});
	}
}