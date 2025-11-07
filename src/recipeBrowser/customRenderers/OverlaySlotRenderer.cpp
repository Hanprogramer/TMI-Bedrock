#include "OverlaySlotRenderer.hpp"
#include "recipeBrowser/RecipeBrowserModule.hpp"

#include <mc/src/common/locale/I18n.hpp>
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
				if (mRecipeMod.isCheatEnabled()) {
					if (mRecipeMod.isAddItemKeyHeld())
						mRecipeMod.mHoveredText = mRecipeMod.getItemTooltipText(stack) 
							+ "\n" + std::vformat("text.tmi.overlay_press_add"_i18n, std::make_format_args(stack.getItem()->mMaxStackSize)) 
							+ "\n" + "text.tmi.overlay_press_add_secondary"_i18n;
					else {

						auto& options = *Amethyst::GetClientCtx().mOptions;
						auto& mapping = *options.getCurrentKeyboardRemapping();
						auto* keymapping = mapping.getKeymappingByAction("key.tmi.cheat_add_item");
						std::string keyName = keymapping ? mapping.getMappedKeyName(*keymapping) : "Unknown";

						mRecipeMod.mHoveredText = mRecipeMod.getItemTooltipText(stack) + "\n" + std::vformat("text.tmi.overlay_hold_add_item"_i18n, std::make_format_args(keyName));
					}
				}
				else
					mRecipeMod.mHoveredStack = stack;
			}
		}
	}
}
