#ifndef APP_H
#define APP_H
#include <LWPlatform/LWTypes.h>
#include <LWAudio/LWTypes.h>
#include <LWVideo/LWTypes.h>
#include <LWCore/LWTypes.h>
#include <LWETypes.h>
#include "Types.h"
#include "State.h"

class App {
public:
	enum {
		Terminate = 0x1,
		FrameChanged = 0x2,
		UpdateRate = 60
	};

	void NetworkThread(uint64_t lCurrentTime);

	void AudioThread(uint64_t lCurrentTime);

	void InputThread(uint64_t lCurrentTime);

	void RenderThread(uint64_t lCurrentTime);

	void UpdateThread(uint64_t lCurrentTime);

	App &SetTerminate(void);

	App &SetActiveState(uint32_t State);

	LWEAssetManager *GetAssetManager(void);

	LWEUIManager *GetUIManager(void);

	bool LoadGameData(void);

	uint32_t GetFlag(void);

	App(LWAllocator &Allocator);

	~App();

private:
	State *m_States[State::Count];
	LWWindow *m_Window;
	LWVideoDriver *m_VideoDriver;
	LWAudioDriver *m_AudioDriver;
	Renderer *m_Renderer;
	LWEUIManager *m_UIManager;
	LWEAssetManager *m_AssetManager;
	LWAllocator &m_Allocator;
	uint64_t m_LastUpdateTick;
	uint32_t m_Flag;
	uint32_t m_ActiveState;

};

#endif