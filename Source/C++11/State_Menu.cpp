#include "State_Menu.h"
#include <LWEUIManager.h>
#include <LWPlatform/LWWindow.h>
#include <LWPlatform/LWInputDevice.h>
#include "App.h"

State &State_Menu::Update(bool Tick, App *A, uint64_t lCurrentTime) {
	return *this;
}

State &State_Menu::DrawFrame(App *A, Frame *F, Renderer *R) {
	return *this;
}

State &State_Menu::ProcessInput(App *A, LWWindow *Window) {
	LWKeyboard *Keyboard = Window->GetKeyboardDevice();

	if (Keyboard) {
		if (Keyboard->ButtonPressed(LWKey::Esc)) {
			if (m_InstructRect->GetVisible()) InstructBackBtnPressed(nullptr, 0, A);
			else A->SetTerminate();
		}
	}
	return *this; 
}

State &State_Menu::Activated(App *A) {
	m_MenuRect->SetVisible(true);
	m_InstructRect->SetVisible(false);
	return *this;
}

State &State_Menu::Deactivated(App *A) {
	m_MenuRect->SetVisible(false);
	m_InstructRect->SetVisible(false);
	return *this;
}

void State_Menu::SinglePlayBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	Settings &Set = A->GetSettings();
	Set.m_GameMode = Settings::OnePlayer;
	A->SetActiveState(State::Game);
	return;
}

void State_Menu::DoublePlayBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	Settings &Set = A->GetSettings();
	Set.m_GameMode = Settings::TwoPlayer;
	A->SetActiveState(State::Game);
	return;
}

void State_Menu::InstructionBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	m_MenuRect->SetVisible(false);
	m_InstructRect->SetVisible(true);
	return;
}

void State_Menu::InstructBackBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	m_MenuRect->SetVisible(true);
	m_InstructRect->SetVisible(false);
	return;
}

void State_Menu::QuitBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	A->SetTerminate();
	return;
}

State_Menu::State_Menu(App *A, LWEUIManager *UIManager) : State() {
	m_MenuRect = UIManager->GetNamedUI("MainMenu");
	m_InstructRect = UIManager->GetNamedUI("InsMenu");

	UIManager->RegisterMethodEvent("Instruct_BackBtn", LWEUI::Event_Released, &State_Menu::InstructBackBtnPressed, this, A);
	UIManager->RegisterMethodEvent("Menu_OnePlayerBtn", LWEUI::Event_Released, &State_Menu::SinglePlayBtnPressed, this, A);
	UIManager->RegisterMethodEvent("Menu_TwoPlayerBtn", LWEUI::Event_Released, &State_Menu::DoublePlayBtnPressed, this, A);
	UIManager->RegisterMethodEvent("Menu_InstructionsBtn", LWEUI::Event_Released, &State_Menu::InstructionBtnPressed, this, A);
	UIManager->RegisterMethodEvent("Menu_QuitBtn", LWEUI::Event_Released, &State_Menu::QuitBtnPressed, this, A);
}