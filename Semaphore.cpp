#undef UNICODE
#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include "Philosopher.h"
#include <commctrl.h>

#pragma warning(disable : 4996)

TPhilosopher* PhilosopherBuf[5];
HANDLE philosopherThreads[5];
HANDLE forkSemaphoreBuf[5];
HANDLE kitchenSemaphore;
HINSTANCE MainHInstance;

BOOL CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	MainHInstance = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, DLGPROC(MainWndProc));
	return 0;
}

DWORD WINAPI PhilosopherLifeThread(void* id) {
	int philosopherID = (int)id;
	while (PhilosopherBuf[philosopherID]->GetState() != psDie) {

		PhilosopherBuf[philosopherID]->Think();

		WaitForSingleObject(kitchenSemaphore, INFINITE);
		WaitForSingleObject(forkSemaphoreBuf[philosopherID], INFINITE);
		WaitForSingleObject(forkSemaphoreBuf[(philosopherID + 1) % 5], INFINITE);

		PhilosopherBuf[philosopherID]->Eat();

		ReleaseSemaphore(forkSemaphoreBuf[(philosopherID + 1) % 5], 1, NULL);
		ReleaseSemaphore(forkSemaphoreBuf[philosopherID], 1, NULL);
		ReleaseSemaphore(kitchenSemaphore, 1, NULL);
	}
	return 0;
}

void initializeInterface(HWND hWnd) {
	EnableWindow(GetDlgItem(hWnd, IDC_START), TRUE);
	EnableWindow(GetDlgItem(hWnd, IDC_STOP), FALSE);
	EnableWindow(GetDlgItem(hWnd, IDC_VORACITY), FALSE);

	for (int i = 0; i < 5; i++) {
		forkSemaphoreBuf[i] = CreateSemaphore(NULL, 1, 1, NULL);
	}

	kitchenSemaphore = CreateSemaphore(NULL, 4, 4, NULL);

	for (int i = 0; i < 5; i++) {
		PhilosopherBuf[i] = NULL;
	}
}

void startButton(HWND hWnd) {
	EnableWindow(GetDlgItem(hWnd, IDC_START), FALSE);
	EnableWindow(GetDlgItem(hWnd, IDC_STOP), TRUE);
	EnableWindow(GetDlgItem(hWnd, IDC_VORACITY), TRUE);
	for (int i = 0; i < 5; i++) {
		PhilosopherBuf[i] = new TPhilosopher();
		philosopherThreads[i] = CreateThread(NULL, 0, PhilosopherLifeThread, (LPVOID)i, 0, NULL);

	}
}

void stopButton(HWND hWnd) {
	EnableWindow(GetDlgItem(hWnd, IDC_START), TRUE);
	EnableWindow(GetDlgItem(hWnd, IDC_STOP), FALSE);
	EnableWindow(GetDlgItem(hWnd, IDC_VORACITY), FALSE);
	for (int i = 0; i < 5; i++) {
		if (PhilosopherBuf[i] != NULL) {
			PhilosopherBuf[i]->Kill();
			WaitForSingleObject(philosopherThreads[i], INFINITE);
			CloseHandle(philosopherThreads[i]);
			delete PhilosopherBuf[i];
			PhilosopherBuf[i] = NULL;
			philosopherThreads[i] = NULL;

		}
	}
	for (int i = 0; i < 5; i++) {
		SetDlgItemText(hWnd, IDC_STATE1 + i, "");
		SetDlgItemText(hWnd, IDC_FORCE1 + i, "");
		SetDlgItemText(hWnd, IDC_WISDOM1 + i, "");
	}
}

void paintText(HWND hWnd, HDC hdcStatic, HBRUSH hBrush) {
	int controlID = GetDlgCtrlID(hWnd);

	for (int i = 0; i < 5; i++) {

		if (controlID == IDC_STATE1 + i && PhilosopherBuf[i] != NULL) {
			switch (PhilosopherBuf[i]->GetState())
			{
			case psThink:
				SetTextColor(hdcStatic, RGB(0, 0, 255));
				SetBkColor(hdcStatic, RGB(224, 224, 224));
				break;
			case psEat:
				SetTextColor(hdcStatic, RGB(0, 255, 0));
				SetBkColor(hdcStatic, RGB(224, 224, 224));
				break;
			case psDie:
				SetTextColor(hdcStatic, RGB(255, 0, 0));
				SetBkColor(hdcStatic, RGB(224, 224, 224));
				break;
			case psIdle:
				SetTextColor(hdcStatic, RGB(0, 0, 0));
				SetBkColor(hdcStatic, RGB(224, 224, 224));
				break;
			default:
				SetTextColor(hdcStatic, RGB(0, 0, 0));
				break;
			}
		}
	}
	SetBkColor(hdcStatic, RGB(240, 240, 240));
}

void updatePhilosopherState(HWND hWnd, int philosofer) {
	int stateID = IDC_STATE1 + philosofer;
	int forceID = IDC_FORCE1 + philosofer;
	int wisdomID = IDC_WISDOM1 + philosofer;

	if (PhilosopherBuf[philosofer] != NULL) {
		int state = PhilosopherBuf[philosofer]->GetState();
		int force = PhilosopherBuf[philosofer]->GetForce();
		int wisdom = PhilosopherBuf[philosofer]->GetWisdom();

		char stateText[10] = { 0 };
		char forceBuf[10];
		char wisdomBuf[10];

		switch (state) {
		case 0:
			sprintf(stateText, "%s", "Idle");
			break;
		case 1:
			sprintf(stateText, "%s", "Think");
			break;
		case 2:
			sprintf(stateText, "%s", "Eat");
			break;
		case 3:
			sprintf(stateText, "%s", "Died");
			break;
		}

		sprintf(forceBuf, "%d", force);
		sprintf(wisdomBuf, "%d", wisdom);

		SetDlgItemText(hWnd, stateID, stateText);
		SetDlgItemText(hWnd, forceID, forceBuf);
		SetDlgItemText(hWnd, wisdomID, wisdomBuf);
	}
}

BOOL CALLBACK VoracityWndProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
	case WM_INITDIALOG:
	{
		int vorocity = PhilosopherBuf[0]->GetVoracity();
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER), TBM_SETPOS, TRUE, vorocity);
	}
	return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
		{
			int newVoracity = SendMessage(GetDlgItem(hDlg, IDC_SLIDER), TBM_GETPOS, 0, 0);

			for (int i = 0; i < 5; i++) {
				PhilosopherBuf[i]->SetVoracity(newVoracity);
			}
		}
		EndDialog(hDlg, IDOK);
		return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return TRUE;
		case IDC_EXIT:
			DestroyWindow(hDlg);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}




BOOL CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	switch (Msg) {
	case WM_INITDIALOG:
		initializeInterface(hWnd);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_START:
			startButton(hWnd);
			return TRUE;
		case IDC_STOP:
			stopButton(hWnd);
			return TRUE;
		case IDC_VORACITY:
			DialogBox(MainHInstance, MAKEINTRESOURCE(IDD_VORACITYDIALOG), hWnd, DLGPROC(VoracityWndProc));
			return TRUE;
		case IDC_EXIT:
			DestroyWindow(hWnd);
			return TRUE;
		}
		return FALSE;
	case WM_CTLCOLORSTATIC:
	{
		HBRUSH hBrush = CreateSolidBrush(RGB(242, 242, 242));
		HDC hdcStatic = (HDC)wParam;
		HWND hWnd = (HWND)lParam;
		SetBkColor(hdcStatic, RGB(240, 240, 240));
		paintText(hWnd, hdcStatic, hBrush);
		return (INT_PTR)hBrush;
	}
	case WM_PHILOSOPHER:
	{
		int philosofer = wParam;
		updatePhilosopherState(hWnd, philosofer);
	}
	return TRUE;
	case WM_DESTROY:
		for (int i = 0; i < 5; i++) {
			if (PhilosopherBuf[i] != NULL) {
				PhilosopherBuf[i]->Kill();
				WaitForSingleObject(philosopherThreads[i], INFINITE);
				delete PhilosopherBuf[i];
			}
		}
		for (int i = 0; i < 5; i++) {
			CloseHandle(forkSemaphoreBuf[i]);
		}
		CloseHandle(kitchenSemaphore);
		PostQuitMessage(0);
		return TRUE;
	}
	return FALSE;
}