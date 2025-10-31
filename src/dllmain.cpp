#include "dllmain.hpp"


// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(AmethystContext& ctx, const Amethyst::Mod& mod)
{
	// Initialize Amethyst mod backend
	Amethyst::InitializeAmethystMod(ctx, mod);
	CrossairInfoModule::Init();
	auto& recipeMod = TMI::RecipeBrowserModule::getInstance();
	recipeMod.initRecipeBrowser();
}
