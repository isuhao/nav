//------------------------------------------------------------------------------
// �ļ���:		rcDebugDraw.h
// ��  ��:		
// ˵  ��:		
// ��������:	2014��5��16��
// ������:		����־
// ��ע: 
// ��Ȩ����:	������ţ�������޹�˾
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
