#include "State_Game.h"
#include "App.h"
#include <LWEUIManager.h>
#include <LWEAsset.h>
#include "Sprite.h"
#include "Renderer.h"

State &State_Game::Update(bool Tick, App *A, uint64_t lCurrentTime) {
	return *this;
}

State &State_Game::DrawFrame(App *A, Frame *F, Renderer *R) {
	LWTexture *Tex = A->GetAssetManager()->GetAsset("Test")->AsTexture();
	//Sprite s(Tex, LWVector2i(0, 0), LWVector2i(25));
	//R->WriteSprite(F, &s, LWVector2f(100.0f), 0.0f, LWVector2f(1.0f), LWVector4f(1.0f));

	return *this;
}

State &State_Game::ProcessInput(App *A, LWWindow *Window) {
	return *this;
}

State &State_Game::Activated(App *A) {
	return *this;
}

State &State_Game::Deactivated(App *A) {
	return *this;
}

State_Game::State_Game(App *A, LWEUIManager *UIMan) {
}