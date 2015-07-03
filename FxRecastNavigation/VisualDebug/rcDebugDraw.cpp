//------------------------------------------------------------------------------
// 文件名:		rcDebugDraw.cpp
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月16日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#include "rcDebugDraw.h"
#include "DetourDebugDraw.h"
#include "DetourCommon.h"
#include <float.h>

rcDebugDraw::rcDebugDraw(void):m_showPath(false)
,m_showGrid(false)
,m_showVO(false)
,m_showCorners(false)
,m_anticipateTurns(false)
,m_showNeis(false)
,m_showNodes(false)
,m_showCollisionSegments(false)
,m_showTiles(false)
,m_navMeshDrawFlags(DU_DRAWNAVMESH_OFFMESHCONS|DU_DRAWNAVMESH_OFFMESHCONS)
{
}

rcDebugDraw::~rcDebugDraw(void)
{
}

void rcDebugDraw::Draw( float fSeconds, duDebugDraw* dd,
					   dtNavMesh*navMesh, 
					   dtNavMeshQuery* navQuery, 
					   dtCrowd* navCrowd, 
					   dtTileCache* tileCache, 
					   const float* boxmin, const float* boxmax,
					   const BuildParamter* parm )
{

	if( boxmax && boxmin && parm)
		drawBoxAndGrid(dd, boxmin, boxmax, *parm);


	if( navMesh && navQuery)
		duDebugDrawNavMeshWithClosedList(dd, *navMesh, *navQuery, m_navMeshDrawFlags);
	if( navMesh )
		duDebugDrawNavMeshBVTree(dd, *navMesh);
	if( navQuery )
		duDebugDrawNavMeshNodes(dd, *navQuery);
	if( navMesh )
		duDebugDrawNavMeshPolysWithFlags(dd, *navMesh, SAMPLE_POLYFLAGS_DISABLED, duRGBA(0,0,0,128));

	if(navMesh && navCrowd)
		drawCrowd(dd, fSeconds, navCrowd, navMesh);

	if (tileCache && m_showTiles)
		drawTiles(dd, tileCache);


	if (tileCache )
		drawObstacles(dd, tileCache);
}

void rcDebugDraw::drawBoxAndGrid(duDebugDraw*dd, const float* bmin, const float* bmax, const BuildParamter& buildParam)
{
	duDebugDrawBoxWire(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], duRGBA(255,255,255,128), 1.0f);


	// Tiling grid.
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, buildParam.m_cellSize, &gw, &gh);
	const int tw = (gw + (int)buildParam.m_tileSize-1) / (int)buildParam.m_tileSize;
	const int th = (gh + (int)buildParam.m_tileSize-1) / (int)buildParam.m_tileSize;
	const float s = buildParam.m_tileSize*buildParam.m_cellSize;
	duDebugDrawGridXZ(dd, bmin[0],bmin[1],bmin[2], tw,th, s, duRGBA(0,0,0,64), 1.0f);
}

void rcDebugDraw::drawTiles(duDebugDraw* dd, dtTileCache* tc)
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


void rcDebugDraw::drawObstacles(duDebugDraw* dd, const dtTileCache* tc)
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
			//unsigned int colors[6] = {col,col,col,col,col,col};
			//duDebugDrawBox(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], colors);
			//duDebugDrawBoxWire(dd, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], duDarkenCol(col), 2);
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



void rcDebugDraw::drawCrowd(duDebugDraw* dd, float fSeconds, dtCrowd* crowd, dtNavMesh* nav)
{

	if (!nav || !crowd)
		return;

	if (m_showNodes && crowd->getPathQueue())
	{
		const dtNavMeshQuery* navquery = crowd->getPathQueue()->getNavQuery();
		if (navquery)
			duDebugDrawNavMeshNodes(dd, *navquery);
	}

	dd->depthMask(false);

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
				duDebugDrawNavMeshPoly(dd, *nav, path[j], duRGBA(255,255,255,24));
		}
	}

	// 	if (m_targetRef)
	// 		duDebugDrawCross(dd, m_targetPos[0],m_targetPos[1]+0.1f,m_targetPos[2], rad, duRGBA(255,255,255,192), 2.0f);

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

		dd->begin(DU_DRAW_QUADS);
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
				dd->vertex(x*cs, gridy, y*cs, col);
				dd->vertex(x*cs, gridy, y*cs+cs, col);
				dd->vertex(x*cs+cs, gridy, y*cs+cs, col);
				dd->vertex(x*cs+cs, gridy, y*cs, col);
			}
		}
		dd->end();
	}

#if 0
	// Trail
	for (int i = 0; i < crowd->getAgentCount(); ++i)
	{
		const dtCrowdAgent* ag = crowd->getAgent(i);
		if (!ag->active) continue;

		const AgentTrail* trail = &m_trails[i];
		const float* pos = ag->npos;

		dd->begin(DU_DRAW_LINES,3.0f);
		float prev[3], preva = 1;
		dtVcopy(prev, pos);
		for (int j = 0; j < AGENT_MAX_TRAIL-1; ++j)
		{
			const int idx = (trail->htrail + AGENT_MAX_TRAIL-j) % AGENT_MAX_TRAIL;
			const float* v = &trail->trail[idx*3];
			float a = 1 - j/(float)AGENT_MAX_TRAIL;
			dd->vertex(prev[0],prev[1]+0.1f,prev[2], duRGBA(0,0,0,(int)(128*preva)));
			dd->vertex(v[0],v[1]+0.1f,v[2], duRGBA(0,0,0,(int)(128*a)));
			preva = a;
			dtVcopy(prev, v);
		}
		dd->end();

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
				dd->begin(DU_DRAW_LINES, 2.0f);
				for (int j = 0; j < ag->ncorners; ++j)
				{
					const float* va = j == 0 ? pos : &ag->cornerVerts[(j-1)*3];
					const float* vb = &ag->cornerVerts[j*3];
					dd->vertex(va[0],va[1]+radius,va[2], duRGBA(128,0,0,192));
					dd->vertex(vb[0],vb[1]+radius,vb[2], duRGBA(128,0,0,192));
				}
				if (ag->ncorners && ag->cornerFlags[ag->ncorners-1] & DT_STRAIGHTPATH_OFFMESH_CONNECTION)
				{
					const float* v = &ag->cornerVerts[(ag->ncorners-1)*3];
					dd->vertex(v[0],v[1],v[2], duRGBA(192,0,0,192));
					dd->vertex(v[0],v[1]+radius*2,v[2], duRGBA(192,0,0,192));
				}

				dd->end();


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

					dd->begin(DU_DRAW_LINES, 2.0f);

					dd->vertex(ag->pos[0],y,ag->pos[2], duRGBA(255,0,0,192));
					dd->vertex(pos[0],y,pos[2], duRGBA(255,0,0,192));

					dd->vertex(pos[0],y,pos[2], duRGBA(255,0,0,192));
					dd->vertex(tgt[0],y,tgt[2], duRGBA(255,0,0,192));

					dd->end();*/
				}
			}
		}

		if (m_showCollisionSegments)
		{
			const float* center = ag->boundary.getCenter();
			duDebugDrawCross(dd, center[0],center[1]+radius,center[2], 0.2f, duRGBA(192,0,128,255), 2.0f);
			duDebugDrawCircle(dd, center[0],center[1]+radius,center[2], ag->params.collisionQueryRange,
				duRGBA(192,0,128,128), 2.0f);

			dd->begin(DU_DRAW_LINES, 3.0f);
			for (int j = 0; j < ag->boundary.getSegmentCount(); ++j)
			{
				const float* s = ag->boundary.getSegment(j);
				unsigned int col = duRGBA(192,0,128,192);
				if (dtTriArea2D(pos, s, s+3) < 0.0f)
					col = duDarkenCol(col);

				duAppendArrow(dd, s[0],s[1]+0.2f,s[2], s[3],s[4]+0.2f,s[5], 0.0f, 0.3f, col);
			}
			dd->end();
		}

		if (m_showNeis)
		{
			duDebugDrawCircle(dd, pos[0],pos[1]+radius,pos[2], ag->params.collisionQueryRange,
				duRGBA(0,192,128,128), 2.0f);

			dd->begin(DU_DRAW_LINES, 2.0f);
			for (int j = 0; j < ag->nneis; ++j)
			{
				// Get 'n'th active agent.
				// TODO: fix this properly.
				const dtCrowdAgent* nei = crowd->getAgent(ag->neis[j].idx);
				if (nei)
				{
					dd->vertex(pos[0],pos[1]+radius,pos[2], duRGBA(0,192,128,128));
					dd->vertex(nei->npos[0],nei->npos[1]+radius,nei->npos[2], duRGBA(0,192,128,128));
				}
			}
			dd->end();
		}

		// 		if (m_toolParams.m_showOpt)
		// 		{
		// 			dd->begin(DU_DRAW_LINES, 2.0f);
		// 			dd->vertex(m_agentDebug.optStart[0],m_agentDebug.optStart[1]+0.3f,m_agentDebug.optStart[2], duRGBA(0,128,0,192));
		// 			dd->vertex(m_agentDebug.optEnd[0],m_agentDebug.optEnd[1]+0.3f,m_agentDebug.optEnd[2], duRGBA(0,128,0,192));
		// 			dd->end();
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

		duDebugDrawCircle(dd, pos[0], pos[1], pos[2], radius, col, 2.0f);
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

		duDebugDrawCylinder(dd, pos[0]-radius, pos[1]+radius*0.1f, pos[2]-radius,
			pos[0]+radius, pos[1]+height, pos[2]+radius, col);
	}

	bool m_showTargetPos = true;
	if (m_showTargetPos)
	{
		for (int i = 0; i < crowd->getAgentCount(); ++i)
		{
			const dtCrowdAgent* ag = crowd->getAgent(i);
			if (!ag->active) continue;
			if(ag->targetState==DT_CROWDAGENT_TARGET_NONE) continue;
			if(ag->targetState==DT_CROWDAGENT_TARGET_VELOCITY) continue;
			const float height = ag->params.height;
			const float radius = ag->params.radius;
			const float* pos = ag->npos;
			const float* targetPos = ag->targetPos;
			dd->begin(DU_DRAW_LINES, 2.0);
			dd->vertex(pos, duRGBA(255,255,0, 128));
			dd->vertex(targetPos, duRGBA(255,255,0, 128));
			dd->end();
		}
	}



	if (m_showVO && false)
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

			duDebugDrawCircle(dd, dx,dy,dz, ag->params.maxSpeed, duRGBA(255,255,255,64), 2.0f);

			dd->begin(DU_DRAW_QUADS);
			for (int j = 0; j < vod->getSampleCount(); ++j)
			{
				const float* p = vod->getSampleVelocity(j);
				const float sr = vod->getSampleSize(j);
				const float pen = vod->getSamplePenalty(j);
				const float pen2 = vod->getSamplePreferredSidePenalty(j);
				unsigned int col = duLerpCol(duRGBA(255,255,255,220), duRGBA(128,96,0,220), (int)(pen*255));
				col = duLerpCol(col, duRGBA(128,0,0,220), (int)(pen2*128));
				dd->vertex(dx+p[0]-sr, dy, dz+p[2]-sr, col);
				dd->vertex(dx+p[0]-sr, dy, dz+p[2]+sr, col);
				dd->vertex(dx+p[0]+sr, dy, dz+p[2]+sr, col);
				dd->vertex(dx+p[0]+sr, dy, dz+p[2]-sr, col);
			}
			dd->end();
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

		duDebugDrawCircle(dd, pos[0], pos[1]+height, pos[2], radius, col, 2.0f);

		duDebugDrawArrow(dd, pos[0],pos[1]+height,pos[2],
			pos[0]+dvel[0],pos[1]+height+dvel[1],pos[2]+dvel[2],
			0.0f, 0.4f, duRGBA(0,192,255,192), 1.0f);

		duDebugDrawArrow(dd, pos[0],pos[1]+height/2,pos[2],
			pos[0]+vel[0],pos[1]+height/2+vel[1],pos[2]+vel[2],
			0.0f, 0.4f, duRGBA(0,0,0,160), 2.0f);
	}

	dd->depthMask(true);
}
