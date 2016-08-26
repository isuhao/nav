//------------------------------------------------------------------------------
// 文件名:		VisualDebugOenGLImpl.cpp
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月14日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#include "VisualDebugOenGLImpl.h"
#include "DetourDebugDraw.h"
#include "DetourCommon.h"
#include "../InputTerrainGeom.h"
#include "duDebugDrawGL.h"
#include "RecastDebugDraw.h"
#include "DebugDraw.h"
#include "imgui.h"
#include "imguiRenderGL.h"
#include "duDisplayItem.h"
#include <float.h>
#include <zmouse.h>
#include <math.h>
#include <set>
#include <sstream>

#ifdef USE_SDL
#pragma comment(lib, "SDL2.lib")
#endif


#pragma comment (lib, "opengl32.lib")  /* link with Microsoft OpenGL lib */
#pragma comment (lib, "glu32.lib")     /* link with OpenGL Utility lib */
std::set<VisualDebugOenGLImpl*> msgProcessor;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	std::set<VisualDebugOenGLImpl*>::iterator it = msgProcessor.begin();
	for(;it!=msgProcessor.end();++it)
	{
		VisualDebugOenGLImpl* ptr = (*it);
		if(NULL==ptr|| ptr->GetHWnd()!=hWnd)
			continue;
		ptr->WndProc(hWnd,uMsg,wParam,lParam);		
	}
	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}


VisualDebugOenGLImpl::VisualDebugOenGLImpl(void):m_isInitialized(false),m_viewWidth(1280),m_viewHeight(720)
,m_bDrawGeometry(true)
,m_bShow(true)
,m_uiMousScroll(0)
,m_navMesh(0)
,m_navMeshQuery(0)
,m_dtCrowd(0)
,m_dtTileCache(0)
,m_inputGeom(0)
,m_buildParmter(0)
{
	m_scrollZoom = 0;
	dtVset(m_cameraRot, 0, 0, 0);

	memset(&m_context, 0, sizeof(m_context));

	moveW = 0;
	moveS = 0;
	moveA = 0;
	moveD = 0;
	
}

VisualDebugOenGLImpl::~VisualDebugOenGLImpl(void){}

bool VisualDebugOenGLImpl::Create()
{	

#ifndef USE_SDL
	CreateGLWindow("", m_viewWidth, m_viewHeight, 16, false, m_context);

	if (!m_inputControl.Initialize(m_context.hInstance, m_context.hWnd, m_viewWidth, m_viewHeight))
		return false;
#else
	// Init SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("Could not initialise SDL\n");
		return false;
	}

	// Center window
	char env[] = "SDL_VIDEO_CENTERED=1";
	putenv(env);

	// Init OpenGL
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	//#ifndef WIN32
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);


	m_pSDLWindow = SDL_CreateWindow("Hello World!", 100, 100, m_viewWidth, m_viewHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (m_pSDLWindow == NULL){
		//std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return false;
	}
	SDL_GLContext maincontext = SDL_GL_CreateContext(m_pSDLWindow);
#endif

	msgProcessor.insert(this);

	m_isInitialized = true;



	if ( !imguiRenderGLInit("DroidSans.ttf") )
		return false;

	m_crowdTool.init();
	m_crowdTool.reset(&m_recastDraw);

	return true;
}

bool VisualDebugOenGLImpl::IsCreated() const
{
	return m_isInitialized;
}

void VisualDebugOenGLImpl::BeginRending()
{
	float fogCol[4] = { 0.32f, 0.31f, 0.30f, 1.0f };
	if (m_inputGeom)
	{
		const float* bmin = m_inputGeom->getMeshBoundsMin();
		const float* bmax = m_inputGeom->getMeshBoundsMax();
		float camr = sqrtf(rcSqr(bmax[0]-bmin[0]) +
			rcSqr(bmax[1]-bmin[1]) +
			rcSqr(bmax[2]-bmin[2])) / 2;
		camr *= 3;
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, camr*0.1f);
		glFogf(GL_FOG_END, camr*1.25f);
		glFogfv(GL_FOG_COLOR, fogCol);
	}

	// Update and render
	glViewport(0, 0, m_viewWidth, m_viewHeight);
	glClearColor(0.3f, 0.3f, 0.32f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);

	// Render 3d
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0f, (float)m_viewWidth/(float)m_viewHeight, 1.0f, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
 	glRotatef(m_cameraRot[0],1,0,0);
 	glRotatef(m_cameraRot[1],0,1,0);
	glTranslatef(-m_cameraPos[0], -m_cameraPos[1], -m_cameraPos[2]);
}
void VisualDebugOenGLImpl::Update(float fSeconds)
{
	if (!m_bShow)
		return;

#ifndef USE_SDL
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//if ( m_inputControl.IsAcquire() )
		m_inputControl.Frame();

	ProcessInput(fSeconds);
#endif // !USE_SDL


	BeginRending();
	duDebugDrawGL dd;


	int mx, my;
	m_inputControl.GetMouseLocation(mx, my);
	my = m_viewHeight-1-my;

	GLdouble proj[16];
	GLdouble model[16];
	GLint view[4];


	// Get hit ray position and direction.
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetIntegerv(GL_VIEWPORT, view);
	GLdouble x, y, z;
	gluUnProject(mx, my, 0.0f, model, proj, view, &x, &y, &z);
	m_rays[0] = (float)x; m_rays[1] = (float)y; m_rays[2] = (float)z;
	gluUnProject(mx, my, 1.0f, model, proj, view, &x, &y, &z);
	m_raye[0] = (float)x; m_raye[1] = (float)y; m_raye[2] = (float)z;

	dd.begin(DU_DRAW_LINES, 2);
	dd.vertex((const float*)m_raye,duRGBA(255,255,255,255));
	dd.vertex((const float*)m_rays,duRGBA(255,255,255,255));
	dd.end();
	//draw mouse hit 
	if (true)
	{
		float s = 0.3;
		unsigned int color = duRGBA(0,0,0,128);
		const float* hitPos = m_mouseHitpos;
		dd.begin(DU_DRAW_LINES, 2.f);
		dd.vertex(hitPos[0]-s,hitPos[1]+0.1f,  hitPos[2], color);
		dd.vertex(hitPos[0]+s,hitPos[1]+0.1f,  hitPos[2], color);
		dd.vertex(hitPos[0],  hitPos[1]-s+0.1f,hitPos[2], color);
		dd.vertex(hitPos[0],  hitPos[1]+s+0.1f,hitPos[2], color);
		dd.vertex(hitPos[0],  hitPos[1]+0.1f,  hitPos[2]-s, color);
		dd.vertex(hitPos[0],  hitPos[1]+0.1f,  hitPos[2]+s, color);
		dd.end();

	}


	// draw geometry
	if (m_inputGeom && m_bDrawGeometry)
	{
		const float texScale = 1.0f / (m_buildParmter->m_cellSize * 10.0f);
		duDebugDrawTriMeshSlope(&dd, m_inputGeom->getMesh()->getVerts(), m_inputGeom->getMesh()->getVertCount(),
			m_inputGeom->getMesh()->getTris(), m_inputGeom->getMesh()->getNormals(), m_inputGeom->getMesh()->getTriCount(),
			m_buildParmter->m_agentMaxSlope, texScale);
	}
#if 1

	m_recastDraw.Draw(fSeconds, &dd, 
		m_navMesh, m_navMeshQuery, m_dtCrowd, m_dtTileCache, 
		m_inputGeom?m_inputGeom->getMeshBoundsMin():0, 
		m_inputGeom?m_inputGeom->getMeshBoundsMax():0, 
		m_buildParmter);

	const int nDispayItemCount = GetDisplayItemCount();
	for (int i=0; i<nDispayItemCount; ++i)
	{
		duDebugDrawItem* draw = GetDisplayItemByIndex(i);
		draw->draw(&dd);
	}

	if(m_inputGeom)
		m_inputGeom->drawOffMeshConnections(&dd);

#endif
	RenderOverlay();

	Present();
}

void VisualDebugOenGLImpl::RenderOverlay()
{
	// Render GUI
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, m_viewWidth, 0, m_viewHeight);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	bool isMbsChanged = false;
	int mx, my;
	unsigned char mbut = 0;
	m_inputControl.GetMouseLocation(mx, my);
	my = m_viewHeight - 1 - my;
	if(m_inputControl.GetMouseButtonDown(InputControl::MBTN_LEFT))
		mbut |= IMGUI_MBUT_LEFT;
	if(m_inputControl.GetMouseButtonDown(InputControl::MBTN_RIGHT))
		mbut |= IMGUI_MBUT_RIGHT;

	imguiBeginFrame(mx,my,mbut,m_uiMousScroll);



	const char msg[] = "W/S/A/D: Move  RMB: Rotate";
	imguiDrawText(280, m_viewHeight-20, IMGUI_ALIGN_LEFT, msg, imguiRGBA(255,255,255,128));

	// Tools
	if (true) // && geom && sample)
	{
		int toolsScroll = 0;
		if (imguiBeginScrollArea("Tools", 10, 10, 250, m_viewHeight-20, &toolsScroll));

		m_crowdTool.handleMenu();


		//world content
		if (imguiCollapse("World", 0, m_bExpandWorld))
			m_bExpandWorld = !m_bExpandWorld;
		if (m_bExpandWorld)
		{
			imguiIndent();

			for (WorldByNameConstIter it= m_worldByName.begin(); 
				it!=m_worldByName.end(); ++it)
			{
				if (imguiCheck(it->first.c_str(), m_strCurWorldName==it->first))
				{
					const NaviWorld& world = it->second;
					m_strCurWorldName = it->first;
					m_navMesh		= world.m_navMesh;
					m_navMeshQuery	= world.m_navMeshQuery;
					m_dtCrowd		= world.m_dtCrowd;
					m_dtTileCache	= world.m_dtTileCache;
					m_inputGeom		= world.m_inputGeom;
					m_buildParmter	= world.m_buildParmter;

					OnShowContentChanged(m_strCurWorldName.c_str());
				}
			}
			


			//if (imguiCheck("Show Corners", params->m_showCorners))
			//	params->m_showCorners = !params->m_showCorners;
			//if (imguiCheck("Show Collision Segs", params->m_showCollisionSegments))
			//	params->m_showCollisionSegments = !params->m_showCollisionSegments;
			//if (imguiCheck("Show Path", params->m_showPath))
			//	params->m_showPath = !params->m_showPath;
			//if (imguiCheck("Show VO", params->m_showVO))
			//	params->m_showVO = !params->m_showVO;
			//// 		if (imguiCheck("Show Path Optimization", params->m_showOpt))
			//// 			params->m_showOpt = !params->m_showOpt;
			//if (imguiCheck("Show Neighbours", params->m_showNeis))
			//	params->m_showNeis = !params->m_showNeis;
			imguiUnindent();
		}


		imguiEndScrollArea();
	}




	imguiEndFrame();
	imguiRenderGLDraw();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);

}


void VisualDebugOenGLImpl::Present()
{
#ifndef USE_SDL
	SwapBuffers(m_context.hDC);
#else
	SDL_GL_SwapWindow(m_pSDLWindow);
#endif // !USE_SDL


}

int VisualDebugOenGLImpl::InitGL(GLvoid)									
{
 	glShadeModel(GL_SMOOTH);							
	//glEnable(GL_MULTISAMPLE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0f, (float)m_viewWidth/(float)m_viewHeight, 1.0f, 1000);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	return TRUE;										
}

GLvoid VisualDebugOenGLImpl::KillGLWindow(Res & res)
{

	if (res.hRC)
	{
		if (!wglMakeCurrent(NULL,NULL))
		{
			//MessageBox(NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(res.hRC))	
		{
			//MessageBox(NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		}
		res.hRC=NULL;
	}

	if (res.hDC && !ReleaseDC(res.hWnd,res.hDC))
	{
		//MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		res.hDC=NULL;
	}

	if (res.hWnd && !DestroyWindow(res.hWnd))
	{
		//MessageBox(NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		res.hWnd=NULL;
	}

	if (!UnregisterClass(m_szClassName, res.hInstance))
	{

		res.hInstance=NULL;
	}
}
BOOL VisualDebugOenGLImpl::CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag, Res & res)
{
	GLuint		PixelFormat;			
	WNDCLASS	wc;						
	DWORD		dwExStyle;			
	DWORD		dwStyle;				
	RECT		WindowRect;				
	WindowRect.left=(long)0;			
	WindowRect.right=(long)width;		
	WindowRect.top=(long)0;				
	WindowRect.bottom=(long)height;		
	
	wsprintf(m_szClassName,L"VisualDebug%d",(int)this);


	res.hInstance			= GetModuleHandle(NULL);				
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	
	wc.lpfnWndProc		= (WNDPROC) ::WndProc;					
	wc.cbClsExtra		= 0;									
	wc.cbWndExtra		= 0;									
	wc.hInstance		= res.hInstance;							
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;									
	wc.lpszMenuName		= NULL;									
	wc.lpszClassName	= m_szClassName;								

	if (!RegisterClass(&wc))									
	{
	}

	dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	dwStyle=WS_OVERLAPPEDWINDOW;	

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	// Create The Window
	if (!(res.hWnd=CreateWindowEx(	dwExStyle,
		m_szClassName,
		L"Visual Debug Tool",
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,
		WindowRect.right-WindowRect.left,
		WindowRect.bottom-WindowRect.top,	
		NULL,
		NULL,
		res.hInstance,
		NULL)))
	{
		KillGLWindow(res);
		//MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	static	PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |	
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,	
		bits,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	if (!(res.hDC=GetDC(res.hWnd)))
	{
		KillGLWindow(res);
		//MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(PixelFormat=ChoosePixelFormat(res.hDC,&pfd)))
	{
		KillGLWindow(res);
		//MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!SetPixelFormat(res.hDC,PixelFormat,&pfd))
	{
		KillGLWindow(res);
		//MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(res.hRC=wglCreateContext(res.hDC)))
	{
		KillGLWindow(res);
		//MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!wglMakeCurrent(res.hDC,res.hRC))
	{
		KillGLWindow(res);
		//MessageBox(NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	ShowWindow(res.hWnd,SW_SHOW);
	SetForegroundWindow(res.hWnd);
	SetFocus(res.hWnd);

	if (!InitGL())
	{
		KillGLWindow(res);
		//MessageBox(NULL,"Initialization Failed.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	return TRUE;
}


LRESULT VisualDebugOenGLImpl::ProcessInput(float dt)
{
	bool mouseOverMenu = false;

	GLdouble proj[16];
	GLdouble model[16];
	GLint view[4];
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetIntegerv(GL_VIEWPORT, view);


	int mouseMove[3];
	m_inputControl.GetMouseMoving(mouseMove);
	bool isChanged;
	if(m_inputControl.GetMouseButtonDown(InputControl::MBTN_RIGHT, &isChanged) )
	{
		if (!mouseOverMenu)
		{
			m_cameraRot[1] += mouseMove[0] * 0.25f;
			m_cameraRot[0] += mouseMove[1] * 0.25f;			
		}
	}

	if (mouseMove[2]!=0)
		m_scrollZoom = mouseMove[2]>0?-1.0f:1.0f;



	// Handle keyboard movement.
	moveW = rcClamp(moveW + dt * 4 * (m_inputControl.GetKeyState(DIK_W) ? 1 : -1), 0.0f, 1.0f);
	moveS = rcClamp(moveS + dt * 4 * (m_inputControl.GetKeyState(DIK_S) ? 1 : -1), 0.0f, 1.0f);
	moveA = rcClamp(moveA + dt * 4 * (m_inputControl.GetKeyState(DIK_A) ? 1 : -1), 0.0f, 1.0f);
	moveD = rcClamp(moveD + dt * 4 * (m_inputControl.GetKeyState(DIK_D) ? 1 : -1), 0.0f, 1.0f);
	float keybSpeed = 22.0f;
	float movex = (moveD - moveA) * keybSpeed * dt;
	float movey = (moveS - moveW) * keybSpeed * dt;

	movey += m_scrollZoom * 2.0f;
	m_scrollZoom = 0;

	m_cameraPos[0] += movex * (float)model[0];
	m_cameraPos[1] += movex * (float)model[4];
	m_cameraPos[2] += movex * (float)model[8];

	m_cameraPos[0] += movey * (float)model[2];
	m_cameraPos[1] += movey * (float)model[6];
	m_cameraPos[2] += movey * (float)model[10];


	if( m_inputControl.GetMouseButtonDown(InputControl::MBTN_LEFT, &isChanged) && isChanged 
		&& m_inputGeom)
	{
		float hitt;
		bool hit = m_inputGeom->raycastMesh(m_rays, m_raye, hitt);

		if (hit)
		{
			m_mouseHitpos[0] = m_rays[0] + (m_raye[0] - m_rays[0])*hitt;
			m_mouseHitpos[1] = m_rays[1] + (m_raye[1] - m_rays[1])*hitt;
			m_mouseHitpos[2] = m_rays[2] + (m_raye[2] - m_rays[2])*hitt;
		}
	}

	if (m_inputControl.GetKeyState(DIK_LCONTROL)
		 && m_inputControl.GetKeyState(DIK_C,&isChanged) && isChanged )
	{
		HANDLE m_hGlobalMemory = GlobalAlloc(GHND, 255);
		LPBYTE lpGlobalMemory = (LPBYTE)GlobalLock(m_hGlobalMemory);
		if(lpGlobalMemory)
		{
			sprintf_s((char*)lpGlobalMemory, 255, "%f,%f,%f", m_mouseHitpos[0],m_mouseHitpos[1],m_mouseHitpos[2]);
			::GlobalUnlock(m_hGlobalMemory);
			::OpenClipboard(m_context.hWnd);
			::EmptyClipboard();
			::SetClipboardData(CF_TEXT, m_hGlobalMemory);
			::CloseClipboard();
		}
	}


	return 0;
}

void VisualDebugOenGLImpl::Show( bool bShow )
{
	if (!IsCreated())
		return;
	m_bShow = bShow;
	ShowWindow(m_context.hWnd, m_bShow?SW_SHOW:SW_HIDE);
}

LRESULT VisualDebugOenGLImpl::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_ACTIVATE)
	{
 		bool isActive = LOWORD(wParam)!=WA_INACTIVE;
 		if (isActive)
 			m_inputControl.Acquire();
 		else
 			m_inputControl.UnAcquire();
	}

	if (uMsg == WM_SIZE)
	{
		m_viewWidth = LOWORD(lParam);
		m_viewHeight = HIWORD(lParam);
	}
 	if(uMsg==WM_CLOSE)
 	{
 		KillGLWindow(m_context);
 		m_inputControl.Shutdown();
 		m_isInitialized = false;
 	}

	return 0;
}

void VisualDebugOenGLImpl::OnAddShowContent(const char* name)
{
	if (m_strCurWorldName.empty())
	{
		m_strCurWorldName = name;
		NaviWorld world;
		GetShowContent(name, world);

		m_navMesh		= world.m_navMesh;
		m_navMeshQuery	= world.m_navMeshQuery;
		m_dtCrowd		= world.m_dtCrowd;
		m_dtTileCache	= world.m_dtTileCache;
		m_inputGeom		= world.m_inputGeom;
		m_buildParmter	= world.m_buildParmter;
		OnShowContentChanged(m_strCurWorldName.c_str());
	}
}

void VisualDebugOenGLImpl::OnShowContentChanged(const char* name)
{
	if( !m_inputGeom )
		return;
	const float* bmin = m_inputGeom->getMeshBoundsMin();
	const float* bmax = m_inputGeom->getMeshBoundsMax();
	float center[3];

	center[0] = dtLerp(bmin[0], bmax[0], 0.5f);
	center[1] = dtLerp(bmin[1], bmax[1], 0.5f);
	center[2] = dtLerp(bmin[2], bmax[2], 0.5f);

	m_cameraPos[0] = center[0];
	m_cameraPos[1] = bmax[1];
	m_cameraPos[2] = center[2];
}

void VisualDebugOenGLImpl::OnRemoveShowContent( const char* name )
{
	if( m_strCurWorldName == name )
	{
		if( !m_worldByName.empty() )
		{
			WorldByNameConstIter it = m_worldByName.begin();
			m_strCurWorldName = it->first;
			const NaviWorld& world = it->second;

			m_navMesh		= world.m_navMesh;
			m_navMeshQuery	= world.m_navMeshQuery;
			m_dtCrowd		= world.m_dtCrowd;
			m_dtTileCache	= world.m_dtTileCache;
			m_inputGeom		= world.m_inputGeom;
			m_buildParmter	= world.m_buildParmter;
			OnShowContentChanged(m_strCurWorldName.c_str());
		}
		else
		{
			m_strCurWorldName.clear();
			m_navMesh		= 0;
			m_navMeshQuery	= 0;
			m_dtCrowd		= 0;
			m_dtTileCache	= 0;
			m_inputGeom		= 0;
			m_buildParmter	= 0;
		}

	}
}
void VisualDebugOenGLImpl::AllocDebugDraw( duDebugDrawItem** out)
{
	(*out) = new duDisplayItem();
}

VisualDebug* GetVisualDebug()
{
	static VisualDebugOenGLImpl ogl;
	return &ogl;
}