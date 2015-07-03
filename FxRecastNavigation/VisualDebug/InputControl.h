//------------------------------------------------------------------------------
// 文件名:		inputclass.h
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月14日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
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