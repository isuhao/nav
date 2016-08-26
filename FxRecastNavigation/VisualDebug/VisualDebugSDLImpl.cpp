//------------------------------------------------------------------------------
// 文件名:		VisualDebugSDLImpl.cpp
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月6日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------

#include "VisualDebugSDLImpl.h"
#include "imguiRenderGL.h"
#include "imgui.h"
#include "DetourDebugDraw.h"
#include "DetourCommon.h"
#include "../InputTerrainGeom.h"
#include "duDebugDrawGL.h"
#include "RecastDebugDraw.h"
#include <float.h>
#include "duDisplayItem.h"
#include <GL/glu.h>

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")


const char* TITLE_STRING = "Recast Navigation Visual Debug";

//duDisplayList debugDraw;

//DisplayList debugDraw;
VisualDebugSDLImpl::VisualDebugSDLImpl(void):m_isInitialized(false)
,m_showPath(true)
,m_showGrid(false)
,m_showVO(false)
,m_showCorners(false)
,m_anticipateTurns(false)
,m_showNeis(true)
,m_showNodes(true)
,m_showCollisionSegments(true)
,m_inputGeom(NULL)
,m_navMeshDrawFlags(DU_DRAWNAVMESH_OFFMESHCONS|DU_DRAWNAVMESH_OFFMESHCONS)
,m_bDrawTiles(false)
,m_bDrawGeometry(true)
,m_bMousePosSet(false)
, m_viewWidth(1024)
, m_viewHeight(768){}

VisualDebugSDLImpl::~VisualDebugSDLImpl(void){}

bool VisualDebugSDLImpl::Create()
{
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
	//#endif
#if 0
	const SDL_VideoInfo* vi = SDL_GetVideoInfo();

	presentationMode = false;

	SDL_Surface* screen = 0;

	if (presentationMode)
	{
		m_viewWidth = vi->current_w;
		m_viewHeight = vi->current_h;
		screen = SDL_CreateWindow(m_viewWidth, m_viewHeight, 0, SDL_OPENGL | SDL_FULLSCREEN);
	}
	else
	{	
		m_viewWidth = vi->current_w - 20;
		m_viewHeight = vi->current_h - 80;
		screen = SDL_SetVideoMode(m_viewWidth, m_viewHeight, 0, SDL_OPENGL);
	}

	if (!screen)
	{
		printf("Could not initialise SDL opengl\n");
		return false;
	}
	glEnable(GL_MULTISAMPLE);

	SDL_WM_SetCaption(TITLE_STRING, 0);
#else
	m_pSDLWindow = SDL_CreateWindow("Hello World!", 100, 100, m_viewWidth, m_viewHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (m_pSDLWindow == NULL){
		//std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return false;
	}
	SDL_GLContext maincontext = SDL_GL_CreateContext(m_pSDLWindow);
#endif

	if (!imguiRenderGLInit("DroidSans.ttf"))
	{
		printf("Could not init GUI renderer.\n");
		SDL_Quit();
		return false;
	}

	m_isInitialized = true;

	camr = 1000;
	rx = 45;
	ry = -45;


	m_hRenderWnd	= ::FindWindowA(0, TITLE_STRING);




	return true;
}

void VisualDebugSDLImpl::ProcessInput(float dt)
{
	static float t = 0.0f;
	static float timeAcc = 0.0f;
	static int mx = 0, my = 0;
	static float moveW = 0, moveS = 0, moveA = 0, moveD = 0;
	static float origrx = 0, origry = 0;
	static int origx = 0, origy = 0;
	static float scrollZoom = 0;
	static bool rotate = false;
	bool movedDuringRotate = false;
	static float rays[3], raye[3]; 
	bool mouseOverMenu = false;
	bool showMenu = !presentationMode;
	bool showLog = false;
	bool showTools = true;
	bool showLevels = false;
	bool showSample = false;
	bool showTestCases = false;

	int propScroll = 0;
	int logScroll = 0;
	int toolsScroll = 0;

	int mscroll = 0;
	bool processHitTest = false;
	bool processHitTestShift = false;
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYUP:
			{
				if ( event.key.keysym.sym==SDLK_c && SDL_GetModState()&KMOD_CTRL )
				{
					HANDLE m_hGlobalMemory = GlobalAlloc(GHND, 255);
					LPBYTE lpGlobalMemory = (LPBYTE)GlobalLock(m_hGlobalMemory);
					if(lpGlobalMemory)
					{
						sprintf_s((char*)lpGlobalMemory, 255, "%f,%f,%f", m_mouseHitpos[0],m_mouseHitpos[1],m_mouseHitpos[2]);
						::GlobalUnlock(m_hGlobalMemory);
						::OpenClipboard(m_hRenderWnd);
						::EmptyClipboard();
						::SetClipboardData(CF_TEXT, m_hGlobalMemory);
						::CloseClipboard();
					}

				}
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				if (!mouseOverMenu)
				{
					// Rotate view
					rotate = true;
					movedDuringRotate = false;
					origx = mx;
					origy = my;
					origrx = rx;
					origry = ry;
				}
			}	

#if 0
			else if (event.button.button == SDL_BUTTON_WHEELUP)
			{
				if (mouseOverMenu)
					mscroll--;
				else
					scrollZoom -= 1.0f;
			}
			else if (event.button.button == SDL_BUTTON_WHEELDOWN)
			{
				if (mouseOverMenu)
					mscroll++;
				else
					scrollZoom += 1.0f;
			}
#endif
			break;

		case SDL_MOUSEBUTTONUP:
			// Handle mouse clicks here.
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				rotate = false;
				if (!mouseOverMenu)
				{
					if (!movedDuringRotate)
					{
						processHitTest = true;
						processHitTestShift = true;
					}
				}
			}
			else if (event.button.button == SDL_BUTTON_LEFT)
			{
				if (!mouseOverMenu)
				{
					processHitTest = true;
					processHitTestShift = (SDL_GetModState() & KMOD_SHIFT) ? true : false;
				}
			}

			break;

		case SDL_MOUSEMOTION:
			mx = event.motion.x;
			my = m_viewHeight-1 - event.motion.y;
			if (rotate)
			{
				int dx = mx - origx;
				int dy = my - origy;
				rx = origrx - dy*0.25f;
				ry = origry + dx*0.25f;
				if (dx*dx+dy*dy > 3*3)
					movedDuringRotate = true;
			}
			break;

		case SDL_QUIT:
			//done = true;
			break;
		}
	}

#if 1
	// Hit test mesh.
	if (processHitTest && m_inputGeom)
	{
		float hitt;
		bool hit = m_inputGeom->raycastMesh(rays, raye, hitt);

		if (hit)
		{
			// Marker
			m_bMousePosSet = true;
			m_mouseHitpos[0] = rays[0] + (raye[0] - rays[0])*hitt;
			m_mouseHitpos[1] = rays[1] + (raye[1] - rays[1])*hitt;
			m_mouseHitpos[2] = rays[2] + (raye[2] - rays[2])*hitt;
		}
	}
#endif


	// Get hit ray position and direction.
	GLdouble proj[16];
	GLdouble model[16];
	GLint view[4];
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetIntegerv(GL_VIEWPORT, view);
	GLdouble x, y, z;
	gluUnProject(m_viewWidth/2, m_viewHeight/2, 0.0f, model, proj, view, &x, &y, &z);
	rays[0] = (float)x; rays[1] = (float)y; rays[2] = (float)z;
	gluUnProject(mx, my, 1.0f, model, proj, view, &x, &y, &z);
	raye[0] = (float)x; raye[1] = (float)y; raye[2] = (float)z;

// 	debugDraw.clear();
// 	debugDraw.begin(DU_DRAW_LINES, 2);
// 	debugDraw.vertex((const float*)raye,duRGBA(255,255,255,255));
// 	debugDraw.vertex((const float*)rays,duRGBA(255,255,255,255));
// 	debugDraw.end();

	// Handle keyboard movement.
	//Uint8* keystate = SDL_GetKeyState(NULL);
	const Uint8* keystate = SDL_GetKeyboardState(NULL);

	moveW = rcClamp(moveW + dt * 4 * (keystate[SDLK_w] ? 1 : -1), 0.0f, 1.0f);
	moveS = rcClamp(moveS + dt * 4 * (keystate[SDLK_s] ? 1 : -1), 0.0f, 1.0f);
	moveA = rcClamp(moveA + dt * 4 * (keystate[SDLK_a] ? 1 : -1), 0.0f, 1.0f);
	moveD = rcClamp(moveD + dt * 4 * (keystate[SDLK_d] ? 1 : -1), 0.0f, 1.0f);

	float keybSpeed = 22.0f;
	if (SDL_GetModState() & KMOD_SHIFT)
		keybSpeed *= 4.0f;

	float movex = (moveD - moveA) * keybSpeed * dt;
	float movey = (moveS - moveW) * keybSpeed * dt;

	movey += scrollZoom * 2.0f;
	scrollZoom = 0;

	m_cameraPos[0] += movex * (float)model[0];
	m_cameraPos[1] += movex * (float)model[4];
	m_cameraPos[2] += movex * (float)model[8];

	m_cameraPos[0] += movey * (float)model[2];
	m_cameraPos[1] += movey * (float)model[6];
	m_cameraPos[2] += movey * (float)model[10];

}

void VisualDebugSDLImpl::Update(float fSeconds)
{
	ProcessInput(fstep);

	unsigned char mbut = 0;
	if (SDL_GetMouseState(0,0) & SDL_BUTTON_LMASK)
		mbut |= IMGUI_MBUT_LEFT;
	if (SDL_GetMouseState(0,0) & SDL_BUTTON_RMASK)
		mbut |= IMGUI_MBUT_RIGHT;



	if( BeginRender() )
	{
		duDebugDrawGL dd;


		int mx, my;
		m_inputControl.GetMouseLocation(mx, my);
		my = m_viewHeight - 1 - my;

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
		dd.vertex((const float*)m_raye, duRGBA(255, 255, 255, 255));
		dd.vertex((const float*)m_rays, duRGBA(255, 255, 255, 255));
		dd.end();

		//draw mouse hit 
		{
			float s = 0.3;
			unsigned int color = duRGBA(0, 0, 0, 128);
			const float* hitPos = m_mouseHitpos;
			dd.begin(DU_DRAW_LINES, 2.f);
			dd.vertex(hitPos[0] - s, hitPos[1] + 0.1f, hitPos[2], color);
			dd.vertex(hitPos[0] + s, hitPos[1] + 0.1f, hitPos[2], color);
			dd.vertex(hitPos[0], hitPos[1] - s + 0.1f, hitPos[2], color);
			dd.vertex(hitPos[0], hitPos[1] + s + 0.1f, hitPos[2], color);
			dd.vertex(hitPos[0], hitPos[1] + 0.1f, hitPos[2] - s, color);
			dd.vertex(hitPos[0], hitPos[1] + 0.1f, hitPos[2] + s, color);
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

		m_recastDraw.Draw(fSeconds, &dd,
			m_navMesh, m_navMeshQuery, m_dtCrowd, m_dtTileCache,
			m_inputGeom ? m_inputGeom->getMeshBoundsMin() : 0,
			m_inputGeom ? m_inputGeom->getMeshBoundsMax() : 0,
			m_buildParmter);

		const int nDispayItemCount = GetDisplayItemCount();
		for (int i = 0; i < nDispayItemCount; ++i)
		{
			duDebugDrawItem* draw = GetDisplayItemByIndex(i);
			draw->draw(&dd);
		}

		if (m_inputGeom)
			m_inputGeom->drawOffMeshConnections(&dd);


		//RenderOverlay();
		EndRender();
	}
}

void VisualDebugSDLImpl::Release()
{
	if(m_isInitialized)
	{
		imguiRenderGLDestroy();
		SDL_Quit();
		m_isInitialized = false;
	}
}

bool VisualDebugSDLImpl::BeginRender()
{
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
	glRotatef(rx,1,0,0);
	glRotatef(ry,0,1,0);
	glTranslatef(-m_cameraPos[0], -m_cameraPos[1], -m_cameraPos[2]);
	return true;
}

void VisualDebugSDLImpl::RenderGUI(float fSeconds)
{
	// Render GUI
	glDisable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, m_viewWidth, 0, m_viewHeight);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Help text.
	if (showMenu)
	{
		const char msg[] = "W/S/A/D: Move  RMB: Rotate";
		imguiDrawText(280, m_viewHeight-20, IMGUI_ALIGN_LEFT, msg, imguiRGBA(255,255,255,128));
	}


	if (m_bMousePosSet)
	{	
		GLdouble x, y, z;
		GLdouble proj[16];
		GLdouble model[16];
		GLint view[4];
		glGetDoublev(GL_PROJECTION_MATRIX, proj);
		glGetDoublev(GL_MODELVIEW_MATRIX, model);
		glGetIntegerv(GL_VIEWPORT, view);
		gluProject((GLdouble)m_mouseHitpos[0], (GLdouble)m_mouseHitpos[1], (GLdouble)m_mouseHitpos[2],
			model, proj, view, &x, &y, &z);
		imguiDrawText((int)x, (int)(y-25), IMGUI_ALIGN_CENTER, "End", imguiRGBA(0,0,0,220));

	}


	imguiEndFrame();
	imguiRenderGLDraw();		

	glEnable(GL_DEPTH_TEST);

}
void VisualDebugSDLImpl::EndRender()
{
	SDL_GL_SwapWindow(m_pSDLWindow);//SDL_GL_SwapBuffers();
}


void VisualDebugSDLImpl::DrawTiles(duDebugDraw* dd, dtTileCache* tc)
{
	unsigned int fcol[6];
	float bmin[3], bmax[3];

	for (int i = 0; i < tc->getTileCount(); ++i)
	{
		const dtCompressedTile* tile = tc->getTile(i);
		if (!tile->header) continue;

		tc->calcTightTileBounds(tile->header, bmin, bmax);

		const unsigned int col = duIntToCol(i,64);
		duCalcBoxColors(fcol, col, col);
		duDebugDrawBox(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], fcol);
	}

	for (int i = 0; i < tc->getTileCount(); ++i)
	{
		const dtCompressedTile* tile = tc->getTile(i);
		if (!tile->header) continue;

		tc->calcTightTileBounds(tile->header, bmin, bmax);

		const unsigned int col = duIntToCol(i,255);
		const float pad = tc->getParams()->cs * 0.1f;
		duDebugDrawBoxWire(dd, bmin[0]-pad,bmin[1]-pad,bmin[2]-pad,
			bmax[0]+pad,bmax[1]+pad,bmax[2]+pad, col, 2.0f);
	}

}

void VisualDebugSDLImpl::drawObstacles(duDebugDraw* dd, const dtTileCache* tc)
{
	// Draw obstacles
	for (int i = 0; i < tc->getObstacleCount(); ++i)
	{
		const dtTileCacheObstacle* ob = tc->getObstacle(i);
		if (ob->state == DT_OBSTACLE_EMPTY) continue;
		float bmin[3], bmax[3];
		tc->getObstacleBounds(ob, bmin,bmax);

		unsigned int col = 0;
		if (ob->state == DT_OBSTACLE_PROCESSING)
			col = duRGBA(255,255,0,128);
		else if (ob->state == DT_OBSTACLE_PROCESSED)
			col = duRGBA(255,192,0,192);
		else if (ob->state == DT_OBSTACLE_REMOVING)
			col = duRGBA(220,0,0,128);

		if (ob->shapeType==dtTileCacheObstacle::dtBox)
		{
			unsigned int colors[6] = {col,col,col,col,col,col};
			//duDebugDrawBox(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], colors);
			duDebugDrawBoxWire(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], duDarkenCol(col), 2);
			duDebugDrawOBB(dd, ob->pos, ob->boxHalfExtents, ob->fRotate, col);
			duDebugDrawOBBWire(dd, ob->pos, ob->boxHalfExtents, ob->fRotate, duDarkenCol(col), 2);
		}
		else
		{
			duDebugDrawCylinder(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], col);
			duDebugDrawCylinderWire(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], duDarkenCol(col), 2);
		}

	}
}


void VisualDebugSDLImpl::RenderCrowd(float fSeconds, dtCrowd* crowd, dtNavMesh* nav)
{
	duDebugDrawGL dd;

	if (!nav || !crowd)
		return;

	if (m_showNodes && crowd->getPathQueue())
	{
		const dtNavMeshQuery* navquery = crowd->getPathQueue()->getNavQuery();
		if (navquery)
			duDebugDrawNavMeshNodes(&dd, *navquery);
	}

	dd.depthMask(false);

	// Draw paths
	if (m_showPath)
	{
		for (int i = 0; i < crowd->getAgentCount(); i++)
		{
// 			if (m_toolParams.m_showDetailAll == false && i != m_agentDebug.idx)
// 				continue;
			const dtCrowdAgent* ag =crowd->getAgent(i);
			if (!ag->active)
				continue;
			const dtPolyRef* path = ag->corridor.getPath();
			const int npath = ag->corridor.getPathCount();			
			for (int j = 0; j < npath; ++j)
				duDebugDrawNavMeshPoly(&dd, *nav, path[j], duRGBA(255,255,255,24));
		}
	}

// 	if (m_targetRef)
// 		duDebugDrawCross(&dd, m_targetPos[0],m_targetPos[1]+0.1f,m_targetPos[2], rad, duRGBA(255,255,255,192), 2.0f);

	// Occupancy grid.
	if (m_showGrid)
	{
		float gridy = -FLT_MAX;
		for (int i = 0; i < crowd->getAgentCount(); ++i)
		{
			const dtCrowdAgent* ag = crowd->getAgent(i);
			if (!ag->active) continue;
			const float* pos = ag->corridor.getPos();
			gridy = dtMax(gridy, pos[1]);
		}
		gridy += 1.0f;

		dd.begin(DU_DRAW_QUADS);
		const dtProximityGrid* grid = crowd->getGrid();
		const int* bounds = grid->getBounds();
		const float cs = grid->getCellSize();
		for (int y = bounds[1]; y <= bounds[3]; ++y)
		{
			for (int x = bounds[0]; x <= bounds[2]; ++x)
			{
				const int count = grid->getItemCountAt(x,y); 
				if (!count) continue;
				unsigned int col = duRGBA(128,0,0,dtMin(count*40,255));
				dd.vertex(x*cs, gridy, y*cs, col);
				dd.vertex(x*cs, gridy, y*cs+cs, col);
				dd.vertex(x*cs+cs, gridy, y*cs+cs, col);
				dd.vertex(x*cs+cs, gridy, y*cs, col);
			}
		}
		dd.end();
	}

#if 0
	// Trail
	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		const dtCrowdAgent* ag = crowd->getAgent(i);
		if (!ag->active) continue;

		const AgentTrail* trail = &m_trails[i];
		const float* pos = ag->npos;

		dd.begin(DU_DRAW_LINES,3.0f);
		float prev[3], preva = 1;
		dtVcopy(prev, pos);
		for (int j = 0; j < AGENT_MAX_TRAIL-1; ++j)
		{
			const int idx = (trail->htrail + AGENT_MAX_TRAIL-j) % AGENT_MAX_TRAIL;
			const float* v = &trail->trail[idx*3];
			float a = 1 - j/(float)AGENT_MAX_TRAIL;
			dd.vertex(prev[0],prev[1]+0.1f,prev[2], duRGBA(0,0,0,(int)(128*preva)));
			dd.vertex(v[0],v[1]+0.1f,v[2], duRGBA(0,0,0,(int)(128*a)));
			preva = a;
			dtVcopy(prev, v);
		}
		dd.end();

	}
#endif

	// Corners & co
	for (int i = 0; i < crowd->getAgentCount(); i++)
	{
// 		if (m_toolParams.m_showDetailAll == false && i != m_agentDebug.idx)
// 			continue;
		const dtCrowdAgent* ag =crowd->getAgent(i);
		if (!ag->active)
			continue;

		const float radius = ag->params.radius;
		const float* pos = ag->npos;

		if (m_showCorners)
		{
			if (ag->ncorners)
			{
				dd.begin(DU_DRAW_LINES, 2.0f);
				for (int j = 0; j < ag->ncorners; ++j)
				{
					const float* va = j == 0 ? pos : &ag->cornerVerts[(j-1)*3];
					const float* vb = &ag->cornerVerts[j*3];
					dd.vertex(va[0],va[1]+radius,va[2], duRGBA(128,0,0,192));
					dd.vertex(vb[0],vb[1]+radius,vb[2], duRGBA(128,0,0,192));
				}
				if (ag->ncorners && ag->cornerFlags[ag->ncorners-1] & DT_STRAIGHTPATH_OFFMESH_CONNECTION)
				{
					const float* v = &ag->cornerVerts[(ag->ncorners-1)*3];
					dd.vertex(v[0],v[1],v[2], duRGBA(192,0,0,192));
					dd.vertex(v[0],v[1]+radius*2,v[2], duRGBA(192,0,0,192));
				}

				dd.end();


				if (m_anticipateTurns)
				{
					/*					float dvel[3], pos[3];
					calcSmoothSteerDirection(ag->pos, ag->cornerVerts, ag->ncorners, dvel);
					pos[0] = ag->pos[0] + dvel[0];
					pos[1] = ag->pos[1] + dvel[1];
					pos[2] = ag->pos[2] + dvel[2];

					const float off = ag->radius+0.1f;
					const float* tgt = &ag->cornerVerts[0];
					const float y = ag->pos[1]+off;

					dd.begin(DU_DRAW_LINES, 2.0f);

					dd.vertex(ag->pos[0],y,ag->pos[2], duRGBA(255,0,0,192));
					dd.vertex(pos[0],y,pos[2], duRGBA(255,0,0,192));

					dd.vertex(pos[0],y,pos[2], duRGBA(255,0,0,192));
					dd.vertex(tgt[0],y,tgt[2], duRGBA(255,0,0,192));

					dd.end();*/
				}
			}
		}

		if (m_showCollisionSegments)
		{
			const float* center = ag->boundary.getCenter();
			duDebugDrawCross(&dd, center[0],center[1]+radius,center[2], 0.2f, duRGBA(192,0,128,255), 2.0f);
			duDebugDrawCircle(&dd, center[0],center[1]+radius,center[2], ag->params.collisionQueryRange,
				duRGBA(192,0,128,128), 2.0f);

			dd.begin(DU_DRAW_LINES, 3.0f);
			for (int j = 0; j < ag->boundary.getSegmentCount(); ++j)
			{
				const float* s = ag->boundary.getSegment(j);
				unsigned int col = duRGBA(192,0,128,192);
				if (dtTriArea2D(pos, s, s+3) < 0.0f)
					col = duDarkenCol(col);

				duAppendArrow(&dd, s[0],s[1]+0.2f,s[2], s[3],s[4]+0.2f,s[5], 0.0f, 0.3f, col);
			}
			dd.end();
		}

		if (m_showNeis)
		{
			duDebugDrawCircle(&dd, pos[0],pos[1]+radius,pos[2], ag->params.collisionQueryRange,
				duRGBA(0,192,128,128), 2.0f);

			dd.begin(DU_DRAW_LINES, 2.0f);
			for (int j = 0; j < ag->nneis; ++j)
			{
				// Get 'n'th active agent.
				// TODO: fix this properly.
				const dtCrowdAgent* nei = crowd->getAgent(ag->neis[j].idx);
				if (nei)
				{
					dd.vertex(pos[0],pos[1]+radius,pos[2], duRGBA(0,192,128,128));
					dd.vertex(nei->npos[0],nei->npos[1]+radius,nei->npos[2], duRGBA(0,192,128,128));
				}
			}
			dd.end();
		}

// 		if (m_toolParams.m_showOpt)
// 		{
// 			dd.begin(DU_DRAW_LINES, 2.0f);
// 			dd.vertex(m_agentDebug.optStart[0],m_agentDebug.optStart[1]+0.3f,m_agentDebug.optStart[2], duRGBA(0,128,0,192));
// 			dd.vertex(m_agentDebug.optEnd[0],m_agentDebug.optEnd[1]+0.3f,m_agentDebug.optEnd[2], duRGBA(0,128,0,192));
// 			dd.end();
// 		}
	}

	// Agent cylinders.
	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		const dtCrowdAgent* ag = crowd->getAgent(i);
		if (!ag->active) continue;

		const float radius = ag->params.radius;
		const float* pos = ag->npos;

		unsigned int col = duRGBA(0,0,0,32);
// 		if (m_agentDebug.idx == i)
// 			col = duRGBA(255,0,0,128);

		duDebugDrawCircle(&dd, pos[0], pos[1], pos[2], radius, col, 2.0f);
	}

	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		const dtCrowdAgent* ag = crowd->getAgent(i);
		if (!ag->active) continue;

		const float height = ag->params.height;
		const float radius = ag->params.radius;
		const float* pos = ag->npos;

		unsigned int col = duRGBA(220,220,220,128);
		if (ag->targetState == DT_CROWDAGENT_TARGET_REQUESTING || ag->targetState == DT_CROWDAGENT_TARGET_WAITING_FOR_QUEUE)
			col = duLerpCol(col, duRGBA(128,0,255,128), 32);
		else if (ag->targetState == DT_CROWDAGENT_TARGET_WAITING_FOR_PATH)
			col = duLerpCol(col, duRGBA(128,0,255,128), 128);
		else if (ag->targetState == DT_CROWDAGENT_TARGET_FAILED)
			col = duRGBA(255,32,16,128);
		else if (ag->targetState == DT_CROWDAGENT_TARGET_VELOCITY)
			col = duLerpCol(col, duRGBA(64,255,0,128), 128);

		duDebugDrawCylinder(&dd, pos[0]-radius, pos[1]+radius*0.1f, pos[2]-radius,
			pos[0]+radius, pos[1]+height, pos[2]+radius, col);
	}


	if (m_showVO)
	{
		for (int i = 0; i < crowd->getAgentCount(); i++)
		{
// 			if (m_toolParams.m_showDetailAll == false && i != m_agentDebug.idx)
// 				continue;
			const dtCrowdAgent* ag =crowd->getAgent(i);
			if (!ag->active)
				continue;

			// Draw detail about agent sela
			const dtObstacleAvoidanceDebugData* vod = NULL;//m_agentDebug.vod;

			const float dx = ag->npos[0];
			const float dy = ag->npos[1]+ag->params.height;
			const float dz = ag->npos[2];

			duDebugDrawCircle(&dd, dx,dy,dz, ag->params.maxSpeed, duRGBA(255,255,255,64), 2.0f);

			dd.begin(DU_DRAW_QUADS);
			for (int j = 0; j < vod->getSampleCount(); ++j)
			{
				const float* p = vod->getSampleVelocity(j);
				const float sr = vod->getSampleSize(j);
				const float pen = vod->getSamplePenalty(j);
				const float pen2 = vod->getSamplePreferredSidePenalty(j);
				unsigned int col = duLerpCol(duRGBA(255,255,255,220), duRGBA(128,96,0,220), (int)(pen*255));
				col = duLerpCol(col, duRGBA(128,0,0,220), (int)(pen2*128));
				dd.vertex(dx+p[0]-sr, dy, dz+p[2]-sr, col);
				dd.vertex(dx+p[0]-sr, dy, dz+p[2]+sr, col);
				dd.vertex(dx+p[0]+sr, dy, dz+p[2]+sr, col);
				dd.vertex(dx+p[0]+sr, dy, dz+p[2]-sr, col);
			}
			dd.end();
		}
	}

	// Velocity stuff.
	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		const dtCrowdAgent* ag = crowd->getAgent(i);
		if (!ag->active) continue;

		const float radius = ag->params.radius;
		const float height = ag->params.height;
		const float* pos = ag->npos;
		const float* vel = ag->vel;
		const float* dvel = ag->dvel;

		unsigned int col = duRGBA(220,220,220,192);
		if (ag->targetState == DT_CROWDAGENT_TARGET_REQUESTING || ag->targetState == DT_CROWDAGENT_TARGET_WAITING_FOR_QUEUE)
			col = duLerpCol(col, duRGBA(128,0,255,192), 32);
		else if (ag->targetState == DT_CROWDAGENT_TARGET_WAITING_FOR_PATH)
			col = duLerpCol(col, duRGBA(128,0,255,192), 128);
		else if (ag->targetState == DT_CROWDAGENT_TARGET_FAILED)
			col = duRGBA(255,32,16,192);
		else if (ag->targetState == DT_CROWDAGENT_TARGET_VELOCITY)
			col = duLerpCol(col, duRGBA(64,255,0,192), 128);

		duDebugDrawCircle(&dd, pos[0], pos[1]+height, pos[2], radius, col, 2.0f);

		duDebugDrawArrow(&dd, pos[0],pos[1]+height,pos[2],
			pos[0]+dvel[0],pos[1]+height+dvel[1],pos[2]+dvel[2],
			0.0f, 0.4f, duRGBA(0,192,255,192), /*(m_agentDebug.idx == i) ? 2.0f : */1.0f);

		duDebugDrawArrow(&dd, pos[0],pos[1]+height,pos[2],
			pos[0]+vel[0],pos[1]+height+vel[1],pos[2]+vel[2],
			0.0f, 0.4f, duRGBA(0,0,0,160), 2.0f);
	}

	dd.depthMask(true);
}

void VisualDebugSDLImpl::ReloadInputGeom( const char* pathfile )
{
// 	if(m_inputGeom)
// 		delete m_inputGeom;

	//m_inputGeom = new InputGeom;
	//m_inputGeom->loadMesh(&m_rcContext, pathfile);
}

void VisualDebugSDLImpl::SetInputGeoMetry( InputTerrainGeom* geo, const BuildParamter& param )
{
	if( !geo ) return;

	m_inputGeom = geo;

	memcpy(&m_buildParamter, &param, sizeof(BuildParamter));

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

void VisualDebugSDLImpl::AllocDebugDraw(duDebugDrawItem** out)
{
	(*out) = new duDisplayItem();
}