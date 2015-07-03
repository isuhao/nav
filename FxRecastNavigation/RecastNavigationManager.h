//------------------------------------------------------------------------------
// 文件名:		RecastNavigationManager.h
// 内  容:		
// 说  明:		
// 创建日期:	2014年4月21日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#pragma once
#include "../Interface/INavigationManager.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "DetourCrowd.h"
#include "DetourLocalBoundary.h"
#include "RecastNavMeshBuilder.h"
#include "DetourTileCache.h"
#include "DetourCommon.h"
#include "Recast.h"
#include "CoordSystem.h"
#include "rcBotAgent.h"
#include "rcBlockAlloc.h"
#include "rcObstacle.h"
#include "string_pod.h"
#include <string>
#include <map>
#include <set>

#ifdef _WIN32
#include "rcDefaultFileIO.h"
#define RC_VSDBG_ENBALE
#endif


class VisualDebug;
struct LinearAllocator;
struct FastLZCompressor;
struct MeshProcess;
class InputTerrainGeom;
class duDebugDrawItem;
class RecastNavigationManager :	public INavigationManager
{
	static const int MAX_POLYS = 256;
	struct NavMeshSetHeader
	{
		int magic;
		int version;
		int numTiles;
		dtNavMeshParams params;
	};

	class World
	{
		World& operator =(const World&);
	public:
		World():m_navMesh(0)
			,m_navQuery(0)
			,m_navCrowd(0)
			,m_tileCache(0)
			,m_tmproc(0)
			,m_geom(0){}

		void Destroy();
		dtNavMesh*			m_navMesh;
		dtNavMeshQuery*		m_navQuery;
		dtCrowd*			m_navCrowd;
		dtTileCache*		m_tileCache;
		MeshProcess*		m_tmproc;
		InputTerrainGeom*	m_geom;
	};

	typedef std::map<std::string, World> WorldByName;
	typedef WorldByName::iterator WorldByNameIter;
	typedef WorldByName::const_iterator WorldByNameConstIter;

	typedef std::map<std::string, std::set<std::string> > BotAgentsByWorldName;
	typedef BotAgentsByWorldName::iterator BotAgentsByWorldNameIter;
	typedef BotAgentsByWorldName::const_iterator BotAgentsByWorldNameConstIter;

	typedef std::map<std::string, std::set<std::string> > ObstaclesByWorldName;
	typedef ObstaclesByWorldName::iterator ObstaclesByWorldNameIter;
	typedef ObstaclesByWorldName::const_iterator ObstaclesByWorldNameConstIter;

	typedef TStringPod<char, rcObstacle*>	ObstacleByName;
	typedef ObstacleByName::iterator ObstacleByNameIter;
	typedef ObstacleByName::const_iterator ObstacleByNameConstIter;

	typedef TStringPod<char, rcBotAgent*> BotAgentByName;
	typedef BotAgentByName::iterator BotAgentByNameIter;
	typedef BotAgentByName::const_iterator BotAgentByNameConstIter;
	friend class BotAgentObserv;
	unsigned int			m_instanceID;
public:
	RecastNavigationManager(void);
	~RecastNavigationManager(void);


	virtual bool	Initialize(const NaviMgrInitConfig& conf);
	virtual bool	IsInitialized();

	virtual void	ShowVisualDebug(bool);
	virtual bool	HasWorld( const char* name );
	virtual bool	CreateWorld(const char* name, const NaviWorldInitConfig& conf);
	virtual void	DestroyWorld(const char* name);
	virtual bool	AddNaviData(const char* worldName, const char* szNaviFile);
	virtual bool	RemoveNaviData( const char* worldName, const char* szNaviFile);

	virtual void	Update(float fSecond);
	virtual void	Clear();
	virtual void	Realize(const NaviGeoRender* render);

	virtual bool	BotCreate(const char* world, const char* name, const NaviBotInitConfig& conf);
	virtual void	BotRemove(const char* name);
	virtual bool	BotHas(const char* name);
	virtual bool	BotCango(const char* name, const NaviVector3& dest);
	virtual bool	BotGoto(const char* name, const NaviBotGotoConfig& conf);
	virtual bool	BotStop(const char* name);

	virtual void	BotSetPosition(const char* name, const NaviVector3& pos);
	virtual bool	BotGetPosition(const char* name, NaviVector3& pos);
	virtual bool	BotGetSpeed(const char* name, NaviVector3& speed);
	virtual bool	BotSetSpeed(const char* name, float fSpeed);
	virtual bool	BotPauseMoving(const char*name);
	virtual bool	BotResumMoving(const char*name);
	virtual void	BotSetEnableAvoidance(const char* name, bool bEnable);

	virtual bool	BodyCreate(const char* world, const char* name, const NaviBodyInitConfig& conf);
	virtual void	BodyRemove(const char* name);
	virtual bool	BodyHas(const char* name);
	virtual void	BodyGoto(const char* name, const NaviBodyGotoConfig& conf);
	virtual void	BodySetPosition(const char* name, const NaviVector3& pos);
	virtual void	BodySetSpeed(const char* name, const NaviVector3& linerspeed, const NaviVector3& angularSpeed );

	virtual bool	QueryCanGo(const char* world, const NaviVector3& from, const NaviVector3& to, float propagationBoxExtent, NaviVector3**pathout = 0, int* nNodeCount = 0);
	virtual bool	QueryRayCanGo(const char* world, const NaviVector3& from, const NaviVector3& to, NaviVector3* hitPos=0);
	virtual bool	QueryNearestPosOnGround(const char* world, const NaviVector3& pos, NaviVector3& precisePos);

protected:
	rcObstacle*		GetObstacleRefByName(const char* name) const;
	rcBotAgent*		GetBotAgentByName(const char* name) const;

	bool			LoadNavMeshFromFile(const char* path, dtNavMesh* mesh);
	bool			GeneratTileCache(World& world);
	const World*	GetWorld(const char* worldName) const;

private:

	unsigned char		m_navMeshDrawFlags;
	bool				m_bVisualDebug;
	VisualDebug*		m_visualDebug;

	NavMeshSetHeader	m_navMeshHeader;
	LinearAllocator*	m_talloc;
	FastLZCompressor*	m_tcomp;

	rcContext*			m_ctx;

	WorldByName			m_worldContiner;
	ObstacleByName		m_obstacleContiner;
	BotAgentByName		m_botAgentContiner;

	BotAgentsByWorldName m_botAgentsByWorldName;
	ObstaclesByWorldName m_obstaclesByWorldName;

	int					m_cacheCompressedSize;
	int					m_cacheRawSize;
	int					m_cacheLayerCount;
	CoordSystem			m_coordSys;

	bool				m_IsInitialize;
	NaviFileIO*			m_pFileIO;

	float				m_straightPath[MAX_POLYS*3];
	unsigned char		m_straightPathFlags[MAX_POLYS];
	dtPolyRef			m_straightPathPolys[MAX_POLYS];
	int					m_nstraightPath;


	INavigationObserver*		m_observer;

	rcBlockAlloc<rcBotAgent,20>	m_botAlloc;


	BuildParamter		m_buildParamater;

	IBotAgentObserver*	m_pBotAgentObserver;

#ifdef RC_VSDBG_ENBALE
	duDebugDrawItem*	m_queryCanGoDebugDrawLine;
	duDebugDrawItem*	m_queryCanGoDebugDrawBox;
	duDebugDrawItem*	m_queryCanGoDebugDrawBox2;
	duDebugDrawItem*	m_queryCanGoDebugDrawPath;
	duDebugDrawItem*	m_queryCanGoDebugDrawNodes;
	duDebugDrawItem*	m_queryRayCanGoDebugDrawLine;
	duDebugDrawItem*	m_queryRayCanGoDebugDrawPoint;
#endif

#ifdef _WIN32
	rcDefaultFileIO		m_DefaultFileio;
#endif

	float m_queryPointExtents[3];

};
