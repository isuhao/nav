//------------------------------------------------------------------------------
// 文件名:		rcDebugDraw.h
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月16日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#include "../RecastNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "DetourCrowd.h"
#include "DebugDraw.h"
#pragma once

class rcDebugDraw
{
public:
	rcDebugDraw(void);
	~rcDebugDraw(void);

	void Draw( float fSeconds, duDebugDraw* dd,
		dtNavMesh*navMesh, 
		dtNavMeshQuery* navQuery, 
		dtCrowd* navCrowd, 
		dtTileCache* tileCache, 
		const float* boxmin, const float* boxmax,
		const BuildParamter* parm );

	bool			m_showPath;
	bool			m_showGrid;
	bool			m_showVO;
	bool			m_showCorners;
	bool			m_anticipateTurns;
	bool			m_showNeis;
	bool			m_showNodes;
	bool			m_showCollisionSegments;
	bool			m_showTiles;
	unsigned char	m_navMeshDrawFlags;

protected:
	void drawTiles(duDebugDraw* dd, dtTileCache* tc);
	void drawObstacles(duDebugDraw* dd, const dtTileCache* tc);
	void drawCrowd(duDebugDraw* dd, float fSeconds, dtCrowd* crowd, dtNavMesh* nav);
	void drawBoxAndGrid(duDebugDraw*dd, const float* bmin, const float* bmax, const BuildParamter& buildParam);
};
