#include "OverlaySlotRenderer.hpp"
#include "recipeBrowser/RecipeBrowserModule.hpp"



TMI::OverlaySlotRenderer::OverlaySlotRenderer() : CustomItemRenderer() {
}

std::shared_ptr<UICustomRenderer> TMI::OverlaySlotRenderer::clone() const
{
	return std::make_shared<OverlaySlotRenderer>();
}

void TMI::OverlaySlotRenderer::render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB)
{
	if (owner.mPropertyBag != nullptr && !owner.mPropertyBag->mJsonValue.isNull() && owner.mPropertyBag->mJsonValue.isObject())
	{
		auto& mRecipeMod = RecipeBrowserModule::getInstance();
		auto id = owner.mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt() + (mRecipeMod.mOverlayPage * mRecipeMod.mOverlayItemPerPage);
		if (id > -1 && id < mRecipeMod.overlayItemCount())
		{
			ItemStack& stack = mRecipeMod.getOverlayItem(id);
			if (stack.isNull() || stack == ItemStack::EMPTY_ITEM && stack.getItem() == nullptr)
				return;

			drawItem(ctx, _client, stack, owner.getPosition(), 1.0f, 1.0f, 1.0f, renderAABB);

			if (owner.mHover || owner.mParent.lock()->mChildren[0]->mHover)
			{
				mRecipeMod.mHoveredStack = stack;
			}
		}
	}
}
