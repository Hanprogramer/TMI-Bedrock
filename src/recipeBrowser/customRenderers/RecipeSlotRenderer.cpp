#include "RecipeSlotRenderer.hpp"
#include "recipeBrowser/RecipeBrowserScreenController.hpp"
#include "recipeBrowser/RecipeBrowserModule.hpp"

TMI::RecipeSlotRenderer::RecipeSlotRenderer() : CustomItemRenderer() {}

std::shared_ptr<UICustomRenderer> TMI::RecipeSlotRenderer::clone() const
{
	return std::make_shared<RecipeSlotRenderer>();
}

void TMI::RecipeSlotRenderer::render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB)
{
	if (owner.mPropertyBag != nullptr && !owner.mPropertyBag->mJsonValue.isNull() && owner.mPropertyBag->mJsonValue.isObject())
	{
		auto& controller = RecipeBrowserModule::getInstance();
		auto id = owner.mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt();
		auto recipe_index = owner.mPropertyBag->mJsonValue.get("#tmi_recipe_index", Json::Value(-1)).asInt() + (controller.controller->currentPage * controller.controller->currentTab->getItemPerPage());
		auto isResultSlot = owner.mPropertyBag->mJsonValue.get("#tmi_is_result_slot", Json::Value(false)).asBool();
		auto recipeType = owner.mPropertyBag->mJsonValue.get("#tmi_recipe_type", Json::Value("_unknown")).asString();
		if (id > -1 && recipe_index > -1 && recipe_index < controller.controller->currentTab->getItemCount())
		{
			ItemStack stack;
			if (isResultSlot)
			{
				stack = controller.controller->currentTab->getResult(recipe_index);
			}
			else
			{
				auto stacks = controller.controller->currentTab->getIngredient(id, recipe_index);
				if (stacks.size() > 0) {
					stack = stacks.front(); //TODO: Handle if no ingredient or multiple
				}
			}

			if (stack.isNull() || stack == ItemStack::EMPTY_ITEM)
				return;

			this->drawItem(ctx, _client, stack, owner.getPosition(), 1.0f, 1.0f, 1.0f, renderAABB);

			if (owner.mHover || owner.mParent.lock()->mChildren[0]->mHover)
			{
				controller.mHoveredStack = stack;
			}
		}
	}
}