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

void App::NetworkThread(uint64_t lCurrentTime) {
	return;
}

void App::AudioThread(uint64_t lCurrentTime) {
	return;
}

void App::InputThread(uint64_t lCurrentTime) {
	m_Window->Update(lCurrentTime);
	LWKeyboard *Keyboard = m_Window->GetKeyboardDevice();
	if (m_Window->GetFlag()&LWWindow::Terminate) m_Flag |= Terminate;
	if (Keyboard->ButtonPressed(LWKey::Esc)) m_Flag |= Terminate;
	m_UIManager->Update(lCurrentTime);
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

bool App::LoadGameData(void) {
	char FileBuffer[1024 * 32];
	LWFileStream Stream;
	if (!LWFileStream::OpenStream(Stream, "App:GameData.xml", LWFileStream::ReadMode | LWFileStream::BinaryMode, m_Allocator)) {
		std::cout << "Error opening: 'App:GameData.xml'" << std::endl;
		return false;
	}
	Stream.Read(FileBuffer, sizeof(FileBuffer));
	LWEXML *X = m_Allocator.Allocate<LWEXML>();
	if (!LWEXML::ParseBuffer(*X, m_Allocator, FileBuffer, true)) {
		std::cout << "Error parsing xml: 'App:GameData.xml'" << std::endl;
		return false;
	}
	m_AssetManager = m_Allocator.Allocate<LWEAssetManager>(m_VideoDriver, nullptr, m_Allocator);
	m_UIManager = m_Allocator.Allocate<LWEUIManager>(m_Window, &m_Allocator, nullptr, m_AssetManager);
	X->PushParser("AssetManager", LWEAssetManager::XMLParser, m_AssetManager);
	X->PushParser("UIManager", LWEUIManager::XMLParser, m_UIManager);
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

uint32_t App::GetFlag(void) {
	return m_Flag;
}

App::App(LWAllocator &Allocator) : m_Flag(0), m_Allocator(Allocator), m_VideoDriver(nullptr), m_AudioDriver(nullptr), m_Renderer(nullptr), m_UIManager(nullptr), m_AssetManager(nullptr), m_LastUpdateTick(LWTimer::GetCurrent()) {
	char DriverNames[][32] = { "OpenGL3.2", "OpenGL2.1", "DirectX11.1", "OpenGLES2", "DirectX12", "DirectX9", "OpenGL4.5", "OpenGLES3", "Metal" };
	char ArchNames[][32] = LWARCH_NAMES;
	char PlatformNames[][32] = LWPLATFORM_NAMES;
	LWVideoMode CurrentMode = LWVideoMode::GetActiveMode();

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
	if (!LoadGameData()) {
		m_Flag |= Terminate;
		return;
	}
	m_Renderer = m_Allocator.Allocate<Renderer>(m_Window, m_VideoDriver, m_Allocator, m_AssetManager);
	m_States[State::Menu] = (State *)m_Allocator.Allocate<State_Menu>(this, m_UIManager);
	m_States[State::Game] = (State*)m_Allocator.Allocate<State_Game>(this, m_UIManager);
	m_ActiveState = State::Menu;
	m_States[m_ActiveState]->Activated(this);
}

App::~App() {
	LWAllocator::Destroy((State_Menu*)m_States[State::Menu]);
	LWAllocator::Destroy((State_Game*)m_States[State::Game]);
	LWAllocator::Destroy(m_UIManager);
	LWAllocator::Destroy(m_AssetManager);
	LWAllocator::Destroy(m_Renderer);
	LWVideoDriver::DestroyVideoDriver(m_VideoDriver);
	LWAllocator::Destroy(m_AudioDriver);
	LWAllocator::Destroy(m_Window);
}