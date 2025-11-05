#pragma once
#include <mc/src-client/common/client/gui/controls/renderers/MinecraftUICustomRenderer.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-client/common/client/gui/controls/UIPropertyBag.hpp>
#include <mc/src/common/world/item/ItemStack.hpp>
#include <mc/src-client/common/client/renderer/screen/MinecraftUIRenderContext.hpp>
#include <mc/src-client/common/client/renderer/BaseActorRenderContext.hpp>
#include <mc/src-client/common/client/game/ClientInstance.hpp>
#include <mc/src-client/common/client/renderer/actor/ItemRenderer.hpp>
#include <mc/src/common/world/item/Item.hpp>
#include <mc/src-client/common/client/game/MinecraftGame.hpp>

namespace TMI {
	class CustomItemRenderer : public MinecraftUICustomRenderer {
	public:
		CustomItemRenderer() : MinecraftUICustomRenderer() {}
		virtual std::shared_ptr<UICustomRenderer> clone() const
		{
			return std::make_shared<CustomItemRenderer>();
		};
		virtual void render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB) {

		}

        void drawItem(MinecraftUIRenderContext& ctx, IClientInstance& _client, ItemStack& stack, glm::tvec2<float> pos, float transparency, float lightMultiplyer, float scale, RectangleArea& renderAABB) {
			
			ClientInstance& client = _client.asInstance();
			auto& item = *stack.getItem();

			//if (stack.isBlock()) {
			//	const BlockLegacy& renderBlock = *stack.getLegacyBlock()->getRenderBlock().mLegacyBlock;
			//	stack.reinit(renderBlock, 1);
			//}

			// This disables the item visual from bobbing whenever the itemStack stack changes in content
			stack.mShowPickup = false;

			BaseActorRenderContext renderCtx(*ctx.mScreenContext, client, *client.mMinecraftGame);
			int yOffset = item.getIconYOffset();

			auto* player = client.getLocalPlayer();
			auto frame = item.getAnimationFrameFor((Mob*)player, false, &stack, true);

			renderCtx.itemRenderer->renderGuiItemNew(&renderCtx, &stack, frame, pos.x + 1.0f, pos.y - yOffset + 1.0f, false, 1.0f, mPropagatedAlpha, 1.0f);
			if (item.isGlint(stack))
				renderCtx.itemRenderer->renderGuiItemNew(&renderCtx, &stack, frame, pos.x + 1.0f, pos.y - yOffset + 1.0f, true, 1.0f, mPropagatedAlpha, 1.0f);

			mce::Color color(1.0f, 1.0f, 1.0f, mPropagatedAlpha);
			ctx.flushImages(color, 1.0f, "ui_flush");
			
			if (stack.mCount > 1)
			{
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
        }
	};
}