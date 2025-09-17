#pragma hdrstop

#include <time.h>
#include "Philosopher.h"

#pragma package(smart_init)

#define MAX_PHILOSOPHERS 5
#define random(x) (rand() % x)

#define INIT_FORCE  50
#define THINK_TIME  ((500 + random(501)))
#define EAT_TIME    ((500 + random(501)))
#define IDLE_FORCE  -2
#define THINK_FORCE -1
#define EAT_FORCE    3

static TPhilosopher * Philosophers[MAX_PHILOSOPHERS] = {NULL, NULL, NULL, NULL, NULL};

DWORD WINAPI _PhilosopherThread(void* P);

static void Stop(const char * Msg) {
  for (int i = 0; i < MAX_PHILOSOPHERS; i++) if (Philosophers[i]) Philosophers[i]->Kill();
  MessageBox(NULL, Msg, "Problem is not solved", MB_OK | MB_ICONERROR);
  TerminateProcess(GetCurrentProcess(), 1);
}

TPhilosopher::TPhilosopher()
{
  srand((unsigned)time(NULL));
  bool found = false;
  for (int i = 0; i < MAX_PHILOSOPHERS; i++) if (!Philosophers[i]) {
    found = true;
    Philosophers[i] = this;
    break;
  }
  if (!found) Stop("Population of the philosophers is overflowed");
  FForce  = INIT_FORCE;
  FWisdom = 0;
  FState  = psIdle;
  InitializeCriticalSection(&CPhilosopher);
  FStop = CreateEvent(NULL, TRUE, FALSE, NULL);
  FVoracity = 50;
  FLife = CreateThread(NULL, 0, &_PhilosopherThread, (void*)this, 0, NULL);
}

TPhilosopher::TPhilosopher(TPhilosopher& P)
{
  throw std::string("Can't make a copy of philosopher");
}

TPhilosopher::~TPhilosopher()
{
	Kill();

	Sleep(100);

	WaitForSingleObject(FLife, INFINITE);
	CloseHandle(FLife);

	DeleteCriticalSection(&CPhilosopher);

	for (int i = 0; i < MAX_PHILOSOPHERS; i++) if (Philosophers[i] == this) {
		Philosophers[i] = NULL;
		break;
	}

	CloseHandle(FStop);
}

DWORD WINAPI _PhilosopherThread(void* P)
{
	HWND wnd = FindWindow("#32770", NULL);
	DWORD id = -1;
	for (int i = 0; i < MAX_PHILOSOPHERS; i++) if (Philosophers[i] == P) { id = i; break; }
	TPhilosopher* Philosopher = (TPhilosopher*)P;
	bool die = false;
	while (!die && WaitForSingleObject(Philosopher->FStop, 100) == WAIT_TIMEOUT) {
		EnterCriticalSection(&Philosopher->CPhilosopher);
		switch (Philosopher->FState) {
		case psIdle: Philosopher->FForce += IDLE_FORCE; break;
		case psThink: Philosopher->FForce += THINK_FORCE; Philosopher->FWisdom++; break;
		case psEat: Philosopher->FForce += EAT_FORCE; break;
		}
		die = Philosopher->FState == psDie || Philosopher->FForce < 0;
		LeaveCriticalSection(&Philosopher->CPhilosopher);
		if (wnd) PostMessage(wnd, WM_PHILOSOPHER, id, (LPARAM)P);
	}
	Philosopher->Kill();
	if (wnd) PostMessage(wnd, WM_PHILOSOPHER, id, (LPARAM)P);
	return 0;
}

void TPhilosopher::Think()
{
  if (GetState() != psIdle) return;
  SetState(psThink);
  WaitForSingleObject(FStop, THINK_TIME * (100 - FVoracity) / 100);
  SetState(psIdle);
}

void TPhilosopher::Eat()
{
  if (GetState() != psIdle) return;
  SetState(psEat);
  WaitForSingleObject(FStop, EAT_TIME * FVoracity / 100);
  SetState(psIdle);
}

void TPhilosopher::SetState(int AState)
{
  int state = GetState();
  if (state == psDie || state == AState) return;

  if (AState == psEat) {
    int cnt = 0;
    for (int i = 0; i < MAX_PHILOSOPHERS; i++) if (Philosophers[i] && Philosophers[i]->GetState() == psEat) cnt++;
    if (cnt >= 2) Stop("Two philosophers tried to use one fork at the same time");
  }

  EnterCriticalSection(&CPhilosopher);
  FState = AState;
  LeaveCriticalSection(&CPhilosopher);
}

int TPhilosopher::GetState()
{
  EnterCriticalSection(&CPhilosopher);
  int state = FState;
  LeaveCriticalSection(&CPhilosopher);
  return state;
}

int TPhilosopher::GetForce()
{
  EnterCriticalSection(&CPhilosopher);
  int force = FForce;
  LeaveCriticalSection(&CPhilosopher);
  return force;
}

int TPhilosopher::GetWisdom()
{
  EnterCriticalSection(&CPhilosopher);
  int wisdom = FWisdom;
  LeaveCriticalSection(&CPhilosopher);
  return wisdom;
}

void TPhilosopher::Kill()
{
	SetEvent(FStop);
	SetState(psDie);
}

int  TPhilosopher::GetVoracity()
{
	EnterCriticalSection(&CPhilosopher);
	int voracity = FVoracity;
	LeaveCriticalSection(&CPhilosopher);
	return voracity;
}

bool TPhilosopher::SetVoracity(int AVoracity)
{
	if (AVoracity < 0 || AVoracity > 100) return false;
	EnterCriticalSection(&CPhilosopher);
	FVoracity = AVoracity;
	LeaveCriticalSection(&CPhilosopher);
	return true;
}

