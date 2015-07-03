//------------------------------------------------------------------------------
// 文件名:		VisualDebugSDLImpl.h
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月6日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------

#include "DetourNavMesh.h"
#include "DetourCrowd.h"
#include "DetourNavMeshQuery.h"
#include "Recast.h"
#include "DetourTileCache.h"
#include "../RecastNavMeshBuilder.h"
#include "../VisualDebug.h"
#include <Windows.h>
#pragma once

struct duDebugDraw;
class InputTerrainGeom;
class VisualDebugSDLImpl : public VisualDebug
{
public:
	VisualDebugSDLImpl(void);
	~VisualDebugSDLImpl(void);

	virtual bool	Initialize();
	virtual bool	IsInitialized()const { return m_isInitialized; }
	virtual void	Update(float fSeconds, dtNavMesh*, dtNavMeshQuery*, dtCrowd*, dtTileCache*);
	virtual void	SetInputGeoMetry(InputTerrainGeom* geo, const BuildParamter& param);
	virtual void	ReloadInputGeom(const char* pathfile);

private:
	bool			BeginRender();
	void			EndRender();
	void			Release();
	void			ProcessInput(float fSeconds);
	void			RenderGUI(float fSeconds);
	void			RenderCrowd(float fSeconds, dtCrowd* crowd, dtNavMesh* nav);
	void			DrawTiles(duDebugDraw* dd, dtTileCache* tc);
	void			drawObstacles(duDebugDraw* dd, const dtTileCache* tc);
private:

	bool			m_isInitialized;
	int				m_viewWidth;
	int				m_viewHeight;
	unsigned char	m_navMeshDrawFlags;
	bool			presentationMode;
	bool			showMenu;
	bool			m_showPath;
	bool			m_showGrid;
	bool			m_showVO;
	bool			m_showCorners;
	bool			m_anticipateTurns;
	bool			m_showNeis;
	bool			m_showNodes;
	bool			m_showCollisionSegments;
	bool			m_bDrawTiles;
	bool			m_bDrawGeometry;

	float			m_mouseHitpos[3];
	HWND			m_hRenderWnd;
	rcContext		m_rcContext;

	BuildParamter	m_buildParamter;


	InputTerrainGeom*		m_inputGeom;

	float m_cameraPos[3], camr;
	float rx;
	float ry;
	bool m_bMousePosSet;



};
