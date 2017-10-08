#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWTimer.h>
#include "App.h"
#include <thread>
#include <chrono>

void AudioThread(App *A) {
	while (!(A->GetFlag()&App::Terminate)) {
		A->AudioThread(LWTimer::GetCurrent());
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void UpdateThread(App *A) {
	while (!(A->GetFlag()&App::Terminate)) {
		A->UpdateThread(LWTimer::GetCurrent());
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

int LWMain(int argc, char **argv) {
	LWAllocator_Default Allocator;
	App *A = Allocator.Allocate<App>(Allocator);
	std::thread UThread(UpdateThread, A);
	std::thread AThread(AudioThread, A);
	while (!(A->GetFlag()&App::Terminate)) {
		uint64_t Current = LWTimer::GetCurrent();
		A->InputThread(Current);
		A->RenderThread(Current);
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
	UThread.join();
	AThread.join();

	LWAllocator::Destroy(A);
	return 0;
}