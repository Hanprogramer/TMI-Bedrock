#include "RecipeBrowserScreenController.hpp"
#include <mc/src-client/common/client/gui/screens/controllers/CraftingScreenController.hpp>

namespace TMI {
	static void refreshPage() {
		if (TMI::mRecipeWindowCurrentPage < 0)
			TMI::mRecipeWindowCurrentPage = TMI::mRecipeWindowMaxPage;
		if (TMI::mRecipeWindowCurrentPage > TMI::mRecipeWindowMaxPage)
			TMI::mRecipeWindowCurrentPage = 0;

		if (TMI::mOverlayPage < 0)
			TMI::mOverlayPage = TMI::mOverlayMaxPage;
		if (TMI::mOverlayPage > TMI::mOverlayMaxPage)
			TMI::mOverlayPage = 0;
	}
	class RecipeSlotRenderer : public MinecraftUICustomRenderer {
	public:
		RecipeSlotRenderer() : MinecraftUICustomRenderer() {};

		virtual std::shared_ptr<UICustomRenderer> clone() const override {
			return std::make_shared<RecipeSlotRenderer>();
		}

		virtual void render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB) override {
			if (owner.mPropertyBag != nullptr && !owner.mPropertyBag->mJsonValue.isNull() && owner.mPropertyBag->mJsonValue.isObject()) {
				auto id = owner.mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt();
				auto recipe_index = owner.mPropertyBag->mJsonValue.get("#tmi_recipe_index", Json::Value(-1)).asInt() + (TMI::mRecipeWindowCurrentPage * 2) - 1;
				auto isResultSlot = owner.mPropertyBag->mJsonValue.get("#tmi_is_result_slot", Json::Value(false)).asBool();
				if (id > -1 && recipe_index > -1 && recipe_index < TMI::recipeCount()) {
					ItemStack stack;
					if (isResultSlot) {
						stack = TMI::getResult(recipe_index);
					}
					else {
						stack = TMI::getCraftingIngredient(id, recipe_index);
					}

					if (stack.isNull() || stack == ItemStack::EMPTY_ITEM) return;

					ClientInstance& client = _client.asInstance();

					// This disables the item visual from bobbing whenever the itemStack stack changes in content
					stack.mShowPickup = false;

					glm::tvec2<float> pos = owner.getPosition();

					BaseActorRenderContext renderCtx(*ctx.mScreenContext, client, *client.mMinecraftGame);
					int yOffset = stack.getItem()->getIconYOffset();

					// TODO: trying to fix fence block not rendering
					/*if (stack.getItem()->getLegacyBlock() != nullptr) {
						const BlockLegacy& renderBlock = *stack.getLegacyBlock()->getRenderBlock().mLegacyBlock;
						stack.reinit(renderBlock, 1);
						stack.mShowPickup = false;
					}*/

					renderCtx.itemRenderer->renderGuiItemNew(&renderCtx, &stack, 0, pos.x + 1.0f, pos.y - yOffset + 1.0f, false, 1.0f, mPropagatedAlpha, 1.0f);

					mce::Color color(1.0f, 1.0f, 1.0f, mPropagatedAlpha);
					ctx.flushImages(color, 1.0f, "ui_flush");

					if (stack.mCount > 1) {
						std::string text = std::format("{}", stack.mCount);
						float lineLength = ctx.getLineLength(*client.mMinecraftGame->mFontHandle.mDefaultFont, text, 1.0f, false);

						renderAABB._x0 = pos.x + (18.0f - lineLength);
						renderAABB._x1 = pos.x + 8.0f;
						renderAABB._y0 = pos.y + 10.0f;
						renderAABB._y1 = pos.y + 10.0f;

						TextMeasureData textData;
						memset(&textData, 0, sizeof(TextMeasureData));
						textData.fontSize = 1.0f;
						textData.renderShadow = true;

						CaretMeasureData caretData;
						memset(&caretData, 1, sizeof(CaretMeasureData));

						ctx.drawDebugText(renderAABB, text, mce::Color::WHITE, 1.0f, ui::Right, textData, caretData);
						ctx.flushText(0.0f);
					}


					if (owner.mHover || owner.mParent.lock()->mChildren[0]->mHover) {
						TMI::mHoveredStack = stack;
					}
				}
			}
		}
	};

	class OverlaySlotRenderer : public MinecraftUICustomRenderer {
	public:
		OverlaySlotRenderer() : MinecraftUICustomRenderer() {};

		virtual std::shared_ptr<UICustomRenderer> clone() const override {
			return std::make_shared<OverlaySlotRenderer>();
		}

		virtual void render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB) override {
			if (owner.mPropertyBag != nullptr && !owner.mPropertyBag->mJsonValue.isNull() && owner.mPropertyBag->mJsonValue.isObject()) {
				auto id = owner.mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt() + (TMI::mOverlayPage * TMI::mOverlayItemPerPage);
				if (id > -1 && id < TMI::overlayItemCount()) {
					ItemStack& stack = TMI::getOverlayItem(id);
					if (stack.isNull() || stack == ItemStack::EMPTY_ITEM) return;

					ClientInstance& client = _client.asInstance();

					// This disables the item visual from bobbing whenever the itemStack stack changes in content
					stack.mShowPickup = false;

					glm::tvec2<float> pos = owner.getPosition();

					BaseActorRenderContext renderCtx(*ctx.mScreenContext, client, *client.mMinecraftGame);
					int yOffset = stack.getItem()->getIconYOffset();

					// TODO: trying to fix fence block not rendering
					/*if (stack.getItem()->getLegacyBlock() != nullptr) {
						const BlockLegacy& renderBlock = *stack.getLegacyBlock()->getRenderBlock().mLegacyBlock;
						stack.reinit(renderBlock, 1);
						stack.mShowPickup = false;
					}*/

					renderCtx.itemRenderer->renderGuiItemNew(&renderCtx, &stack, 0, pos.x + 1.0f, pos.y - yOffset + 1.0f, false, 1.0f, mPropagatedAlpha, 1.0f);

					mce::Color color(1.0f, 1.0f, 1.0f, mPropagatedAlpha);
					ctx.flushImages(color, 1.0f, "ui_flush");

					if (stack.mCount > 1) {
						std::string text = std::format("{}", stack.mCount);
						float lineLength = ctx.getLineLength(*client.mMinecraftGame->mFontHandle.mDefaultFont, text, 1.0f, false);

						renderAABB._x0 = pos.x + (18.0f - lineLength);
						renderAABB._x1 = pos.x + 8.0f;
						renderAABB._y0 = pos.y + 10.0f;
						renderAABB._y1 = pos.y + 10.0f;

						TextMeasureData textData;
						memset(&textData, 0, sizeof(TextMeasureData));
						textData.fontSize = 1.0f;
						textData.renderShadow = true;

						CaretMeasureData caretData;
						memset(&caretData, 1, sizeof(CaretMeasureData));

						ctx.drawDebugText(renderAABB, text, mce::Color::WHITE, 1.0f, ui::Right, textData, caretData);
						ctx.flushText(0.0f);
					}


					if (owner.mHover || owner.mParent.lock()->mChildren[0]->mHover) {
						TMI::mHoveredStack = stack;
					}
				}
			}
		}
	};
	class OverlayGridSizerRenderer : public MinecraftUICustomRenderer {
		int lastW = 0;
		int lastH = 0;
	public:
		OverlayGridSizerRenderer() : MinecraftUICustomRenderer() {};

		virtual std::shared_ptr<UICustomRenderer> clone() const override {
			return std::make_shared<OverlayGridSizerRenderer>();
		}

		virtual void render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB) override {
			// Automatically caclulates the required stuffs
			// note that this is still a bit buggy, the size only changes after 2nd resize
			// the variable of the binding has changed, but the UI didnt detect it somehow
			int w = owner.mSize.x;//(renderAABB._x1 - renderAABB._x0);
			int h = owner.mSize.y;//(renderAABB._y1 - renderAABB._y0);
			if (lastW != w || lastH != h) {
				Log::Info("Custom grid {}, {}", owner.mSize.x, owner.mSize.y);
				lastW = w;
				lastH = h;

				auto json = Json::Value(Json::arrayValue);
				json.append(w / 18);
				json.append(h / 18);

				TMI::mOverlayItemPerPage = (w / 18) * (h / 18);
				if (TMI::mOverlayItemPerPage > 0)
					TMI::mOverlayMaxPage = TMI::overlayItemCount() / TMI::mOverlayItemPerPage;
				else
					TMI::mOverlayMaxPage = 0;

				owner.mChildren.front()->mPropertyBag->mJsonValue["#tmi_grid_size"] = json;
				refreshPage();
			}
		}
	};

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
					/*return ui::ViewRequest::Refresh;*/
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