////////////////////////////////////////////////////////////////////////////////
// Filename: inputclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "InputControl.h"


InputControl::InputControl()
{
	m_directInput	= 0;
	m_keyboard		= 0;
	m_mouse			= 0;
	m_bIsAcquired	= true;

	memset(&m_mouseState, 0, sizeof(DIMOUSESTATE));
	memset(&m_mousePreState, 0, sizeof(DIMOUSESTATE));
	memset(m_keyboardState, 0, sizeof(m_keyboardState));
	memset(m_keyboardPreState, 0, sizeof(m_keyboardPreState));
}


InputControl::InputControl(const InputControl& other)
{
}


InputControl::~InputControl()
{
}


bool InputControl::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	HRESULT result;

	m_hwnd = hwnd;


	// Store the screen size which will be used for positioning the mouse cursor.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// Initialize the location of the mouse on the screen.
	m_mouseX = 0;
	m_mouseY = 0;

	// Initialize the main direct input interface.
	result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_directInput, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Initialize the direct input interface for the keyboard.
	result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Set the data format.  In this case since it is a keyboard we can use the predefined data format.
	result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the keyboard to not share with other programs.
	result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
	if(FAILED(result))
	{
		return false;
	}

// 	// Now acquire the keyboard.
// 	result = m_keyboard->Acquire();
// 	if(FAILED(result))
// 	{
// 		return false;
// 	}

	// Initialize the direct input interface for the mouse.
	result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
	if(FAILED(result))
	{
		return false;
	}

	// Set the data format for the mouse using the pre-defined mouse data format.
	result = m_mouse->SetDataFormat(&c_dfDIMouse);
	if(FAILED(result))
	{
		return false;
	}

	// Set the cooperative level of the mouse to share with other programs.
	result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	if(FAILED(result))
	{
		return false;
	}

	// Acquire the mouse.
// 	result = m_mouse->Acquire();
// 	if(FAILED(result))
// 	{
// 		return false;
// 	}
// 
// 	m_bIsAcquired = true;

	return true;
}


void InputControl::Shutdown()
{
	// Release the mouse.
	if(m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = 0;
	}

	// Release the keyboard.
	if(m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = 0;
	}

	// Release the main interface to direct input.
	if(m_directInput)
	{
		m_directInput->Release();
		m_directInput = 0;
	}

	return;
}

#include <iostream>
bool InputControl::Frame()
{
	bool result;

	if (!m_bIsAcquired)
	{
		memset(&m_mouseState, 0, sizeof(DIMOUSESTATE));
		memset(&m_mousePreState, 0, sizeof(DIMOUSESTATE));
		memset(m_keyboardState, 0, sizeof(m_keyboardState));
		memset(m_keyboardPreState, 0, sizeof(m_keyboardPreState));
		return true;
	}

	memcpy(&m_mousePreState, &m_mouseState, sizeof(DIMOUSESTATE));
	memcpy(&m_keyboardPreState, &m_keyboardState, sizeof(m_keyboardState));


	// Read the current state of the keyboard.
	result = ReadKeyboard();
	if(!result)
	{
		return false;
	}

	// Read the current state of the mouse.
	result = ReadMouse();
	if(!result)
	{
		return false;
	}

	// Process the changes in the mouse and keyboard.
	ProcessInput();

	return true;
}


bool InputControl::ReadKeyboard()
{
	HRESULT result;


	// Read the keyboard device.
	result = m_keyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	if(FAILED(result))
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_keyboard->Acquire();
		}
		else
		{
			return false;
		}
	}
		
	return true;
}


bool InputControl::ReadMouse()
{
	HRESULT result;


	// Read the mouse device.
	result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if(FAILED(result))
	{
		// If the mouse lost focus or was not acquired then try to get control back.
		if((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
		{
			m_mouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}


void InputControl::ProcessInput()
{
	// Update the location of the mouse cursor based on the change of the mouse location during the frame.
	m_mouseX += m_mouseState.lX;
	m_mouseY += m_mouseState.lY;

	// Ensure the mouse location doesn't exceed the screen width or height.
 	if(m_mouseX < 0)  { m_mouseX = 0; }
 	if(m_mouseY < 0)  { m_mouseY = 0; }
 	
 	if(m_mouseX > m_screenWidth)  { m_mouseX = m_screenWidth; }
 	if(m_mouseY > m_screenHeight) { m_mouseY = m_screenHeight; }

	
	return;
}


bool InputControl::IsEscapePressed()
{
	// Do a bitwise and on the keyboard state to check if the escape key is currently being pressed.
	if(m_keyboardState[DIK_ESCAPE] & 0x80)
	{
		return true;
	}

	return false;
}


void InputControl::GetMouseLocation(int& mouseX, int& mouseY)
{
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(m_hwnd, &pt);
	//pt.y -=45;

	mouseX = pt.x;
	mouseY = pt.y;

	return;
}

bool InputControl::GetMouseButtonDown( MouseButton btn, bool* isChange /*= NULL*/ )
{
	if(isChange)
		(*isChange) = m_mouseState.rgbButtons[btn] ^ m_mousePreState.rgbButtons[btn];

	return (m_mouseState.rgbButtons[btn]&0x80);
}

void InputControl::GetMouseMoving( int* pos )
{
	pos[0] = m_mouseState.lX;
	pos[1] = m_mouseState.lY;
	pos[2] = m_mouseState.lZ;
}

bool InputControl::GetKeyState( char key, bool* isChanged /*= NULL*/ )
{
	if(isChanged)
		(*isChanged) = m_keyboardState[key]&0x80 ^ m_keyboardPreState[key]&0x80;

	return m_keyboardState[key]&0x80;	
}

bool InputControl::Acquire()
{
	HRESULT result;

	// Now acquire the keyboard.
	result = m_keyboard->Acquire();
	if(FAILED(result))
	{
		return false;
	}

	// Acquire the mouse.
	result = m_mouse->Acquire();
	if(FAILED(result))
	{
		return false;
	}

	m_bIsAcquired = true;
	return true;
}

void InputControl::UnAcquire()
{
	if(m_mouse)
		m_mouse->Unacquire();

	if(m_keyboard)
		m_keyboard->Unacquire();

	m_bIsAcquired = false;
}