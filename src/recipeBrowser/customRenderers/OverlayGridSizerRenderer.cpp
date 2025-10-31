#include "OverlayGridSizerRenderer.hpp"

inline TMI::OverlayGridSizerRenderer::OverlayGridSizerRenderer(RecipeBrowserModule* recipeMod) : MinecraftUICustomRenderer(), mRecipeMod(recipeMod) {}

inline std::shared_ptr<UICustomRenderer> TMI::OverlayGridSizerRenderer::clone() const {
	return std::make_shared<OverlayGridSizerRenderer>();
}

inline void TMI::OverlayGridSizerRenderer::render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB) {
	// Automatically caclulates the required stuffs
	// note that this is still a bit buggy, the size only changes after 2nd resize
	// the variable of the binding has changed, but the UI didnt detect it somehow
	int w = owner.mSize.x;//(renderAABB._x1 - renderAABB._x0);
	int h = owner.mSize.y;//(renderAABB._y1 - renderAABB._y0);
	if (lastW != w || lastH != h) {
		Log::Info("Custom grid {}, {}", owner.mSize.x, owner.mSize.y);
		lastW = w;
		lastH = h;

		auto json = Json::Value(Json::arrayValue);
		json.append(w / 18);
		json.append(h / 18);

		mRecipeMod->mOverlayItemPerPage = (w / 18) * (h / 18);

		if (mRecipeMod->mOverlayItemPerPage > 0)
			mRecipeMod->mOverlayMaxPage = mRecipeMod->overlayItemCount() / mRecipeMod->mOverlayItemPerPage;
		else
			mRecipeMod->mOverlayMaxPage = 0;

		owner.mChildren.front()->mPropertyBag->mJsonValue["#tmi_grid_size"] = json;
		mRecipeMod->refreshOverlayPage();
	}
}
