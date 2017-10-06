#ifndef STATE_GAME_H
#define STATE_GAME_H
#include "State.h"
#include <LWETypes.h>

class State_Game : public State {
public:
	State &Update(bool Tick, App *A, uint64_t lCurrentTime);

	State &DrawFrame(App *A, Frame *F, Renderer *R);

	State &ProcessInput(App *A, LWWindow *Window);

	State &Activated(App *A);

	State &Deactivated(App *A);

	State_Game(App *A, LWEUIManager *UIMan);
private:
};

#endif