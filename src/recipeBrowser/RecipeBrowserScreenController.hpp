#pragma once
#include <mc/src-client/common/client/gui/screens/controllers/ClientInstanceScreenController.hpp>
#include <mc/src-client/common/client/gui/screens/ScreenController.hpp>
#include <mc/src-client/common/client/gui/screens/UIScene.hpp>

namespace TMI
{
}


class RecipeBrowserScreenController : public ClientInstanceScreenController {
public:
	RecipeBrowserScreenController(std::shared_ptr<ClientInstanceScreenModel> model) : ClientInstanceScreenController(model) {

	}
};