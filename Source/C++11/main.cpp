#include <LWCore/LWAllocators/LWAllocator_Default.h>
#include <LWCore/LWTimer.h>
#include "App.h"
#include <LWPlatform/LWThread.h>
#include <LWPlatform/LWApplication.h>
#include <chrono>

LWThread *AThread;
LWThread *UThread;

void AudioThread(LWThread *T) {

	LWRunLoop([](void *UserData)->bool {
		App *A = (App*)UserData;
		A->AudioThread(LWTimer::GetCurrent());
		return (A->GetFlag()&App::Terminate) == 0;
	}, LWTimer::GetResolution() / 60, T->GetUserData());
	return;
}

void UpdateThread(LWThread *T) {

	LWRunLoop([](void *UserData)->bool {
		App *A = (App*)UserData;
		A->UpdateThread(LWTimer::GetCurrent());
		return (A->GetFlag()&App::Terminate) == 0;
	}, LWTimer::GetResolution() / 60, T->GetUserData());
	return;
}

int LWMain(int argc, char **argv) {
	LWAllocator_Default *Allocator = new LWAllocator_Default();
	App *A = Allocator->Allocate<App>(*Allocator);
	AThread = Allocator->Allocate<LWThread>(AudioThread, A);
	UThread = Allocator->Allocate<LWThread>(UpdateThread, A);
	LWRunLoop([](void *UserData) ->bool{
		App *A = (App*)UserData;
		uint64_t Current = LWTimer::GetCurrent();
		A->InputThread(Current);
		A->RenderThread(Current);

		bool Running = (A->GetFlag()&App::Terminate) == 0;
		if (Running) return Running;

		LWAllocator_Default *DefAlloc = (LWAllocator_Default*)&A->GetAllocator();
		AThread->Join();
		UThread->Join();
		LWAllocator::Destroy(AThread);
		LWAllocator::Destroy(UThread);
		LWAllocator::Destroy(A);
		delete DefAlloc;
		return Running;
	}, LWTimer::GetResolution()/60, A);
	return 0;
}