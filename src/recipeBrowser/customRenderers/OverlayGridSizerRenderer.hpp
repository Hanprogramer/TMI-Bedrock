#pragma once
#include <mc/src-client/common/client/gui/controls/renderers/MinecraftUICustomRenderer.hpp>
#include <mc/src-client/common/client/gui/gui/UIControl.hpp>
#include <mc/src-client/common/client/gui/controls/UIPropertyBag.hpp>
#include "recipeBrowser/RecipeBrowserModule.hpp"

namespace TMI {
    
	class OverlayGridSizerRenderer : public MinecraftUICustomRenderer {
		int lastW = 0;
		int lastH = 0;
	public:
		OverlayGridSizerRenderer() : MinecraftUICustomRenderer() {};

		virtual std::shared_ptr<UICustomRenderer> clone() const override {
			return std::make_shared<OverlayGridSizerRenderer>();
		}

		virtual void render(MinecraftUIRenderContext& ctx, IClientInstance& _client, UIControl& owner, int32_t pass, RectangleArea& renderAABB) override {
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

				TMI::mOverlayItemPerPage = (w / 18) * (h / 18);
				if (TMI::mOverlayItemPerPage > 0)
					TMI::mOverlayMaxPage = TMI::overlayItemCount() / TMI::mOverlayItemPerPage;
				else
					TMI::mOverlayMaxPage = 0;

				owner.mChildren.front()->mPropertyBag->mJsonValue["#tmi_grid_size"] = json;
				refreshPage();
			}
		}
	};
}