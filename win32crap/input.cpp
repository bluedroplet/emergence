#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#define DIRECTINPUT_VERSION 0x0800
#include <windows.h>
#include <dinput.h>

#include "../../common/types.h"
#include "main.h"
#include "winmain.h"
#include "console.h"
#include "control.h"

// DirectInput Structures

LPDIRECTINPUT8			diObject = NULL;
LPDIRECTINPUTDEVICE8	Keyboard = NULL;
LPDIRECTINPUTDEVICE8	Mouse = NULL;

HANDLE hKeyboardEvent = NULL;


int shift = 0x20;

char get_ascii(dword key)
{
	switch(key)
	{
	case DIK_SPACE:
		return ' ';

	case DIK_0:
	case DIK_NUMPAD0:
		return '0';
	
	case DIK_1:
	case DIK_NUMPAD1:
		return '1';
	
	case DIK_2:
	case DIK_NUMPAD2:
		return '2';
	
	case DIK_3:
	case DIK_NUMPAD3:
		return '3';
	
	case DIK_4:
	case DIK_NUMPAD4:
		return '4';
	
	case DIK_5:
	case DIK_NUMPAD5:
		return '5';
	
	case DIK_6:
	case DIK_NUMPAD6:
		return '6';
	
	case DIK_7:
	case DIK_NUMPAD7:
		return '7';
	
	case DIK_8:
	case DIK_NUMPAD8:
		return '8';
	
	case DIK_9:
	case DIK_NUMPAD9:
		return '9';

	case DIK_PERIOD:
		return '.';

	case DIK_MINUS:
		return '_';

	case DIK_A:
		return 'A' + shift;

	case DIK_B:
		return 'B' + shift;

	case DIK_C:
		return 'C' + shift;

	case DIK_D:
		return 'D' + shift;

	case DIK_E:
		return 'E' + shift;

	case DIK_F:
		return 'F' + shift;

	case DIK_G:
		return 'G' + shift;

	case DIK_H:
		return 'H' + shift;

	case DIK_I:
		return 'I' + shift;

	case DIK_J:
		return 'J' + shift;

	case DIK_K:
		return 'K' + shift;

	case DIK_L:
		return 'L' + shift;

	case DIK_M:
		return 'M' + shift;

	case DIK_N:
		return 'N' + shift;

	case DIK_O:
		return 'O' + shift;

	case DIK_P:
		return 'P' + shift;

	case DIK_Q:
		return 'Q' + shift;

	case DIK_R:
		return 'R' + shift;

	case DIK_S:
		return 'S' + shift;

	case DIK_T:
		return 'T' + shift;

	case DIK_U:
		return 'U' + shift;

	case DIK_V:
		return 'V' + shift;

	case DIK_W:
		return 'W' + shift;

	case DIK_X:
		return 'X' + shift;

	case DIK_Y:
		return 'Y' + shift;

	case DIK_Z:
		return 'Z' + shift;

	default:
		return '\0';
	}
}


void keyboard_callback()
{
	dword key;
	int state;

	DIDEVICEOBJECTDATA didod;
	dword size = 1;

	if(Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &didod, &size, 0) != DI_OK)
		return;

	while(size)
	{
		key = didod.dwOfs;
		state = didod.dwData;

		if(key == DIK_LSHIFT || key == DIK_RSHIFT)
		{
			if(state)
				shift = 0x0;
			else
				shift = 0x20;
		}
		else
		{
			if(state)
			{
				char c = get_ascii(key);

				if(c) console_keypress(c);
			}
		}

		process_keypress(key, state);

		if(Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &didod, &size, 0) != DI_OK)
			return;
	}
}


void init_input()
{
	console_print("Initializing DirectInput\n");

	if(CoCreateInstance(CLSID_DirectInput8, NULL, CLSCTX_INPROC_SERVER, IID_IDirectInput8, 
		(LPVOID*)&diObject) != S_OK)
		shutdown();

	if(diObject->Initialize(hInst, DIRECTINPUT_VERSION) != DI_OK)
		shutdown();


#ifndef _DEBUG

	// Get mouse
	
	if(diObject->CreateDevice(GUID_SysMouse, &Mouse, NULL) != DI_OK)
		shutdown();

	if(Mouse->SetDataFormat(&c_dfDIMouse) != DI_OK)
		shutdown();
	
	if(Mouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND) != DI_OK)
		shutdown();

	if(Mouse->Acquire() != DI_OK)
		shutdown();

#endif


	// Get keyboard
	
	if(diObject->CreateDevice(GUID_SysKeyboard, &Keyboard, NULL) != DI_OK)
		shutdown();

	if(Keyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK)
		shutdown();

#ifdef _DEBUG
	if(Keyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK)
#else
	if(Keyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) != DI_OK)
#endif
		shutdown();

	DIPROPDWORD dipropdword;

	dipropdword.diph.dwSize = sizeof(DIPROPDWORD);
	dipropdword.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipropdword.diph.dwObj = 0;
	dipropdword.diph.dwHow = DIPH_DEVICE;
	dipropdword.dwData = 20;
	
	if(Keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipropdword.diph) != DI_OK)
		shutdown();

	hKeyboardEvent = CreateEvent(NULL, 0, 0, NULL);
	
	if(Keyboard->SetEventNotification(hKeyboardEvent) != DI_OK)
		shutdown();

	if(Keyboard->Acquire() != DI_OK)
		shutdown();
}


void kill_input()
{
	if(Keyboard != NULL)
	{
		Keyboard->Unacquire();
		Keyboard->Release();
	}

	if(Mouse != NULL)
	{
		Mouse->Unacquire();
		Mouse->Release();
	}

	if(diObject != NULL)
	{
		diObject->Release();
	}
}


