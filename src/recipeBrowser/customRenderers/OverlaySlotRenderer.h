#pragma once
#include <mc/src-client/common/client/gui/controls/renderers/MinecraftUICustomRenderer.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-client/common/client/gui/controls/UIPropertyBag.hpp>
#include "recipeBrowser/RecipeBrowserModule.hpp"

namespace TMI
{
    class OverlaySlotRenderer : public MinecraftUICustomRenderer
    {
    public:
        OverlaySlotRenderer() : MinecraftUICustomRenderer() {};

        virtual std::shared_ptr<UICustomRenderer> clone() const override
        {
            return std::make_shared<OverlaySlotRenderer>();
        }

        virtual void render(MinecraftUIRenderContext &ctx, IClientInstance &_client, UIControl &owner, int32_t pass, RectangleArea &renderAABB) override
        {
            if (owner.mPropertyBag != nullptr && !owner.mPropertyBag->mJsonValue.isNull() && owner.mPropertyBag->mJsonValue.isObject())
            {
                auto id = owner.mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt() + (TMI::mOverlayPage * TMI::mOverlayItemPerPage);
                if (id > -1 && id < TMI::overlayItemCount())
                {
                    ItemStack &stack = TMI::getOverlayItem(id);
                    if (stack.isNull() || stack == ItemStack::EMPTY_ITEM)
                        return;

                    ClientInstance &client = _client.asInstance();

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

                    if (owner.mHover || owner.mParent.lock()->mChildren[0]->mHover)
                    {
                        TMI::mHoveredStack = stack;
                    }
                }
            }
        }
    };
}