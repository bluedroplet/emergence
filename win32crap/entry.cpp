#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <objbase.h>


#include "../../common/types.h"
#include "main.h"


HINSTANCE hInst;
HWND hWnd;



void terminate_process()
{
	if(hWnd != NULL)
		DestroyWindow(hWnd);

	CoUninitialize();

	ExitProcess(0);
}


void process_wnd_msgs()
{
	int GotMsg;
	MSG  message;

	GotMsg = PeekMessage(&message, NULL, 0, 0, PM_REMOVE);

	while(GotMsg)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		GotMsg = PeekMessage(&message, NULL, 0, 0, PM_REMOVE);
    }
}


LRESULT WINAPI WinProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, Message, wParam, lParam);
}


void MakeWindow()
{
	WNDCLASSEX	wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hInstance = hInst;
	wcex.lpszClassName = "NetFighter";
	wcex.lpfnWndProc = WinProc;
	wcex.style = 0;
	wcex.hIcon = NULL;
	wcex.hIconSm = NULL;
	wcex.hCursor = NULL;
	wcex.lpszMenuName = NULL;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);

	if(!RegisterClassEx(&wcex))
		terminate_process();

#ifdef _DEBUG

	hWnd = CreateWindowEx(WS_EX_TOPMOST, "NetFighter", "NetFighter", WS_VISIBLE | WS_POPUP,
		0, 0, 0, 0, NULL, NULL, hInst, NULL);
#else
	
	hWnd = CreateWindowEx(WS_EX_TOPMOST, "NetFighter", "NetFighter", WS_VISIBLE | WS_POPUP,
		0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), NULL, NULL, hInst, NULL);

#endif


	if(hWnd == NULL)
		terminate_process();

	SetFocus(hWnd);
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	process_wnd_msgs();
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR CmdLine, int nCmdShow)
{
	::hInst = hInst;

	CoInitialize(NULL);

	MakeWindow();

	main(); // never returns

	return 1;
}
