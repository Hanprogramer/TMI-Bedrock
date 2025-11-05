#include "TabIconRenderer.hpp"
#include "recipeBrowser/RecipeBrowserScreenController.hpp"
#include "recipeBrowser/RecipeBrowserModule.hpp"

TMI::TabIconRenderer::TabIconRenderer() : CustomItemRenderer()
{
}

std::shared_ptr<UICustomRenderer> TMI::TabIconRenderer::clone() const
{
    return std::make_shared<TabIconRenderer>();
}

void TMI::TabIconRenderer::render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB)
{
    if (owner.mPropertyBag != nullptr && !owner.mPropertyBag->mJsonValue.isNull() && owner.mPropertyBag->mJsonValue.isObject())
    {
        auto id = owner.mPropertyBag->mJsonValue.get("#tab_index", Json::Value(-1)).asInt();
        auto& controller = RecipeBrowserModule::getInstance();
        if (id > -1 && id < controller.controller->tabs.size())
        {
            ItemStack stack = controller.controller->tabs[id]->getIcon();
            if (stack.isNull() || stack == ItemStack::EMPTY_ITEM)
                return;

			this->drawItem(ctx, _client, stack, owner.getPosition(), 1.0f, 1.0f, 1.0f, renderAABB);

            if (owner.mHover || owner.mParent.lock()->mChildren[0]->mHover || owner.mParent.lock()->mChildren[1]->mHover)
            {
                controller.mHoveredText = controller.controller->currentTab->getTitle();
            }
        }
    }
}