#include "RecipeBrowserScreenController.hpp"

namespace TMI {
	class TestRenderer : public MinecraftUICustomRenderer {
	public:
		TestRenderer() : MinecraftUICustomRenderer() {};

		virtual std::shared_ptr<UICustomRenderer> clone() const override {
			return std::make_shared<TestRenderer>();
		}

		virtual void render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB) override {
			ClientInstance& client = _client.asInstance();
			LocalPlayer* player = client.getLocalPlayer();
			if (!player || mPropagatedAlpha < 0.5) return;

			const ItemStack& itemStack = player->playerInventory->getSelectedItem();
			if (itemStack.isNull()) return;

			// This disables the item visual from bobbing whenever the itemStack stack changes in content
			ItemStack itemStackCopy = TMI::selectedItemStack;
			itemStackCopy.mShowPickup = false;

			glm::tvec2<float> pos = owner.getPosition();

			BaseActorRenderContext renderCtx(*ctx.mScreenContext, client, *client.mMinecraftGame);
			int yOffset = itemStack.getItem()->getIconYOffset();

			renderCtx.itemRenderer->renderGuiItemNew(&renderCtx, &itemStackCopy, 0, pos.x + 1.0f, pos.y - yOffset + 1.0f, false, 1.0f, mPropagatedAlpha, 1.0f);
			mce::Color color(1.0f, 1.0f, 1.0f, mPropagatedAlpha);
			ctx.flushImages(color, 1.0f, "ui_flush");

			if (itemStack.mCount == 1) return;

			std::string text = std::format("{}", itemStack.mCount);
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

	RecipeBrowserScreenController::RecipeBrowserScreenController(std::shared_ptr<ClientInstanceScreenModel> model, InteractionModel interaction, const Item* item) : ClientInstanceScreenController(model), mItem(item)
	{
		auto& player = *model->getPlayer();
		_registerBindings();
	}

	void RecipeBrowserScreenController::_registerBindings()
	{
		std::string itemName = std::format("item.{}.name", mItem->mFullName.getString());
		itemName = getI18n().get(itemName, nullptr);

		bindString("#title_text", [itemName]() {
			return itemName;
			}, []() { return true; });


		auto self = this;
		this->registerButtonInteractedHandler(StringToNameId("tmi_close_modal"), [self](UIPropertyBag* props) {
			auto& clientInstance = *Amethyst::GetClientCtx().mClientInstance;
			auto& game = *clientInstance.mMinecraftGame;
			auto& factory = *clientInstance.mSceneFactory;

			factory.getCurrentSceneStack()->schedulePopScreen(1);
			return ui::ViewRequest::Exit;
			});

		this->mControlCreateCallback = [](const std::string a, const UIPropertyBag& props) {
			Log::Info("Creating UI Control {}", a);
			};

	}

	void RecipeBrowserScreenController::onDelete()
	{
		Log::Info("Deleting");
	}

	ui::ViewRequest RecipeBrowserScreenController::tryExit()
	{
		Log::Info("Try exiting");
		return ui::ViewRequest::None;
	}

	void RecipeBrowserScreenController::leaveScreen(const std::string& arg1)
	{
		Log::Info("Leaving {}", arg1);
	}

	void RecipeBrowserScreenController::onDictationEvent(const std::string& arg1)
	{
		Log::Info("OnDictation {}", arg1);
	}


	void RecipeBrowserScreenController::onLeave()
	{
		// DO nothing
		Log::Info("is leaving");
	}
}