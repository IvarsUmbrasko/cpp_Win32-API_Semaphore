#ifndef PhilosopherH
#define PhilosopherH

#undef UNICODE
#include <Windows.h>
#include <string>

#define psIdle  0
#define psThink 1
#define psEat   2
#define psDie   3

#define WM_PHILOSOPHER WM_USER

class TPhilosopher {
  friend DWORD WINAPI _PhilosopherThread(void* P);
  private:
    int FForce, FWisdom, FState;
    CRITICAL_SECTION CPhilosopher;
    HANDLE FLife;
	HANDLE FStop;
	int FVoracity;
    void SetState(int AState);
  public:
    TPhilosopher();
    TPhilosopher(TPhilosopher& P);
    ~TPhilosopher();

    void Think();
    void Eat();
    void Kill();
    int  GetState();
    int  GetForce();
    int  GetWisdom();
	int  GetVoracity();
	bool SetVoracity(int AVoracity);
};


#endif
