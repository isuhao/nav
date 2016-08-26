//------------------------------------------------------------------------------
// 文件名:		VisualDebugOenGLImpl.h
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月14日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#pragma once
#include "../visualdebug.h"
#include "rcDebugDraw.h"
#include <Windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
//#include "glInfo.h"
#include "InputControl.h"
#include "rcCrowdTool.h"


//#define USE_SDL

#ifdef USE_SDL
#include "SDL.h"
#endif


struct Res 
{
	HDC			hDC;
	HGLRC		hRC;
	HWND		hWnd;
	HINSTANCE	hInstance;
	GLfloat	 rtri;
};
class VisualDebugOenGLImpl : public VisualDebug
{
public:
	VisualDebugOenGLImpl(void);
	~VisualDebugOenGLImpl(void);

	virtual bool Create();
	virtual bool IsCreated()const;
	virtual void OnAddShowContent(const char* name);
	virtual void OnRemoveShowContent(const char* name);
	virtual void AllocDebugDraw(duDebugDrawItem**);

	virtual void Update(float fSeconds);
	virtual void Show(bool bShow);

	LRESULT		 WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HWND		 GetHWnd(){return m_context.hWnd;}

private:
	void		RenderCrowd(float fSeconds, dtCrowd* crowd, dtNavMesh* nav);
	void		DrawTiles(duDebugDraw* dd, dtTileCache* tc);
	void		drawObstacles(duDebugDraw* dd, const dtTileCache* tc);
	GLvoid		KillGLWindow(Res & res);
	BOOL		CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag, Res & res);
	int			InitGL(GLvoid);
	void		BeginRending();
	void		Present();
	LRESULT		ProcessInput(float);
	void		RenderOverlay();
	void		OnShowContentChanged(const char* name);

	Res			m_context;
	bool		m_isInitialized;

	rcDebugDraw	m_recastDraw;

	int			m_viewWidth;
	int			m_viewHeight;
	bool		m_bDrawGeometry;

	float		m_mouseHitpos[3];
	rcContext	m_rcContext;

	float		m_cameraPos[3];
	float		m_cameraRot[3];
	int			m_mousePos[2];
	bool		m_bRotating;
	float		m_rays[3];
	float		m_raye[3]; 
	float		m_scrollZoom;
	int			m_uiMousScroll;

	bool		m_bShow;

	float moveW, moveS, moveA, moveD;


	InputControl		m_inputControl;
	CrowdTool	m_crowdTool;


	dtNavMesh*			m_navMesh;
	dtNavMeshQuery*		m_navMeshQuery;
	dtCrowd*			m_dtCrowd;
	dtTileCache*		m_dtTileCache;
	InputTerrainGeom*	m_inputGeom;
	BuildParamter*		m_buildParmter;
	std::string			m_strCurWorldName;

	bool				m_bExpandWorld;

	WCHAR				m_szClassName[255];

#ifdef USE_SDL
	SDL_Window*			m_pSDLWindow;
#endif
};
