//------------------------------------------------------------------------------
// �ļ���:		inputclass.h
// ��  ��:		
// ˵  ��:		
// ��������:	2014��5��14��
// ������:		����־
// ��ע: 
// ��Ȩ����:	������ţ�������޹�˾
//------------------------------------------------------------------------------
#ifndef _INPUTCLASS_H_
#define _INPUTCLASS_H_
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


#define DIRECTINPUT_VERSION 0x0800

class InputControl
{
public:
	enum MouseButton
	{
		MBTN_LEFT,
		MBTN_RIGHT,
		MBTN_MIDD,
	};
	InputControl();
	InputControl(const InputControl&);
	~InputControl();

	bool Initialize(HINSTANCE, HWND, int, int);
	bool Acquire();
	bool IsAcquire(){}
	void UnAcquire();
	void Shutdown();
	bool Frame();

	bool GetMouseButtonDown(MouseButton btn, bool* isChanged = NULL);
	void GetMouseMoving(int* pos);
	bool GetKeyState(char key, bool* isChanged = NULL);


	bool IsEscapePressed();
	void GetMouseLocation(int&, int&);

private:
	bool ReadKeyboard();
	bool ReadMouse();
	void ProcessInput();

private:
	IDirectInput8*			m_directInput;
	IDirectInputDevice8*	m_keyboard;
	IDirectInputDevice8*	m_mouse;


	unsigned char			m_keyboardState[256];
	unsigned char			m_keyboardPreState[256];
	DIMOUSESTATE			m_mouseState;
	DIMOUSESTATE			m_mousePreState;

	int						m_screenWidth;
	int						m_screenHeight;

	int						m_mouseX;
	int						m_mouseY;

	HWND					m_hwnd;

	bool					m_bIsAcquired;
};

#endif