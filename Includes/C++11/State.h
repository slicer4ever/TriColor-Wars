#ifndef STATE_H
#define STATE_H
#include "Types.h"
#include <LWCore/LWTypes.h>
#include <LWPlatform/LWTypes.h>
class State {
public:
	enum {
		Menu = 0,
		Game,
		Count
	};

	virtual State &Update(bool Tick, App *A, uint64_t lCurrentTime) = 0;

	virtual State &DrawFrame(App *A, Frame *F, Renderer *R) = 0;

	virtual State &ProcessInput(App *A, LWWindow *Window) = 0;

	virtual State &Activated(App *A) = 0;

	virtual State &Deactivated(App *A) = 0;
};

#endif
