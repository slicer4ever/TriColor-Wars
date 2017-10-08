#ifndef STATE_MENU_H
#define STATE_MENU_H
#include "State.h"
#include <LWETypes.h>

class State_Menu : public State {
public:

	State &Update(bool Tick, App *A, uint64_t lCurrentTime);

	State &DrawFrame(App *A, Frame *F, Renderer *R);

	State &ProcessInput(App *A, LWWindow *Window);

	State &Activated(App *A);

	State &Deactivated(App *A);

	void SinglePlayBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData);

	void DoublePlayBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData);

	void InstructionBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData);

	void InstructBackBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData);

	void QuitBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData);

	State_Menu(App *A, LWEUIManager *UIManager);
private:
	LWEUI *m_MenuRect;
	LWEUI *m_InstructRect;
};


#endif
