#include "RecipeBrowserScreenController.hpp"
#include "RecipeBrowserModule.hpp"
#include "tabs/CraftingTab.hpp"
#include "tabs/FurnaceTab.hpp"

namespace TMI {
	CraftingTab* craftingTab;
	FurnaceTab* furnaceTab;

	RecipeBrowserScreenController::RecipeBrowserScreenController(RecipeBrowserModule* module, std::shared_ptr<ClientInstanceScreenModel> model, InteractionModel interaction, ItemStack& itemStack) : ClientInstanceScreenController(model),
		mItemStack(itemStack), mModule(module)
	{
		mItemStack = itemStack;
		auto& player = *model->getPlayer();

		tabs = std::vector<TMITab*>();
		craftingTab = new CraftingTab(mModule);
		furnaceTab = new FurnaceTab(mModule);
		init();

		_registerBindings();
	}

	void RecipeBrowserScreenController::init() {
		// Add the tabs
		tabs.clear();

		if (mModule->mCraftingRecipes.size() > 0)
			tabs.push_back(craftingTab);

		if (mModule->mFurnaceRecipes.size() > 0)
			tabs.push_back(furnaceTab);

		if (tabs.size() == 0) {
			this->tryExit();
			return;
		}

		currentTab = tabs[0];
		currentTabIndex = 0;

		currentPage = 0;
		maxPage = currentTab->getMaxPage();
	}

	void RecipeBrowserScreenController::_registerBindings()
	{
		bindString("#title_text", [this]() {
			return currentTab->getTitle();
			}, []() { return true; });

		bindString("#page_text", [this]() {
			return std::format("{}/{}", currentPage + 1, maxPage + 1);
			}, []() { return true; });

		bindBool("#prevBtnVisible", [this]() {
			if (currentPage == 0 || maxPage == 0) {
				return false;
			}
			return true;
			}, []() { return true; });

		bindBool("#nextBtnVisible", [this]() {
			if (currentPage == maxPage || maxPage == 0) {
				return false;
			}
			return true;
			}, []() { return true; });

		bindFloat("#tmi_current_tab", [this]() {
			return (float)currentTabIndex;
			},
			[]() { return true; }
		);
		bindFloat("#tmi_tab_count", [this]() {
			return tabs.size();
			},
			[]() { return true; }
		);

		bindString("#tmi_current_recipe_type", [this]() {
			return currentTab->getID();
			},
			[]() { return true; }
		);

		// Recipe count per page
		bindFloat("#tmi_recipe_count", [&]() {
			if (currentPage < 0 || currentPage > maxPage) {
				return 0.0f;
			}
			int startItem = currentPage * currentTab->getItemPerPage();
			int remainingItems = currentTab->getItemCount() - startItem;

			return  std::fminf(currentTab->getItemPerPage(), remainingItems);
			}, []() { return true; });

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
				auto recipe_index = mPropertyBag->mJsonValue.get("#tmi_recipe_index", Json::Value(-1)).asInt() + (currentPage * currentTab->getItemPerPage());
				auto isResultSlot = mPropertyBag->mJsonValue.get("#tmi_is_result_slot", Json::Value(false)).asBool();

				if (id > -1 && recipe_index > -1 && recipe_index < currentTab->getItemCount()) {
					ItemStack stack;
					if (isResultSlot) {
						stack = currentTab->getResult(recipe_index);
					}
					else {
						auto stacks = currentTab->getIngredient(id, recipe_index);
						if (stacks.size() > 0)
							stack = stacks.front(); //TODO: Handle if no ingredient
					}
					if (stack.isNull()) return ui::ViewRequest::Refresh;
					if (mModule->setRecipesForItem(*stack.getItem())) {
						init();
					}
				}
			}
			return ui::ViewRequest::Refresh;
			});

		this->registerButtonInteractedHandler(StringToNameId("tmi_slot_pressed_secondary"), [this](UIPropertyBag* mPropertyBag) {
			if (mPropertyBag != nullptr && !mPropertyBag->mJsonValue.isNull() && mPropertyBag->mJsonValue.isObject()) {
				auto id = mPropertyBag->mJsonValue.get("#tmi_slot_id", Json::Value(-1)).asInt();
				auto recipe_index = mPropertyBag->mJsonValue.get("#tmi_recipe_index", Json::Value(-1)).asInt() + (currentPage * currentTab->getItemPerPage());
				auto isResultSlot = mPropertyBag->mJsonValue.get("#tmi_is_result_slot", Json::Value(false)).asBool();

				if (id > -1 && recipe_index > -1 && recipe_index < currentTab->getItemCount()) {
					ItemStack stack;
					if (isResultSlot) {
						stack = currentTab->getResult(recipe_index);
					}
					else {
						auto stacks = currentTab->getIngredient(id, recipe_index);
						if (stacks.size() > 0)
							stack = stacks.front(); //TODO: Handle if no ingredient
					}

					if (stack.isNull()) return ui::ViewRequest::Refresh;
					if (mModule->setRecipesFromItem(*stack.getItem())) {
						init();
					}
				}
			}
			return ui::ViewRequest::Refresh;
			});
		this->registerButtonInteractedHandler(StringToNameId("tmi_tab_pressed"), [this](UIPropertyBag* mPropertyBag) {
			if (mPropertyBag != nullptr && !mPropertyBag->mJsonValue.isNull() && mPropertyBag->mJsonValue.isObject()) {
				auto id = mPropertyBag->mJsonValue.get("#tab_index", Json::Value(-1)).asInt();
				if (id > -1 && id < tabs.size()) {
					currentTabIndex = id;
					currentPage = 0;
					currentTab = tabs[id];
					maxPage = currentTab->getMaxPage();
				}
			}
			return ui::ViewRequest::Refresh;
			});

		this->registerButtonInteractedHandler(StringToNameId("tmi_prev"), [this](UIPropertyBag* props) {
			currentPage--;
			refreshPage();
			return ui::ViewRequest::Refresh;
			});
		this->registerButtonInteractedHandler(StringToNameId("tmi_next"), [this](UIPropertyBag* props) {
			currentPage++;
			refreshPage();
			return ui::ViewRequest::Refresh;
			});
	}
	void RecipeBrowserScreenController::refreshPage()
	{
		if (currentPage < 0)
			currentPage = maxPage;
		if (currentPage > maxPage)
			currentPage = 0;
		mModule->refreshOverlayPage();
	}
}