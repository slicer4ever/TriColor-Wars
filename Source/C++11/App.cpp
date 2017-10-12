#include "App.h"
#include <LWCore/LWTimer.h>
#include <LWCore/LWVector.h>
#include <LWCore/LWMath.h>
#include <LWCore/LWAllocator.h>
#include <LWPlatform/LWWindow.h>
#include <LWPlatform/LWVideoMode.h>
#include <LWVideo/LWVideoDriver.h>
#include <LWAudio/LWAudioDriver.h>
#include <LWPlatform/LWFileStream.h>
#include <LWEUIManager.h>
#include <LWEAsset.h>
#include "Renderer.h"
#include "State_Menu.h"
#include "State_Game.h"

void App::AudioThread(uint64_t lCurrentTime) {
	while (m_AudioQueue.m_ReadPos != m_AudioQueue.m_WritePos) {
		uint32_t i = m_AudioQueue.m_ReadPos%AudioQueue::MaxQueueSize;
		m_AudioDriver->CreateSound(m_AudioQueue.m_Stream[i], LWSound::RequestPlay, nullptr, m_AudioQueue.m_LoopCount[i], m_AudioQueue.m_StreamVolume[i]);
		m_AudioQueue.m_ReadPos++;
	}

	if (!m_AudioDriver->Update(lCurrentTime, m_Window)) {
		std::cout << "Audio driver failed." << std::endl;
		m_Flag |= Terminate;
	}
	return;
}

void App::InputThread(uint64_t lCurrentTime) {
	m_Window->Update(lCurrentTime);
	LWKeyboard *Keyboard = m_Window->GetKeyboardDevice();
	if (m_Window->GetFlag()&LWWindow::Terminate) m_Flag |= Terminate;
	if (Keyboard->ButtonPressed(LWKey::F11)) {
		bool isFullscreen = (m_Window->GetFlag()&LWWindow::Borderless) != 0;
		m_Window->SetBorderless(!isFullscreen, true);
		
		LWVideoMode ActiveMode = LWVideoMode::GetActiveMode();
		if (isFullscreen) {
			m_Window->SetPosition(m_PrevPosition);
			m_Window->SetSize(m_PrevSize);
		} else {
			m_PrevPosition = m_Window->GetPosition();
			m_PrevSize = m_Window->GetSize();
			m_Window->SetPosition(LWVector2i(0));
			m_Window->SetSize(ActiveMode.GetSize());
		}
	}

	m_UIManager->Update(lCurrentTime);
	m_States[m_ActiveState]->ProcessInput(this, m_Window);
	return;
}

void App::RenderThread(uint64_t lCurrentTime) {
	m_Renderer->Render(m_Window);
	return;
}

void App::UpdateThread(uint64_t lCurrentTime) {
	const uint64_t UpdateFreq = LWTimer::GetResolution() / UpdateRate;
	bool Tick = lCurrentTime - m_LastUpdateTick >= UpdateFreq;
	if (Tick) {
		m_LastUpdateTick += UpdateFreq;
		m_Flag |= FrameChanged;
	}
	m_States[m_ActiveState]->Update(Tick, this, lCurrentTime);
	if (m_Flag&FrameChanged) {
		Frame *F = m_Renderer->BeginFrame();
		if (!F) return;
		m_States[m_ActiveState]->DrawFrame(this, F, m_Renderer);
		m_UIManager->Draw(&F->m_UIFrame, lCurrentTime);
		m_Renderer->EndFrame();
		m_Flag &= ~FrameChanged;
	}

	return;
}

LWEAssetManager *App::GetAssetManager(void) {
	return m_AssetManager;
}

LWEUIManager *App::GetUIManager(void) {
	return m_UIManager;
}

SpriteManager *App::GetSpriteManager(void) {
	return m_SpriteManager;
}

LWWindow *App::GetWindow(void) {
	return m_Window;
}

Settings &App::GetSettings(void) {
	return m_Settings;
}

bool App::LoadGameData(void) {
	char FileBuffer[1024 * 32];
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, "App:GameData.xml", LWFileStream::ReadMode | LWFileStream::BinaryMode, m_Allocator)) {
		std::cout << "Error opening: 'App:FameData.xml'" << std::endl;
		return false;
	}
	Stream.ReadText(FileBuffer, sizeof(FileBuffer));
	LWEXML *X = m_Allocator.Allocate<LWEXML>();
	if (!LWEXML::ParseBuffer(*X, m_Allocator, FileBuffer, true)) {
		std::cout << "Error parsing xml: 'App:GameData.xml'" << std::endl;
		return false;
	}
	X->PushParser("GameData", State_Game::XMLParse, m_States[State::Game]);
	X->Process();
	LWAllocator::Destroy(X);
	return true;
}

bool App::LoadUIData(void) {
	char FileBuffer[1024 * 32];
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, "App:UIData.xml", LWFileStream::ReadMode | LWFileStream::BinaryMode, m_Allocator)) {
		std::cout << "Error opening: 'App:UIData.xml'" << std::endl;
		return false;
	}
	Stream.ReadText(FileBuffer, sizeof(FileBuffer));
	LWEXML *X = m_Allocator.Allocate<LWEXML>();
	if (!LWEXML::ParseBuffer(*X, m_Allocator, FileBuffer, true)) {
		std::cout << "Error parsing xml: 'App:UIData.xml'" << std::endl;
		return false;
	}
	m_AssetManager = m_Allocator.Allocate<LWEAssetManager>(m_VideoDriver, nullptr, m_Allocator);
	m_UIManager = m_Allocator.Allocate<LWEUIManager>(m_Window, &m_Allocator, nullptr, m_AssetManager);
	m_SpriteManager = m_Allocator.Allocate<SpriteManager>(m_AssetManager);
	X->PushParser("AssetManager", LWEAssetManager::XMLParser, m_AssetManager);
	X->PushParser("UIManager", LWEUIManager::XMLParser, m_UIManager);
	X->PushParser("SpriteManager", SpriteManager::XMLParser, m_SpriteManager);
	X->Process();
	LWAllocator::Destroy(X);
	return true;
}

App &App::SetTerminate(void) {
	m_Flag |= Terminate;
	return *this;
}

App &App::SetActiveState(uint32_t State) {
	m_States[m_ActiveState]->Deactivated(this);
	m_ActiveState = State;
	m_States[m_ActiveState]->Activated(this);
	return *this;
}

App &App::DispatchAudio(const char *Name, float Volume, uint32_t LoopCnt) {
	if (m_AudioQueue.m_WritePos - m_AudioQueue.m_ReadPos >= AudioQueue::MaxQueueSize) return *this;
	LWAudioStream *Stream = m_AssetManager->GetAsset<LWAudioStream>(Name);
	if (!Stream) return *this;
	uint32_t i = m_AudioQueue.m_WritePos%AudioQueue::MaxQueueSize;
	m_AudioQueue.m_Stream[i] = Stream;
	m_AudioQueue.m_StreamVolume[i] = Volume;
	m_AudioQueue.m_LoopCount[i] = LoopCnt;

	m_AudioQueue.m_WritePos++;
	return *this;
}

uint32_t App::GetFlag(void) {
	return m_Flag;
}

App::App(LWAllocator &Allocator) : m_Flag(0), m_Allocator(Allocator), m_VideoDriver(nullptr), m_AudioDriver(nullptr), m_Renderer(nullptr), m_UIManager(nullptr), m_AssetManager(nullptr), m_SpriteManager(nullptr), m_LastUpdateTick(LWTimer::GetCurrent()) {
	char DriverNames[][32] = { "OpenGL3.2", "OpenGL2.1", "DirectX11.1", "OpenGLES2", "DirectX12", "DirectX9", "OpenGL4.5", "OpenGLES3", "Metal" };
	char ArchNames[][32] = LWARCH_NAMES;
	char PlatformNames[][32] = LWPLATFORM_NAMES;
	LWVideoMode CurrentMode = LWVideoMode::GetActiveMode();
	memset(m_States, 0, sizeof(m_States));
	m_AudioQueue.m_ReadPos = m_AudioQueue.m_WritePos = 0;

	LWVector2i WndSize = LWVector2i(1280, 720);
	LWVector2i WndPos = CurrentMode.GetSize() / 2 - WndSize / 2;
	m_Window = Allocator.Allocate<LWWindow>("Tri-Color Wars", "TriColor", Allocator, LWWindow::WindowedMode | LWWindow::MouseDevice | LWWindow::KeyboardDevice, WndPos, WndSize);
	m_VideoDriver = LWVideoDriver::MakeVideoDriver(m_Window, LWVideoDriver::Unspecefied);
	if (!m_VideoDriver) {
		std::cout << "Error creating video driver." << std::endl;
		m_Flag |= Terminate;
		return;
	}
	m_AudioDriver = m_Allocator.Allocate<LWAudioDriver>(nullptr, [](LWSound *S, LWAudioDriver *AD) { if (S->GetPlayCount() == S->GetFinishedCount()) AD->ReleaseSound(S); }, nullptr, nullptr);
	if (m_AudioDriver->GetFlag()&LWAudioDriver::Error) {
		std::cout << "Error making audio driver." << std::endl;
		m_Flag |= Terminate;
		return;
	}
	uint32_t DriverID = (uint32_t)(log(m_VideoDriver->GetDriverType()) / log(2));
	m_Window->SetTitlef("Tri-Color Wars | %s | %s | %s", DriverNames[DriverID], ArchNames[LWARCH_ID], PlatformNames[LWPLATFORM_ID]);
	if (!LoadUIData()) {
		m_Flag |= Terminate;
		return;
	}
	m_Renderer = m_Allocator.Allocate<Renderer>(m_Window, m_VideoDriver, m_Allocator, m_AssetManager);
	m_States[State::Menu] = (State *)m_Allocator.Allocate<State_Menu>(this, m_UIManager);
	m_States[State::Game] = (State*)m_Allocator.Allocate<State_Game>(this, m_UIManager, m_SpriteManager);
	m_ActiveState = State::Menu;
	m_States[m_ActiveState]->Activated(this);
	if (!LoadGameData()){
		m_Flag |= Terminate;
		return;
	}
	DispatchAudio("BackgroundAudio", 0.25f, 0xFFFFFF);
}

App::~App() {
	LWAllocator::Destroy((State_Menu*)m_States[State::Menu]);
	LWAllocator::Destroy((State_Game*)m_States[State::Game]);
	LWAllocator::Destroy(m_AudioDriver);
	LWAllocator::Destroy(m_SpriteManager);
	LWAllocator::Destroy(m_UIManager);
	LWAllocator::Destroy(m_AssetManager);
	LWAllocator::Destroy(m_Renderer);
	LWVideoDriver::DestroyVideoDriver(m_VideoDriver);
	LWAllocator::Destroy(m_Window);
}