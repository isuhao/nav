//------------------------------------------------------------------------------
// 文件名:		RecastNavigationManager.cpp
// 内  容:		recast navigation 
// 说  明:		recast navigation 
// 创建日期:	2014年4月21日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#include "RecastNavigationManager.h"
#include "VisualDebug.h"
#include "InputTerrainGeom.h"
#include <math.h>
//#include <vld.h>

#ifdef RC_VSDBG_ENBALE
#include "VisualDebug/VisualDebugSDLImpl.h"
#include "VisualDebug/VisualDebugOenGLImpl.h"
#pragma comment(lib, "FxRecastNavVisualDebug.lib")
#endif




static const int NAVMESHSET_MAGIC = 'M'<<24 | 'S'<<16 | 'E'<<8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;

static const float VECTOR_ZERO[] = {0.f, 0.f, 0.f};

const char AGENT_OBSTACLEAVOIDANCETYPE = 3;
static const float POLYPICK_EXT[3] = {2,4,2};
static unsigned char	sg_instanceID;
static const int EXPECTED_LAYERS_PER_TILE = 4;


class BotAgentObserv : public IBotAgentObserver
{
public:
	BotAgentObserv(RecastNavigationManager* mgr):m_manager(mgr){}
	virtual void	OnBotPathQueryDone(rcBotAgent*, const float*nodeBuff, long nNodeCount);
	virtual void	OnBotSpeedChange(rcBotAgent*, const float* curSpeed, const float* preSpeed,const float* desrDest);
	virtual void	OnBotArrive(rcBotAgent*, const float* curPos, const float* destPos);
private:
	RecastNavigationManager* m_manager;
};

void BotAgentObserv::OnBotPathQueryDone(rcBotAgent*, const float*nodeBuff, long nNodeCount)
{

}

void BotAgentObserv::OnBotSpeedChange(rcBotAgent*, const float* curSpeed, const float* preSpeed,const float* desrDest)
{

}

void BotAgentObserv::OnBotArrive(rcBotAgent*bot, const float* curPos, const float* destPos)
{
	if(m_manager->m_observer)
	{
		NaviVector3 curPosInClient, destPosInClient;
		m_manager->m_coordSys.RecastNavToClientPos(curPos, (float*)&curPosInClient);
		m_manager->m_coordSys.RecastNavToClientPos(destPos, (float*)&destPosInClient);
		m_manager->m_observer->OnBotArrive(bot->GetName(), curPosInClient, destPosInClient);
	}
}


bool NaviWorldInitConfig::AddNaviDataPath(const char* szPathFile)
{
	if(m_naviDataCount>=NAVIDATA_COUNT)
		return false;
	const char* szExtName = "";
	m_strNaviData[m_naviDataCount] = new char[strlen(szPathFile)+strlen(szExtName)+1];
	strcpy(m_strNaviData[m_naviDataCount], szPathFile);
	strcat(m_strNaviData[m_naviDataCount], szExtName);
	//#endif
	++m_naviDataCount;

	return true;
}

void NaviWorldInitConfig::Clear()
{
	for (int i=0; i<m_naviDataCount; ++i)
	{
		delete m_strNaviData[i];
		m_strNaviData[i] = 0;
	}
	m_naviDataCount = 0;
}

void RecastNavigationManager::World::Destroy()
{
	if(m_navMesh) dtFreeNavMesh(m_navMesh);
	m_navMesh = 0;

	if(m_navCrowd) dtFreeCrowd(m_navCrowd);
	m_navCrowd = 0;

	if(m_navQuery) dtFreeNavMeshQuery(m_navQuery);
	m_navQuery = 0;

	if(m_tileCache) dtFreeTileCache(m_tileCache);
	m_tileCache = 0;

	if(m_geom) delete m_geom;
	m_geom = 0;

	if(m_tmproc) delete m_tmproc;
	m_tmproc = 0;
}

RecastNavigationManager::RecastNavigationManager(void)
:m_IsInitialize(false)
,m_pFileIO(0)
,m_observer(0)
,m_instanceID(sg_instanceID++)
,m_pBotAgentObserver(0)
//,m_navMeshDrawFlags(DU_DRAWNAVMESH_OFFMESHCONS|DU_DRAWNAVMESH_CLOSEDLIST)
{
	m_buildParamater.m_tileSize = 48.0f;
	m_buildParamater.m_cellSize = 0.3f;
	m_buildParamater.m_cellHeight = 0.2f;
	m_buildParamater.m_agentHeight = 2.0f;
	m_buildParamater.m_agentRadius = 0.3f;
	m_buildParamater.m_agentMaxClimb = 0.9f;
	m_buildParamater.m_agentMaxSlope = 45.0f;
	m_buildParamater.m_regionMinSize = 8;
	m_buildParamater.m_regionMergeSize = 20;
	m_buildParamater.m_monotonePartitioning = false;
	m_buildParamater.m_edgeMaxLen = 12.0f;
	m_buildParamater.m_edgeMaxError = 1.3f;
	m_buildParamater.m_vertsPerPoly = 6.0f;
	m_buildParamater.m_detailSampleDist = 6.0f;
	m_buildParamater.m_detailSampleMaxError = 1.0f;

	m_queryPointExtents[0] = m_buildParamater.m_agentRadius*2;
	m_queryPointExtents[1] = m_buildParamater.m_agentRadius*3;
	m_queryPointExtents[2] = m_buildParamater.m_agentRadius*2;
}

RecastNavigationManager::~RecastNavigationManager(void)
{
	if(m_talloc ) delete m_talloc;
	if(m_tcomp ) delete m_tcomp;
	if(m_ctx ) delete m_ctx;
	if(m_pBotAgentObserver) delete m_pBotAgentObserver;
#ifdef RC_VSDBG_ENBALE
	if(m_visualDebug) delete m_visualDebug;
#endif

	m_talloc = NULL;
	m_tcomp = NULL;
	m_ctx = NULL;
	m_visualDebug = NULL;
}

bool RecastNavigationManager::Initialize(const NaviMgrInitConfig& conf)
{
	m_bVisualDebug	= conf.m_bVisualDebug;
	m_observer		= conf.m_pObserver;
	m_talloc		= new LinearAllocator(32000);
	m_tcomp			= new FastLZCompressor;
	m_ctx			= new rcContext;//new BuildContext;
	m_pBotAgentObserver = new BotAgentObserv(this);

#ifdef RC_VSDBG_ENBALE
	m_visualDebug	= new VisualDebugOenGLImpl;//VisualDebugSDLImpl;
	if( m_bVisualDebug )
	{
		m_visualDebug->Create();
		m_visualDebug->Show(true);
	}
#endif

	m_IsInitialize	= true;
	m_pFileIO		= conf.m_pFileIO;
#if _WIN32
	if (m_pFileIO==NULL)
	{
		m_pFileIO = &m_DefaultFileio;
	}
#endif

	return true;
}
bool RecastNavigationManager::IsInitialized()
{
	return m_IsInitialize;
}

bool RecastNavigationManager::HasWorld( const char* name ) 
{
	return (GetWorld(name)!=0);
}

bool RecastNavigationManager::CreateWorld(const char* name, const NaviWorldInitConfig& conf)
{
	DestroyWorld(name);

	World newWorld;

	bool bSuccess = false;
	do 
	{
		const BuildParamter& buildParam = m_buildParamater;

		if(!m_pFileIO->openForRead(conf.GetNaviDataPath(0)))
			break;
		GeoMetryHeader header;
		m_pFileIO->read((char*)&header, sizeof(GeoMetryHeader));
		m_pFileIO->seek(header.Start, 0);
		float* vertsBuff = new float[header.nVertexCount*3];
		m_pFileIO->read((char*)vertsBuff, header.nVertexCount*3*sizeof(float));
		newWorld.m_geom = new InputTerrainGeom;
		newWorld.m_geom->loadMesh(m_ctx,vertsBuff, header.nVertexCount*3);
		delete vertsBuff;
		m_pFileIO->close();

		newWorld.m_navMesh = dtAllocNavMesh();
		if( !newWorld.m_navMesh ) break;

		newWorld.m_navQuery = dtAllocNavMeshQuery();
		if( !newWorld.m_navQuery ) break;

		newWorld.m_navCrowd = dtAllocCrowd();
		if( !newWorld.m_navCrowd ) break;


		newWorld.m_tileCache = dtAllocTileCache();
		if( !newWorld.m_tileCache ) break;


		if( !GeneratTileCache(newWorld) )
			break;

		if( !newWorld.m_navCrowd->init(conf.m_nMaxBotCount, 
			buildParam.m_agentRadius, newWorld.m_navMesh) )
			break;

		if( dtStatusFailed(newWorld.m_navQuery->init(newWorld.m_navMesh, 1000)) )
			break;

		bSuccess = true;

	} while (false);

	if ( !bSuccess )
	{
		newWorld.Destroy();
		return false;
	}

	World& world = m_worldContiner[name];
	world.m_navMesh		= newWorld.m_navMesh;
	world.m_navQuery	= newWorld.m_navQuery;
	world.m_navCrowd	= newWorld.m_navCrowd;
	world.m_tileCache	= newWorld.m_tileCache;
	world.m_tmproc		= newWorld.m_tmproc;
	world.m_geom		= newWorld.m_geom;

#ifdef RC_VSDBG_ENBALE
	if(m_visualDebug)
		m_visualDebug->AddShowContent(name, 
		newWorld.m_navMesh,
		newWorld.m_navQuery,
		newWorld.m_navCrowd, 
		newWorld.m_tileCache, 
		newWorld.m_geom,
		&m_buildParamater);
#endif

	return true;
}
bool RecastNavigationManager::GeneratTileCache(World& world)
{
	dtStatus status = 0;

	const BuildParamter& buildParam = m_buildParamater;


	int gridSize = 1;
	int m_maxPolysPerTile = 0;
	int m_maxTiles = 0;
	if (world.m_geom)
	{
		const float* bmin = world.m_geom->getMeshBoundsMin();
		const float* bmax = world.m_geom->getMeshBoundsMax();
		char text[64];
		int gw = 0, gh = 0;
		rcCalcGridSize(bmin, bmax, buildParam.m_cellSize, &gw, &gh);
		const int ts = (int)buildParam.m_tileSize;
		const int tw = (gw + ts-1) / ts;
		const int th = (gh + ts-1) / ts;

		// Max tiles and max polys affect how the tile IDs are caculated.
		// There are 22 bits available for identifying a tile and a polygon.
		int tileBits = rcMin((int)dtIlog2(dtNextPow2(tw*th*EXPECTED_LAYERS_PER_TILE)), 14);
		if (tileBits > 14) tileBits = 14;
		int polyBits = 22 - tileBits;
		m_maxTiles = 1 << tileBits;
		m_maxPolysPerTile = 1 << polyBits;
		gridSize = tw*th;
	}

	if( !world.m_tmproc )
		world.m_tmproc = new MeshProcess;
	world.m_tmproc->init(world.m_geom);

	// Init cache
	const float* bmin = world.m_geom->getMeshBoundsMin();
	const float* bmax = world.m_geom->getMeshBoundsMax();
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, buildParam.m_cellSize, &gw, &gh);
	const int ts = (int)buildParam.m_tileSize;
	const int tw = (gw + ts-1) / ts;
	const int th = (gh + ts-1) / ts;

	// Generation params.
	rcConfig cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.cs = buildParam.m_cellSize;
	cfg.ch = buildParam.m_cellHeight;
	cfg.walkableSlopeAngle = buildParam.m_agentMaxSlope;
	cfg.walkableHeight = (int)ceilf(buildParam.m_agentHeight / cfg.ch);
	cfg.walkableClimb = (int)floorf(buildParam.m_agentMaxClimb / cfg.ch);
	cfg.walkableRadius = (int)ceilf(buildParam.m_agentRadius / cfg.cs);
	cfg.maxEdgeLen = (int)(buildParam.m_edgeMaxLen / buildParam.m_cellSize);
	cfg.maxSimplificationError = buildParam.m_edgeMaxError;
	cfg.minRegionArea = (int)rcSqr(buildParam.m_regionMinSize);		// Note: area = size*size
	cfg.mergeRegionArea = (int)rcSqr(buildParam.m_regionMergeSize);	// Note: area = size*size
	cfg.maxVertsPerPoly = (int)buildParam.m_vertsPerPoly;
	cfg.tileSize = (int)buildParam.m_tileSize;
	cfg.borderSize = cfg.walkableRadius + 3; // Reserve enough padding.
	cfg.width = cfg.tileSize + cfg.borderSize*2;
	cfg.height = cfg.tileSize + cfg.borderSize*2;
	cfg.detailSampleDist = buildParam.m_detailSampleDist < 0.9f ? 0 : buildParam.m_cellSize * buildParam.m_detailSampleDist;
	cfg.detailSampleMaxError = buildParam.m_cellHeight * buildParam.m_detailSampleMaxError;
	rcVcopy(cfg.bmin, bmin);
	rcVcopy(cfg.bmax, bmax);

	// Tile cache params.
	dtTileCacheParams tcparams;
	memset(&tcparams, 0, sizeof(tcparams));
	rcVcopy(tcparams.orig, bmin);
	tcparams.cs = buildParam.m_cellSize;
	tcparams.ch = buildParam.m_cellHeight;
	tcparams.width = (int)buildParam.m_tileSize;
	tcparams.height = (int)buildParam.m_tileSize;
	tcparams.walkableHeight = buildParam.m_agentHeight;
	tcparams.walkableRadius = buildParam.m_agentRadius;
	tcparams.walkableClimb = buildParam.m_agentMaxClimb;
	tcparams.maxSimplificationError = buildParam.m_edgeMaxError;
	tcparams.maxTiles = tw*th*EXPECTED_LAYERS_PER_TILE;
	tcparams.maxObstacles = 128;

	dtFreeTileCache(world.m_tileCache);

	world.m_tileCache = dtAllocTileCache();
	if (!world.m_tileCache)
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not allocate tile cache.");
		return false;
	}
	status = world.m_tileCache->init(&tcparams, m_talloc, m_tcomp, world.m_tmproc);
	if (dtStatusFailed(status))
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not init tile cache.");
		return false;
	}

	if (!world.m_navMesh)
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not allocate navmesh.");
		return false;
	}

	dtNavMeshParams params;
	memset(&params, 0, sizeof(params));
	rcVcopy(params.orig, world.m_geom->getMeshBoundsMin());
	params.tileWidth = buildParam.m_tileSize*buildParam.m_cellSize;
	params.tileHeight = buildParam.m_tileSize*buildParam.m_cellSize;
	params.maxTiles = m_maxTiles;
	params.maxPolys = m_maxPolysPerTile;

	status = world.m_navMesh->init(&params);
	if (dtStatusFailed(status))
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not init navmesh.");
		return false;
	}

	status = world.m_navQuery->init(world.m_navMesh, 2048);
	if (dtStatusFailed(status))
	{
		m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Could not init Detour navmesh query");
		return false;
	}


	// Preprocess tiles.

	m_ctx->resetTimers();

	m_cacheLayerCount = 0;
	m_cacheCompressedSize = 0;
	m_cacheRawSize = 0;


	for (int y = 0; y < th; ++y)
	{
		for (int x = 0; x < tw; ++x)
		{
			TileCacheData tiles[MAX_LAYERS];
			memset(tiles, 0, sizeof(tiles));
			int ntiles = rasterizeTileLayers(m_ctx, world.m_geom, x, y, cfg, tiles, MAX_LAYERS);

			for (int i = 0; i < ntiles; ++i)
			{
				TileCacheData* tile = &tiles[i];
				status = world.m_tileCache->addTile(tile->data, tile->dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
				if (dtStatusFailed(status))
				{
					dtFree(tile->data);
					tile->data = 0;
					continue;
				}

				m_cacheLayerCount++;
				m_cacheCompressedSize += tile->dataSize;
				m_cacheRawSize += calcLayerBufferSize(tcparams.width, tcparams.height);
			}
		}
	}

	// Build initial meshes
	m_ctx->startTimer(RC_TIMER_TOTAL);
	for (int y = 0; y < th; ++y)
		for (int x = 0; x < tw; ++x)
			world.m_tileCache->buildNavMeshTilesAt(x,y, world.m_navMesh);
	m_ctx->stopTimer(RC_TIMER_TOTAL);

	//m_cacheBuildTimeMs = m_ctx->getAccumulatedTime(RC_TIMER_TOTAL)/1000.0f;
	//m_cacheBuildMemUsage = m_talloc->high;


	const dtNavMesh* nav = world.m_navMesh;
	int navmeshMemUsage = 0;
	for (int i = 0; i < nav->getMaxTiles(); ++i)
	{
		const dtMeshTile* tile = nav->getTile(i);
		if (tile->header)
			navmeshMemUsage += tile->dataSize;
	}

	return true;
}
void RecastNavigationManager::DestroyWorld(const char* name) 
{
	WorldByNameIter worldIter = m_worldContiner.find(name);
	if( worldIter==m_worldContiner.end() )
		return;

	World& world = worldIter->second;

	//
	BotAgentsByWorldNameIter botAgentsIter = m_botAgentsByWorldName.find(name);
	if (botAgentsIter!=m_botAgentsByWorldName.end())
	{
		std::set<std::string>& agents = botAgentsIter->second;
		for (std::set<std::string>::iterator i = agents.begin();
			i != agents.end();++i)
		{
			BotAgentByNameIter it = m_botAgentContiner.find(i->c_str());
			if(it==m_botAgentContiner.end())
				continue;

			m_botAlloc.Free(it.GetData());
			m_botAgentContiner.erase(it);
		}
		m_botAgentsByWorldName.erase(botAgentsIter);
	}

	//
	ObstaclesByWorldNameIter obstaclesIter = m_obstaclesByWorldName.find(name);
	if (obstaclesIter!=m_obstaclesByWorldName.end())
	{
		std::set<std::string>& obstacles = obstaclesIter->second;
		const int obstaclesCount = obstacles.size();
		for (std::set<std::string>::iterator i=obstacles.begin(); i!=obstacles.end(); ++i)
			m_obstacleContiner.erase(i->c_str());
		m_obstaclesByWorldName.erase(obstaclesIter);
	}

	world.Destroy();

	m_worldContiner.erase(worldIter);

#ifdef RC_VSDBG_ENBALE
	if(m_visualDebug)
		m_visualDebug->RemoveShowContent(name);
#endif

	return;
}
bool RecastNavigationManager::AddNaviData(const char* worldName, const char* szNaviFile) {return true;}
bool RecastNavigationManager::RemoveNaviData( const char* worldName, const char* szNaviFile) {return true;}

void RecastNavigationManager::Update(float fSecond)
{
	for(WorldByNameIter it = m_worldContiner.begin();
		it != m_worldContiner.end(); ++it)
	{
		World& world = it->second;
		world.m_navCrowd->update(fSecond, NULL);
		world.m_tileCache->update(fSecond, world.m_navMesh);
	}

	for (BotAgentByNameIter it=m_botAgentContiner.begin(); 
		it!=m_botAgentContiner.end(); ++it)
		it.GetData()->Update(fSecond);


#ifdef RC_VSDBG_ENBALE
	if( m_bVisualDebug && m_visualDebug)
		m_visualDebug->Update(fSecond);
#endif
}
void RecastNavigationManager::Clear() 
{
	//
	for ( BotAgentByNameIter it=m_botAgentContiner.begin();
		it!=m_botAgentContiner.end(); ++it )
	{
		rcBotAgent* botagentPtr = it.GetData();
		botagentPtr->~rcBotAgent();
		m_botAlloc.Free(botagentPtr);
	}
	m_botAgentContiner.clear();


	//
	for(ObstacleByNameIter it = m_obstacleContiner.begin();
		it!=m_obstacleContiner.end(); ++it)
	{
		delete it.GetData();
	}
	m_obstacleContiner.clear();

	//
	m_botAgentsByWorldName.clear();
	m_obstaclesByWorldName.clear();


	for(WorldByNameIter worldIter = m_worldContiner.begin();
		worldIter!=m_worldContiner.end(); ++worldIter )
	{
		World& world = worldIter->second;
		world.Destroy();

#ifdef RC_VSDBG_ENBALE
		if( m_visualDebug )
			m_visualDebug->RemoveShowContent(worldIter->first.c_str());
#endif
	}
	m_worldContiner.clear();

	m_botAlloc.Shutdown();
}
void RecastNavigationManager::Realize(const NaviGeoRender* render)
{
	//duDebugDrawNavMeshWithClosedList(&m_debugDraw, *m_navMesh, *m_navQuery, m_navMeshDrawFlags);
}

const RecastNavigationManager::World* RecastNavigationManager::GetWorld(const char* worldName) const
{
	WorldByNameConstIter worldIter = m_worldContiner.find(worldName);
	if ( worldIter==m_worldContiner.end() )
		return 0;

	const World& world = worldIter->second;

	return &world;
}
bool RecastNavigationManager::BotCreate(const char* worldName, const char* name, const NaviBotInitConfig& conf)
{
	const World* WorldPtr = GetWorld(worldName);

	if (0==WorldPtr)
		return false;

	const World& world = *WorldPtr;

	if( m_botAgentsByWorldName[worldName].size() >= world.m_navCrowd->getAgentCount() )
		return false;	

	if( GetBotAgentByName(name)!=0 )
		return false;

	rcBotAgent::InitConfig botConf;
	botConf.fSpeed	= conf.m_fSpeed;
	botConf.fRadius = conf.m_fRadius;
	botConf.fCheckPointRadius	= conf.m_fCheckPointRadius;
	botConf.bEnableAvoidance	= conf.m_bEnableAvoidence;
	botConf.bEnableShortcut		= conf.m_bEnableShortcut;
	botConf.bEnableSlowingDown	= conf.m_bEnableSlowingDown;
	botConf.bEnableStop		= conf.m_bEnableStop;
	botConf.pObserver		= m_pBotAgentObserver;

	rcVcopy(botConf.Position, (const float*)&conf.m_vecPos);
	rcBotAgent* newBotAgent= m_botAlloc.Alloc();
	new (newBotAgent) rcBotAgent(name, worldName, world.m_navMesh, world.m_navQuery, world.m_navCrowd, world.m_tileCache, botConf);
	m_botAgentContiner[name] = newBotAgent;
	m_botAgentsByWorldName[worldName].insert(name);

	return true;
}
void RecastNavigationManager::BotRemove(const char* name) 
{
	BotAgentByNameIter it = m_botAgentContiner.find(name);
	if( it==m_botAgentContiner.end() )
		return;

	rcBotAgent* botagentPtr = it.GetData();
	m_botAgentContiner.erase(it);
	m_botAgentsByWorldName[botagentPtr->GetWorldName()].erase(name);
	botagentPtr->~rcBotAgent();
	m_botAlloc.Free(botagentPtr);
}
bool RecastNavigationManager::BotHas(const char* name) 
{
	return (GetBotAgentByName(name)!=0);
}
bool RecastNavigationManager::BotCango(const char* name, const NaviVector3& dest)
{
	return true;
}

bool RecastNavigationManager::BotGoto(const char* name, const NaviBotGotoConfig& conf)
{
	BotAgentByNameIter it = m_botAgentContiner.find(name);
	if( it==m_botAgentContiner.end() )
		return false;
	rcBotAgent* botagentPtr = it.GetData();

	float destInNavi[3];
	m_coordSys.ClientToRecastNavPos((const float*)&conf.m_destPosition, destInNavi);

	return botagentPtr->Goto(destInNavi, conf.m_fSpeed, m_queryPointExtents);
}
bool RecastNavigationManager::BotStop(const char* name)
{
	BotAgentByNameIter it = m_botAgentContiner.find(name);
	if( it==m_botAgentContiner.end() )
		return false;
	rcBotAgent* botagentPtr = it.GetData();

	botagentPtr->Stop();

	return true;
}

void RecastNavigationManager::BotSetPosition(const char* name, const NaviVector3& pos)
{
	BotAgentByNameIter it = m_botAgentContiner.find(name);
	if( it==m_botAgentContiner.end() )
		return ;
	rcBotAgent* botagentPtr = it.GetData();

	float destInNavi[3];
	m_coordSys.ClientToRecastNavPos((const float*)&pos, destInNavi);

	botagentPtr->SetPosition(destInNavi);
}
bool RecastNavigationManager::BotGetPosition(const char* name, NaviVector3& pos)
{
	BotAgentByNameIter it = m_botAgentContiner.find(name);
	if( it==m_botAgentContiner.end() )
		return false;
	rcBotAgent* botagentPtr = it.GetData();

	m_coordSys.RecastNavToClientPos(botagentPtr->GetPosition(), (float*)&pos);

	return true;
}
bool RecastNavigationManager::BotGetSpeed(const char* name, NaviVector3& speed)
{
	BotAgentByNameIter it = m_botAgentContiner.find(name);
	if( it==m_botAgentContiner.end() )
		return false;
	rcBotAgent* botagentPtr = it.GetData();

	m_coordSys.RecastNavToClientPos(botagentPtr->GetSpeed(), (float*)&speed);

	return true;
}
bool RecastNavigationManager::BotSetSpeed(const char* name, float fSpeed)
{
	BotAgentByNameIter it = m_botAgentContiner.find(name);
	if( it==m_botAgentContiner.end() )
		return false;

	rcBotAgent* botagentPtr = it.GetData();
	botagentPtr->SetSpeed(fSpeed);

	return true;
}
bool RecastNavigationManager::BotPauseMoving(const char*name)
{
	BotAgentByNameIter it = m_botAgentContiner.find(name);
	if( it==m_botAgentContiner.end() )
		return false;

	rcBotAgent* botagentPtr = it.GetData();

	botagentPtr->PauseMoving();

	return true;
}
bool RecastNavigationManager::BotResumMoving(const char*name)
{
	BotAgentByNameIter it = m_botAgentContiner.find(name);
	if( it==m_botAgentContiner.end() )
		return false;

	rcBotAgent* botagentPtr = it.GetData();

	botagentPtr->ResumMoving();

	return true;
}
void RecastNavigationManager::BotSetEnableAvoidance(const char* name, bool bEnable)
{
	BotAgentByNameIter it = m_botAgentContiner.find(name);
	if( it==m_botAgentContiner.end() )
		return;

	rcBotAgent* botagentPtr = it.GetData();
	botagentPtr->SetEnableAvoidence(bEnable);
}

bool RecastNavigationManager::BodyCreate(const char* wroldName, const char* name, const NaviBodyInitConfig& conf)
{
	const World* worldPtr = GetWorld(wroldName);
	if( !worldPtr )
		return false;

	bool res = false;

	if(BodyHas(name))
		return false;


	rcObstacle*obstaclePtr = 0;
	if(strcmp(conf.GetShape(), "Cylinder")==0)
	{
		const NaviCylinderBodyInitConfig* cylinderConf = (const NaviCylinderBodyInitConfig*)&conf;
		float pos[3];
		m_coordSys.ClientToRecastNavPos((const float*)&cylinderConf->m_Position, pos);
		obstaclePtr = new rcCylinderObstacle(name, wroldName, worldPtr->m_tileCache, pos, cylinderConf->m_fRadius, cylinderConf->m_fHeight);
	}

	if (strcmp(conf.GetShape(), "Box")==0)
	{
		const NaviBoxBodyInitConfig* boxConf = (const NaviBoxBodyInitConfig*)&conf;
		float pos[3], boxHalfExtents[3], rotate[3];
		m_coordSys.ClientToRecastNavPos((const float*)&boxConf->m_Position, pos);
		m_coordSys.ClientToRecastNavPos((const float*)&boxConf->m_boxHalfExtents, boxHalfExtents);
		m_coordSys.ClientToRecastNavPos((const float*)&boxConf->m_rotate, rotate);

		obstaclePtr = new rcBoxObstacle(name, wroldName,worldPtr->m_tileCache, pos, boxHalfExtents, rotate[1]);
	}
	if( obstaclePtr!=0 )
	{
		m_obstacleContiner[name] = obstaclePtr;
		m_obstaclesByWorldName[wroldName].insert(name);
	}

	return obstaclePtr!=0;
}
void RecastNavigationManager::BodyRemove(const char* name) 
{

	ObstacleByNameIter it = m_obstacleContiner.find(name);
	if( it==m_obstacleContiner.end() )
		return;

	m_obstaclesByWorldName[it.GetData()->GetWorldName()].erase(name);
	delete it.GetData();
	m_obstacleContiner.erase(it);
}

bool RecastNavigationManager::BodyHas(const char* name)
{
	return (m_obstacleContiner.find(name)!=m_obstacleContiner.end());
}
void RecastNavigationManager::BodyGoto(const char* name, const NaviBodyGotoConfig& conf) {}
void RecastNavigationManager::BodySetPosition(const char* name, const NaviVector3& pos) {}
void RecastNavigationManager::BodySetSpeed(const char* name, const NaviVector3& linerspeed, const NaviVector3& angularSpeed ) {}

bool RecastNavigationManager::QueryCanGo(const char* worldName, const NaviVector3& from, const NaviVector3& to, float propagationBoxExtent, NaviVector3**pathout, int* nNodeCount)
{
	const World* wroldPtr = GetWorld(worldName);
	if (!wroldPtr)
		return false;

	float starPos[3], destPos[3];
	m_coordSys.ClientToRecastNavPos((const float*)&from, starPos);
	m_coordSys.ClientToRecastNavPos((const float*)&to, destPos);

	const float*queryExtents = m_queryPointExtents;

	dtPolyRef refStart = 0, refEnd = 0;
	wroldPtr->m_navQuery->findNearestPoly(starPos, queryExtents, wroldPtr->m_navCrowd->getFilter(), &refStart, 0);
	wroldPtr->m_navQuery->findNearestPoly(destPos, queryExtents, wroldPtr->m_navCrowd->getFilter(), &refEnd, 0);

#ifdef RC_VSDBG_ENBALE
	if (m_visualDebug)
	{
		if( !m_queryCanGoDebugDrawLine )
			m_queryCanGoDebugDrawLine = m_visualDebug->AddDisplayItem();
		m_queryCanGoDebugDrawLine->begin(DU_DRAW_LINES, 2);
		m_queryCanGoDebugDrawLine->vertex(starPos, duRGBA(255,255,255,255));
		m_queryCanGoDebugDrawLine->vertex(destPos, duRGBA(255,255,255,255));
		m_queryCanGoDebugDrawLine->end();

		if( !m_queryCanGoDebugDrawBox )
			m_queryCanGoDebugDrawBox = m_visualDebug->AddDisplayItem();
		float bmin[3], bmax[3];
		rcVsub(bmin, starPos, queryExtents);
		rcVadd(bmax, starPos, queryExtents);
		duDebugDrawBoxWire(m_queryCanGoDebugDrawBox, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], duRGBA(255,255,255,128), 1.0f);
		if( !m_queryCanGoDebugDrawBox2 )
			m_queryCanGoDebugDrawBox2 = m_visualDebug->AddDisplayItem();
		rcVsub(bmin, destPos, queryExtents);
		rcVadd(bmax, destPos, queryExtents);
		duDebugDrawBoxWire(m_queryCanGoDebugDrawBox2, bmin[0],bmin[1],bmin[2], bmax[0],bmax[1],bmax[2], duRGBA(255,255,255,128), 1.0f);
	}
#endif
	if(refStart==0 || refEnd==0) 
		return false;

	int npolys = 0;
	dtPolyRef polyRefs[MAX_POLYS];
	wroldPtr->m_navQuery->findPath(refStart, refEnd, starPos, destPos, wroldPtr->m_navCrowd->getFilter(), polyRefs, &npolys, MAX_POLYS);
	if ( 0==npolys )	
		return false;

	bool bCanGo = polyRefs[npolys-1]==refEnd;
	if (bCanGo && pathout && nNodeCount)
	{
		wroldPtr->m_navQuery->findStraightPath(starPos, destPos, polyRefs, npolys,
			m_straightPath, m_straightPathFlags, m_straightPathPolys, &m_nstraightPath, MAX_POLYS, 0);
		*pathout	= (NaviVector3*)m_straightPath;
		*nNodeCount = m_nstraightPath;

#ifdef RC_VSDBG_ENBALE
		if (m_visualDebug)
		{
			const unsigned int startCol = duRGBA(128,25,0,192);
			const unsigned int endCol = duRGBA(51,102,0,129);
			if( !m_queryCanGoDebugDrawPath )
				m_queryCanGoDebugDrawPath = m_visualDebug->AddDisplayItem();
			m_queryCanGoDebugDrawPath->depthMask(false);
			const unsigned int spathCol = duRGBA(64,16,0,220);
			const unsigned int offMeshCol = duRGBA(128,96,0,220);
			m_queryCanGoDebugDrawPath->depthMask(false);
			m_queryCanGoDebugDrawPath->begin(DU_DRAW_LINES, 2.0f);
			for (int i = 0; i < m_nstraightPath-1; ++i)
			{
				unsigned int col = 0;
				if (m_straightPathFlags[i] & DT_STRAIGHTPATH_OFFMESH_CONNECTION)
					col = offMeshCol;
				else
					col = spathCol;

				m_queryCanGoDebugDrawPath->vertex(m_straightPath[i*3], m_straightPath[i*3+1]+0.1f, m_straightPath[i*3+2], col);
				m_queryCanGoDebugDrawPath->vertex(m_straightPath[(i+1)*3], m_straightPath[(i+1)*3+1]+0.1f, m_straightPath[(i+1)*3+2], col);
			}
			m_queryCanGoDebugDrawPath->end();
			m_queryCanGoDebugDrawPath->depthMask(true);

			if(!m_queryCanGoDebugDrawNodes)
				m_queryCanGoDebugDrawNodes = m_visualDebug->AddDisplayItem();
			m_queryCanGoDebugDrawNodes->depthMask(false);
			m_queryCanGoDebugDrawNodes->begin(DU_DRAW_POINTS, 6.0f);
			for (int i = 0; i < m_nstraightPath; ++i)
			{
				unsigned int col = 0;
				if (m_straightPathFlags[i] & DT_STRAIGHTPATH_START)
					col = startCol;
				else if (m_straightPathFlags[i] & DT_STRAIGHTPATH_START)
					col = endCol;
				else if (m_straightPathFlags[i] & DT_STRAIGHTPATH_OFFMESH_CONNECTION)
					col = offMeshCol;
				else
					col = spathCol;
				m_queryCanGoDebugDrawNodes->vertex(m_straightPath[i*3], m_straightPath[i*3+1]+0.1f, m_straightPath[i*3+2], col);
			}
			m_queryCanGoDebugDrawNodes->end();
			m_queryCanGoDebugDrawNodes->depthMask(true);
		}
#endif

	}



	return bCanGo;
}
bool RecastNavigationManager::QueryRayCanGo(const char* worldName, const NaviVector3& from, const NaviVector3& to, NaviVector3* hitPos/*=0*/)
{
	const World* worldPtr = GetWorld(worldName);
	if( !worldPtr )
		return false;

	float startPos[3],	destPos[3];
	m_coordSys.ClientToRecastNavPos((const float*)&from, startPos);
	m_coordSys.ClientToRecastNavPos((const float*)&to, destPos);

	dtPolyRef refStart = 0;
	worldPtr->m_navQuery->findNearestPoly(startPos, worldPtr->m_navCrowd->getQueryExtents(), worldPtr->m_navCrowd->getFilter(), &refStart, 0);

	if(refStart==0)
		return false;

	float t = 0.f;
	float hitNormal[] = {0,0,0};
	dtPolyRef polys[MAX_POLYS];
	int nPolys = 0;
	float farPos[3]={0,0,0};
	worldPtr->m_navQuery->raycast(refStart, startPos, destPos, worldPtr->m_navCrowd->getFilter(), &t, hitNormal, polys, &nPolys, MAX_POLYS);
	if(hitPos && t<=1.0f)
	{
		float h = 0;
		dtVlerp((float*)farPos, startPos, destPos, t);
		if( nPolys )
		{
			worldPtr->m_navQuery->getPolyHeight(polys[nPolys-1], (const float*)farPos, &h);
			farPos[1] = h;
		}
		m_coordSys.RecastNavToClientPos(farPos, (float*)hitPos);
	}


#ifdef RC_VSDBG_ENBALE
	if( !m_queryRayCanGoDebugDrawPoint )
		m_queryRayCanGoDebugDrawPoint = m_visualDebug->AddDisplayItem();
	m_queryRayCanGoDebugDrawPoint->begin(DU_DRAW_POINTS, 5);
	m_queryRayCanGoDebugDrawPoint->vertex((float*)farPos, duRGBA(255,255,255,255));
	m_queryRayCanGoDebugDrawPoint->end();
	if( !m_queryRayCanGoDebugDrawLine )
		m_queryRayCanGoDebugDrawLine = m_visualDebug->AddDisplayItem();
	m_queryRayCanGoDebugDrawLine->begin(DU_DRAW_LINES, 2);
	m_queryRayCanGoDebugDrawLine->vertex((float*)startPos, duRGBA(0,255,0,255));
	m_queryRayCanGoDebugDrawLine->vertex((float*)destPos, duRGBA(0,255,0,255));
	m_queryRayCanGoDebugDrawLine->end();
#endif

	return (t>1.f);
}
bool RecastNavigationManager::QueryNearestPosOnGround(const char* worldName, const NaviVector3& pos, NaviVector3& precisePos)
{
	const World* worldPtr = GetWorld(worldName);
	if( !worldPtr )
		return false;

	dtPolyRef polyRef;
	float startpos[3];
	m_coordSys.ClientToRecastNavPos((const float*)&pos, startpos);

	return dtStatusSucceed(worldPtr->m_navQuery->findNearestPoly(startpos, POLYPICK_EXT, 
		worldPtr->m_navCrowd->getFilter(), &polyRef, (float*)&precisePos));
}



bool RecastNavigationManager::LoadNavMeshFromFile(const char* path, dtNavMesh* mesh)
{
	FILE* fp = fopen(path, "rb");
	if (!fp) return 0;

	// Read header.
	fread(&m_navMeshHeader, sizeof(NavMeshSetHeader), 1, fp);
	if (m_navMeshHeader.magic != NAVMESHSET_MAGIC)
	{
		fclose(fp);
		return 0;
	}
	if (m_navMeshHeader.version != NAVMESHSET_VERSION)
	{
		fclose(fp);
		return 0;
	}

	if (!mesh)
	{
		fclose(fp);
		return 0;
	}
	dtStatus status = mesh->init(&m_navMeshHeader.params);
	if (dtStatusFailed(status))
	{
		fclose(fp);
		return 0;
	}

	// Read tiles.
	for (int i = 0; i < m_navMeshHeader.numTiles; ++i)
	{
		NavMeshTileHeader tileHeader;
		fread(&tileHeader, sizeof(tileHeader), 1, fp);
		if (!tileHeader.tileRef || !tileHeader.dataSize)
			break;

		unsigned char* data = (unsigned char*)dtAlloc(tileHeader.dataSize, DT_ALLOC_PERM);
		if (!data) break;
		memset(data, 0, tileHeader.dataSize);
		fread(data, tileHeader.dataSize, 1, fp);

		mesh->addTile(data, tileHeader.dataSize, DT_TILE_FREE_DATA, tileHeader.tileRef, 0);
	}

	fclose(fp);

	return 1;
}

 rcObstacle* RecastNavigationManager::GetObstacleRefByName( const char* name ) const
 {
 	ObstacleByNameConstIter it = m_obstacleContiner.find(name);
 	if(it==m_obstacleContiner.end())
 		return 0;
 
 	return it.GetData();
 }

rcBotAgent* RecastNavigationManager::GetBotAgentByName( const char* name ) const
{
	BotAgentByNameConstIter it = m_botAgentContiner.find(name);
	return it!=m_botAgentContiner.end()? it.GetData() : 0;
}

void RecastNavigationManager::ShowVisualDebug(bool bShow)
{
#ifdef RC_VSDBG_ENBALE
	m_bVisualDebug = bShow;
	if (m_visualDebug )
	{
		if( !m_visualDebug->IsCreated() )
			m_visualDebug->Create();
		m_visualDebug->Show(bShow);
	}
#endif
}
static  RecastNavigationManager recastNavigationMgr[2];
NAVI_DLL_EXPORT INavigationManager* GetNavigationManager(int user/* = 0*/)
{
	if(user==1)
		return &recastNavigationMgr[1];
	return &recastNavigationMgr[0];
}

