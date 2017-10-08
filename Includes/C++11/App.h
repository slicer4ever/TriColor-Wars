#ifndef APP_H
#define APP_H
#include <LWPlatform/LWTypes.h>
#include <LWAudio/LWTypes.h>
#include <LWVideo/LWTypes.h>
#include <LWCore/LWTypes.h>
#include <LWETypes.h>
#include "Types.h"
#include "State.h"
#include "Sprite.h"

struct Settings {
	enum {
		OnePlayer = 0,
		TwoPlayer
	};
	uint32_t m_GameMode;
};

struct AudioQueue {
	enum {
		MaxQueueSize = 32
	};
	LWAudioStream *m_Stream[MaxQueueSize];
	float m_StreamVolume[MaxQueueSize];
	uint32_t m_LoopCount[MaxQueueSize];

	uint32_t m_ReadPos;
	uint32_t m_WritePos;
};

class App {
public:
	enum {
		Terminate = 0x1,
		FrameChanged = 0x2,
		UpdateRate = 60
	};

	void AudioThread(uint64_t lCurrentTime);

	void InputThread(uint64_t lCurrentTime);

	void RenderThread(uint64_t lCurrentTime);

	void UpdateThread(uint64_t lCurrentTime);

	App &SetTerminate(void);

	App &SetActiveState(uint32_t State);

	App &DispatchAudio(const char *Name, float Volume, uint32_t LoopCnt=0);

	LWEAssetManager *GetAssetManager(void);

	LWEUIManager *GetUIManager(void);

	SpriteManager *GetSpriteManager(void);

	LWWindow *GetWindow(void);

	bool LoadGameData(void);

	bool LoadUIData(void);

	uint32_t GetFlag(void);

	Settings &GetSettings(void);

	App(LWAllocator &Allocator);

	~App();

private:
	Settings m_Settings;
	AudioQueue m_AudioQueue;
	State *m_States[State::Count];
	LWWindow *m_Window;
	LWVideoDriver *m_VideoDriver;
	LWAudioDriver *m_AudioDriver;
	Renderer *m_Renderer;
	LWEUIManager *m_UIManager;
	LWEAssetManager *m_AssetManager;
	SpriteManager *m_SpriteManager;
	LWAllocator &m_Allocator;
	LWVector2i m_PrevPosition;
	LWVector2i m_PrevSize;
	uint64_t m_LastUpdateTick;
	uint32_t m_Flag;
	uint32_t m_ActiveState;

};

#endif