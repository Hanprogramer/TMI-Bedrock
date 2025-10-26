#include "RecipeBrowserModule.hpp"
#include <amethyst/ui/NinesliceHelper.hpp>
#include <mc/src/common/locale/I18n.hpp>
#include <mc/src-client/common/client/gui/screens/SceneCreationUtils.hpp>
#include <mc/src-client/common/client/gui/screens/models/ClientInstanceScreenModel.hpp>
#include <mc/src-client/common/client/gui/screens/controllers/ContainerScreenController.hpp>
#include "RecipeBrowserScreenController.hpp"
#include <mc/src/common/network/packet/InventoryContentPacket.hpp>

using namespace TMI;
bool initialized = false;

std::map<std::string, ItemStack> itemMap;
int itemCount = 0;
int mposX = 0;
int mposY = 0;
boolean isMousePressed = false;
boolean isMouseJustReleased = false;

const char BUTTON_LEFT = 0x263b;
const char BUTTON_RIGHT = 0x263a;

char last_button;

std::string getModNameFromNamespace(std::string mNamespace) {
	std::string itemNamespace = mNamespace;
	std::string modName;
	std::weak_ptr<const Amethyst::Mod> mod;
	if (itemNamespace.empty() || itemNamespace == "minecraft")
	{
		modName = "Minecraft";
	}
	else if (!(mod = Amethyst::GetContext().mModLoader->GetModByNamespace(itemNamespace)).expired()) {
		modName = mod.lock()->mInfo->FriendlyName;
	}
	else {
		modName.reserve(itemNamespace.size());
		bool cap = true;
		for (char c : itemNamespace) {
			if (c == '_') {
				modName.push_back(' ');
				cap = true;
			}
			else if (cap) {
				modName.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(c))));
				cap = false;
			}
			else {
				modName.push_back(c);
			}
		}
	}
	return modName;
}

std::string getItemName(Item* item) {
	auto& i18n = getI18n();
	auto isItem = item->getLegacyBlock() == nullptr;
	std::string langKey = (isItem ? "item." : "tile.") +
		item->mRawNameId.getString() + ".name";
	return i18n.get(langKey, nullptr);
}

void drawFakeTooltip(ItemStack stack, MinecraftUIRenderContext& ctx) {
	// Draw the background
	ClientInstance& mc = *Amethyst::GetClientCtx().mClientInstance;
	Vec2 screenSize = mc.mGuiData->clientUIScreenSize;
	Font& font = *ctx.mDebugTextFontHandle.mDefaultFont.get();
	auto fontHandlePtr = Bedrock::NonOwnerPointer<const FontHandle>(&ctx.mDebugTextFontHandle);

	Item* item = stack.getItem();
	std::string subtitleStr = std::string("§o");
	std::string mNamespace = getModNameFromNamespace(item->mNamespace);
	const int padding = 4;
	const int maxWidth = 200;
	const int maxHeight = 100;
	subtitleStr.append(mNamespace);
	const char* title = getItemName(item).c_str();
	const char* subtitle = subtitleStr.c_str();


	TextMeasureData tmd = TextMeasureData{
			1,  // fontSize
			0, //linepadding
			true, // renderShadow
			false, // showColorSymbol
			false, // hideHyphen
	};
	CaretMeasureData cmd = CaretMeasureData{
			0, false
	};

	UIMeasureStrategy& meassurer = ctx.mMeasureStrategy;
	MeasureResult result = meassurer.measureText(fontHandlePtr, title, maxWidth, maxHeight, tmd, cmd);
	MeasureResult result2 = meassurer.measureText(fontHandlePtr, subtitle, maxWidth, maxHeight, tmd, cmd);

	float height = result.mSize.y + result2.mSize.y + tmd.linePadding;
	float width = std::max(result.mSize.x, result2.mSize.x);
	float mGuiScale = mc.mGuiData->mGuiScale;
	float x = mposX / mGuiScale + 10;
	float y = mposY / mGuiScale + 10;


	// Draw the background
	Amethyst::NinesliceHelper backgroundSlice(16, 16, 3, 3);
	mce::TexturePtr texture = ctx.getTexture("textures/ui/hud_text_bg", true);
	RectangleArea bgRect = RectangleArea(
		x,
		x + width + padding * 2,
		y,
		y + height + padding * 2);
	backgroundSlice.Draw(bgRect, &texture, &ctx);
	ctx.flushImages(mce::Color::WHITE, 1.0f, "ui_flush");

	// Draw the text
	ctx.drawText(font,
		RectangleArea(
			x + padding,
			x + width + padding,
			y + padding,
			height), title, mce::Color::WHITE, 1.0f, ui::TextAlignment::Center,
		tmd,
		cmd);
	ctx.drawText(font,
		RectangleArea(
			x + padding,
			x + width + padding,
			y + padding + result.mSize.y + tmd.linePadding,
			height), subtitle, mce::Color::BLUE, 1.0f, ui::TextAlignment::Center,
		tmd,
		cmd);
	ctx.flushText(1.0f);
}

void OnAfterRenderUI(AfterRenderUIEvent event)
{
	const std::string name = event.screen.visualTree->mRootControlName->mName;
	auto& mc = Amethyst::GetClientCtx().mClientInstance;
	auto& ctx = event.ctx;
	if (name == "inventory_screen")
	{
		if (!initialized)
		{

			ItemRegistryRef itemRegistryRef = mc->getLocalPlayer()->getLevel()->getItemRegistry();
			std::shared_ptr<ItemRegistry> sharedRegistryPtr = itemRegistryRef._lockRegistry();
			ItemRegistry& itemRegistry = *sharedRegistryPtr;

			for (int i = 0; i < itemRegistry.mMaxItemID; i++) {
				auto itemRef = itemRegistry.mIdToItemMap[i];
				if (!itemRef.isNull()) {
					Item& item = *itemRef;
					ItemStack stack;
					stack.reinit(item, 1, 0);
					itemMap.insert(std::pair<std::string, ItemStack>(item.getFullItemName(), stack));
					itemCount++;
				}
			}

			//Recipes recipes = mc->getLocalPlayer()->getLevel()->getRecipes();

			initialized = true;

		}
		BaseActorRenderContext renderCtxPtr = BaseActorRenderContext(*ctx.mScreenContext, *ctx.mClient, *ctx.mClient->mMinecraftGame);
		int i = 0;
		int y = 0;
		int x = 0;
		const int width = 20;
		const int height = 20;
		int mposXX = mposX / mc->mGuiData->mGuiScale;
		int mposYY = mposY / mc->mGuiData->mGuiScale;
		int hovered = -1;
		std::string hoveredID = "";
		for (const auto& [key, stack] : itemMap) {
			int xx = x * width;
			int yy = y * height;
			if (mposXX > xx && mposXX < xx + width && mposYY > yy && mposYY < yy + height) {
				// Draw hover bg
				ctx.fillRectangle(RectangleArea(xx, xx + width, yy, yy + height), mce::Color(1.0f, 1.0f, 1.0f, 0.5f), 0.5f);
				hovered = i;
				hoveredID = key;
				if (isMouseJustReleased) {
					Log::Info("Pressed: {}", stack.getItem()->mDescriptionId);
					// Open the screen
					// mc->mSceneFactory->createUIScene(*ctx.mClient->mMinecraftGame, *ctx.mClient, "", *event.ctx.mCurrentScene)
					// mc->mSceneStack->pushScreen();

					bool isClientSide = true; // only runs on client side
					auto& clientInstance = *Amethyst::GetClientCtx().mClientInstance;
					auto& game = *clientInstance.mMinecraftGame;
					if (isClientSide) {
						auto& factory = *clientInstance.mSceneFactory;
						auto model = SceneCreationUtils::_createModel<ClientInstanceScreenModel>(
							factory,
							game,
							clientInstance,
							factory.mAdvancedGraphicOptions
						);
						auto controller = std::make_shared<RecipeBrowserScreenController>(model);
						auto item = stack.getItem();
						controller->bindString("#title_text", [item]() { return getItemName(item);  }, []() { return true; });

						auto scene = factory.createUIScene(game, clientInstance, "tmi.recipe_screen", controller);
						auto screen = factory._createScreen(scene);
						factory.getCurrentSceneStack()->pushScreen(screen, false);
					}
					else {
						//auto& minecraft = *Amethyst::GetServerCtx().mMinecraft;
						//ServerPlayer& serverPlayer = static_cast<ServerPlayer&>(player);
						//auto id = serverPlayer.nextContainerId();
						//auto containerManager = std::make_shared<RecipeBrowserScreenController>(id, player);
						//containerManager->postInit();
						//serverPlayer.setContainerManagerModel(containerManager);
						//ContainerOpenPacket packet(containerManager->getContainerId(), containerManager->getContainerType(), BlockPos(0, 0, 0), player.getUniqueID());
						//serverPlayer.sendNetworkPacket(packet);
						//InventoryContentPacket invPacket = InventoryContentPacket::fromPlayerInventoryId(containerManager->getContainerId(), serverPlayer);
						//serverPlayer.sendNetworkPacket(invPacket);
					}
				}
			}
			renderCtxPtr.itemRenderer->renderGuiItemNew(&renderCtxPtr, &stack, 0, xx + 2, yy + 2, false, 1.0f, 1.0f, 0.95f);
			x++;
			if (x > 5) {
				x = 0;
				y += 1;
			}
			i++;
			if (i > 200) break;
		}
		ctx.flushImages(mce::Color::WHITE, 1.0f, "ui_flush");

		if (hovered > -1) {
			drawFakeTooltip(itemMap.at(hoveredID), ctx);
			ctx.flushImages(mce::Color::WHITE, 1.0f, "ui_flush");
		}
	}
	isMouseJustReleased = false;
}

void OnMouseInput(MouseInputEvent event) {
	mposX = event.x;
	mposY = event.y;
	boolean isPressed = !event.mButtonData == char();

	if (!isPressed && isMousePressed) {
		isMouseJustReleased = true;
	}

	last_button = event.mActionButtonId;
	isMousePressed = isPressed;

	//Log::Info("buttonData: {}, actionButtonId: {}", event.mButtonData, event.mActionButtonId);
}

void RecipeBrowserModule::Init()
{
	Amethyst::GetEventBus().AddListener<AfterRenderUIEvent>(&OnAfterRenderUI);
	Amethyst::GetEventBus().AddListener<MouseInputEvent>(&OnMouseInput);
}