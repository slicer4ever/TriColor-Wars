#include "State_Menu.h"
#include <LWEUIManager.h>
#include "App.h"

State &State_Menu::Update(bool Tick, App *A, uint64_t lCurrentTime) {
	return *this;
}

State &State_Menu::DrawFrame(App *A, Frame *F, Renderer *R) {
	return *this;
}

State &State_Menu::ProcessInput(App *A, LWWindow *Window) {
	return *this; 
}

State &State_Menu::Activated(App *A) {
	m_MenuRect->SetVisible(true);
	return *this;
}

State &State_Menu::Deactivated(App *A) {
	m_MenuRect->SetVisible(false);
	return *this;
}

void State_Menu::SinglePlayBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	A->SetActiveState(State::Game);
	return;
}

void State_Menu::DoublePlayBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	return;
}

void State_Menu::InstructionBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	return;
}

void State_Menu::QuitBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	A->SetTerminate();
	return;
}

State_Menu::State_Menu(App *A, LWEUIManager *UIManager) : State() {
	m_MenuRect = UIManager->GetNamedUI("MainMenu");

	UIManager->RegisterMethodEvent("Menu_OnePlayerBtn", LWEUI::Event_Released, &State_Menu::SinglePlayBtnPressed, this, A);
	UIManager->RegisterMethodEvent("Menu_TwoPlayerBtn", LWEUI::Event_Released, &State_Menu::DoublePlayBtnPressed, this, A);
	UIManager->RegisterMethodEvent("Menu_InstructionsBtn", LWEUI::Event_Released, &State_Menu::InstructionBtnPressed, this, A);
	UIManager->RegisterMethodEvent("Menu_QuitBtn", LWEUI::Event_Released, &State_Menu::QuitBtnPressed, this, A);
}