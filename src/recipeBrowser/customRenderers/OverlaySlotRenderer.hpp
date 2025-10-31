#pragma once
#include <mc/src-client/common/client/gui/controls/renderers/MinecraftUICustomRenderer.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-client/common/client/gui/controls/UIPropertyBag.hpp>

namespace TMI
{
    class OverlaySlotRenderer : public MinecraftUICustomRenderer
    {
    public:
        OverlaySlotRenderer();
        virtual std::shared_ptr<UICustomRenderer> clone() const override;
        virtual void render(MinecraftUIRenderContext &ctx, IClientInstance &_client, UIControl &owner, int32_t pass, RectangleArea &renderAABB) override;
    };
}