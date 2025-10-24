#include "CrossairInfoModule.h"
#include <amethyst/runtime/ModContext.hpp>
#include <mc/src-client/common/client/renderer/BaseActorRenderContext.hpp>

std::string CrossairInfoModule::getModNameFromNamespace(std::string mNamespace) {
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

void CrossairInfoModule::Init() {
	Amethyst::GetEventBus().AddListener<BeforeRenderUIEvent>(CrossairInfoModule::OnAfterRenderUi);
}

void CrossairInfoModule::OnAfterRenderUi(BeforeRenderUIEvent event) {
	ClientInstance& mc = *Amethyst::GetClientCtx().mClientInstance;
	if (event.screen.visualTree->mRootControlName->mName == "hud_screen") {
		const int padding = 4;
		const int topMargin = 8;
		const int maxWidth = 200;
		const int maxHeight = 100;
		const int itemIconWidth = 24;

		MinecraftUIRenderContext& ctx = event.ctx;
		Vec2 screenSize = mc.mGuiData->clientUIScreenSize;
		Font& font = *ctx.mDebugTextFontHandle.mDefaultFont.get();
		auto fontHandlePtr = Bedrock::NonOwnerPointer<const FontHandle>(&ctx.mDebugTextFontHandle);

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

		ILevel* level = mc.getLocalPlayer()->getLevel();
		HitResult& hr = level->getHitResult();
		if (hr.mType == HitResultType::TILE) {
			auto& block = mc.getRegion()->getBlock(hr.mBlock);
			auto& i18n = getI18n();
			std::string blockId = block.mLegacyBlock->mNameInfo.mRawName.getString();
			std::string langKey = block.mLegacyBlock->buildDescriptionId(block);
			std::string translated = i18n.get(langKey, nullptr);
			std::string subtitleStr = std::string("§o");
			std::string mNamespace = getModNameFromNamespace(block.mLegacyBlock->mNameInfo.mNamespaceName);
			subtitleStr.append(mNamespace);
			const char* title = translated.c_str();
			const char* subtitle = subtitleStr.c_str();

			UIMeasureStrategy& meassurer = ctx.mMeasureStrategy;
			MeasureResult result = meassurer.measureText(fontHandlePtr, title, maxWidth, maxHeight, tmd, cmd);
			MeasureResult result2 = meassurer.measureText(fontHandlePtr, subtitle, maxWidth, maxHeight, tmd, cmd);

			float height = result.mSize.y + result2.mSize.y + tmd.linePadding;
			float width = std::max(result.mSize.x, result2.mSize.x) + itemIconWidth;
			float x = screenSize.x / 2 - width / 2 - padding;
			float y = topMargin;


			// Draw the background
			Amethyst::NinesliceHelper backgroundSlice(16, 16, 3, 3);
			mce::TexturePtr texture = ctx.getTexture("textures/ui/hud_text_bg", true);
			RectangleArea bgRect = RectangleArea(
				x,
				x + width + padding * 2,
				y,
				y + height + padding * 2);
			backgroundSlice.Draw(bgRect, &texture, &ctx);
			HashedString flushStringUI(0xA99285D21E94FC80, "ui_flush");
			ctx.flushImages(mce::Color::WHITE, 1.0f, flushStringUI);



			// Draw the icon
			BaseActorRenderContext renderCtxPtr = BaseActorRenderContext(*ctx.mScreenContext, *ctx.mClient, *ctx.mClient->mMinecraftGame);

			// Create the itemstack
			ItemStack* stack = new ItemStack();
			const auto& legacyBlock = block.mLegacyBlock;
			stack->reinit(*legacyBlock, 1);

			// Render the item
			auto mat = Matrix::IDENTITY;
			float scaleX = 2.6f;
			mat.scale(scaleX, 1.0f, 1.0f);

			renderCtxPtr.mScreenContext->camera->worldMatrixStack.push(mat);
			renderCtxPtr.itemRenderer->renderGuiItemNew(&renderCtxPtr, stack, renderCtxPtr.mLastFrameTime, (x + padding) / scaleX - 4, y + padding + 6, false, 1.0f, 1.0f, 0.95f);
			ctx.flushImages(mce::Color::WHITE, 1.0f, flushStringUI);
			renderCtxPtr.mScreenContext->camera->worldMatrixStack.pop();
			


			// Draw the text
			ctx.drawText(font,
				RectangleArea(
					x + padding + itemIconWidth,
					x + width + padding,
					y + padding, 
					height), title, mce::Color::WHITE, 1.0f, ui::TextAlignment::Center,
				tmd,
				cmd);
			ctx.drawText(font,
				RectangleArea(
					x + padding + itemIconWidth,
					x + width + padding,
					y + padding + result.mSize.y + tmd.linePadding, 
					height), subtitle, mce::Color::BLUE, 1.0f, ui::TextAlignment::Center,
				tmd,
				cmd);
			ctx.flushText(1.0f);
		}
	}
}

