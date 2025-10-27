#include "RecipeBrowserScreenController.hpp"

namespace TMI {
	class TestRenderer : public MinecraftUICustomRenderer {
	public:
		TestRenderer() : MinecraftUICustomRenderer() {};

		virtual std::shared_ptr<UICustomRenderer> clone() const override {
			return std::make_shared<TestRenderer>();
		}

		virtual void render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB) override {
			if (owner.mPropertyBag != nullptr && !owner.mPropertyBag->mJsonValue.isNull() && owner.mPropertyBag->mJsonValue.isObject()) {
				auto id = owner.mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt();
				auto recipe_index = owner.mPropertyBag->mJsonValue.get("#tmi_recipe_index", Json::Value(-1)).asInt() + (TMI::currentPage * 2) - 1;
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
						TMI::currentHoveredStack = stack;
					}
				}
			}
		}
	};



	SafetyHookInline _UIControlFactory__populateCustomRenderComponent;

	void UIControlFactory__populateCustomRenderComponent(UIControlFactory* self, const UIResolvedDef& resolved, UIControl& control) {
		std::string rendererType = resolved.getAsString("renderer");

		if (rendererType == "offhand_hud_renderer") {
			Log::Info("Making offhand_hud_renderer!");

			control.setComponent<CustomRenderComponent>(
				std::make_unique<CustomRenderComponent>(control)
			);

			CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
			component->setRenderer(std::make_shared<TestRenderer>());

			return;
		}

		_UIControlFactory__populateCustomRenderComponent.call<void, UIControlFactory*, const UIResolvedDef&, UIControl&>(self, resolved, control);
	}


	void RegisterOffhandHud()
	{
		Amethyst::HookManager& hooks = Amethyst::GetHookManager();
		HOOK(UIControlFactory, _populateCustomRenderComponent);
	}

	RecipeBrowserScreenController::RecipeBrowserScreenController(std::shared_ptr<ClientInstanceScreenModel> model, InteractionModel interaction, ItemStack& itemStack) : ClientInstanceScreenController(model), mItemStack(itemStack)
	{
		auto& player = *model->getPlayer();
		_registerBindings();
	}


	static void refreshPage() {
		if (TMI::currentPage < 0)
			TMI::currentPage = TMI::maxPage;
		if (TMI::currentPage > TMI::maxPage)
			TMI::currentPage = 0;
	}

	void RecipeBrowserScreenController::_registerBindings()
	{

		bindString("#title_text", [this]() {
			if (mItemStack.isNull()) return std::string("Crafting Recipes");
			Item& item = *mItemStack.getItem();
			return item.buildDescriptionName(mItemStack);
			}, []() { return true; });

		bindString("#page_text", [this]() {
			return std::format("{}/{}", TMI::currentPage + 1, TMI::maxPage + 1);
			}, []() { return true; });

		bindBool("#recipe2visible", [this]() {
			if (TMI::currentPage == TMI::maxPage && TMI::recipeCount() % 2 != 0) {
				return false;
			}
			return true;
			}, []() {return true; });

		bindBool("#prevBtnVisible", [this]() {
			if (TMI::currentPage == 0 || TMI::maxPage == 0) {
				return false;
			}
			return true;
			}, []() {return true; });

		bindBool("#nextBtnVisible", [this]() {
			if (TMI::currentPage == TMI::maxPage || TMI::maxPage == 0) {
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
				auto recipe_index = mPropertyBag->mJsonValue.get("#tmi_recipe_index", Json::Value(-1)).asInt() + (TMI::currentPage * 2) - 1;
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
				auto recipe_index = mPropertyBag->mJsonValue.get("#tmi_recipe_index", Json::Value(-1)).asInt() + (TMI::currentPage * 2) - 1;
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
			TMI::currentPage--;
			refreshPage();
			return ui::ViewRequest::Refresh;
			});
		this->registerButtonInteractedHandler(StringToNameId("tmi_next"), [this](UIPropertyBag* props) {
			TMI::currentPage++;
			refreshPage();
			return ui::ViewRequest::Refresh;
			});
	}
}