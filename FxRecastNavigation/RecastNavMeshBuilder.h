//------------------------------------------------------------------------------
// 文件名:		RecastNavMeshBuilder.h
// 内  容:		Navmesh builder
// 说  明:		
// 创建日期:	2014年4月25日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#pragma once
#include "fastlz/fastlz.h"
#include "DetourCommon.h"
#include "DetourTileCache.h"
#include "DetourTileCacheBuilder.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMesh.h"
#include "InputTerrainGeom.h"
#include "Recast.h"
#include "PerfTimer.h"
#include <string.h>

/// These are just sample areas to use consistent values across the samples.
/// The use should specify these base on his needs.
enum SamplePolyAreas
{
	SAMPLE_POLYAREA_GROUND,
	SAMPLE_POLYAREA_WATER,
	SAMPLE_POLYAREA_ROAD,
	SAMPLE_POLYAREA_DOOR,
	SAMPLE_POLYAREA_GRASS,
	SAMPLE_POLYAREA_JUMP,
};
enum SamplePolyFlags
{
	SAMPLE_POLYFLAGS_WALK		= 0x01,		// Ability to walk (ground, grass, road)
	SAMPLE_POLYFLAGS_SWIM		= 0x02,		// Ability to swim (water).
	SAMPLE_POLYFLAGS_DOOR		= 0x04,		// Ability to move through doors.
	SAMPLE_POLYFLAGS_JUMP		= 0x08,		// Ability to jump.
	SAMPLE_POLYFLAGS_DISABLED	= 0x10,		// Disabled polygon
	SAMPLE_POLYFLAGS_ALL		= 0xffff	// All abilities.
};


enum {MAX_LAYERS = 32};


struct GeoMetryHeader
{
	int Start;
	int nVertexCount;
};



struct TileCacheData
{
	unsigned char* data;
	int dataSize;
};

struct BuildParamter
{
	float m_tileSize;
	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	float m_regionMinSize;
	float m_regionMergeSize;
	bool m_monotonePartitioning;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	float m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;
};


struct FastLZCompressor : public dtTileCacheCompressor
{
	virtual int maxCompressedSize(const int bufferSize);
	virtual dtStatus compress(const unsigned char* buffer, const int bufferSize,
		unsigned char* compressed, const int /*maxCompressedSize*/, int* compressedSize);
	virtual dtStatus decompress(const unsigned char* compressed, const int compressedSize,
		unsigned char* buffer, const int maxBufferSize, int* bufferSize);
};

struct LinearAllocator : public dtTileCacheAlloc
{
	unsigned char* buffer;
	int capacity;
	int top;
	int high;

	LinearAllocator(const int cap) : buffer(0), capacity(0), top(0), high(0){resize(cap);}
	~LinearAllocator(){dtFree(buffer);}

	void			resize(const int cap);
	virtual void	reset();
	virtual void*	alloc(const int size);
	virtual void	free(void* /*ptr*/){}
};


struct MeshProcess : public dtTileCacheMeshProcess
{
	InputTerrainGeom* m_geom;

	inline			MeshProcess() : m_geom(0) {}
	inline void		init(InputTerrainGeom* geom){ m_geom = geom; }
	virtual void	process(struct dtNavMeshCreateParams*, unsigned char* , unsigned short* );
};


struct NavMeshTileHeader
{
	dtTileRef tileRef;
	int dataSize;
};

struct RasterizationContext
{
	RasterizationContext() :solid(0),triareas(0),lset(0),chf(0),ntiles(0)
	{ memset(tiles, 0, sizeof(TileCacheData)*MAX_LAYERS);}

	~RasterizationContext();

	rcHeightfield* solid;
	unsigned char* triareas;
	rcHeightfieldLayerSet* lset;
	rcCompactHeightfield* chf;
	TileCacheData tiles[MAX_LAYERS];
	int ntiles;
};



/// Recast build context.
class BuildContext : public rcContext
{
	static const int MAX_MESSAGES	= 1000;
	static const int TEXT_POOL_SIZE = 8000;

	TimeVal			m_startTime[RC_MAX_TIMERS];
	int				m_accTime[RC_MAX_TIMERS];

	const char*		m_messages[MAX_MESSAGES];
	int				m_messageCount;
	char			m_textPool[TEXT_POOL_SIZE];
	int				m_textPoolSize;

public:
					BuildContext();
	virtual			~BuildContext();

	void			dumpLog(const char* format, ...);
	int				getLogCount() const;
	const char*		getLogText(const int i) const;

protected:	
	virtual void	doResetLog();
	virtual void	doLog(const rcLogCategory /*category*/, const char* /*msg*/, const int /*len*/);
	virtual void	doResetTimers();
	virtual void	doStartTimer(const rcTimerLabel /*label*/);
	virtual void	doStopTimer(const rcTimerLabel /*label*/);
	virtual int		doGetAccumulatedTime(const rcTimerLabel /*label*/) const;
};


int calcLayerBufferSize(const int gridWidth, const int gridHeight);
int rasterizeTileLayers(rcContext* ctx, InputTerrainGeom* geom,
							   const int tx, const int ty,const rcConfig& cfg,TileCacheData* tiles,const int maxTiles);

template<class T> 
T dtLerp(const T& start, const T&end, float lerp)
{
	T res;
	res = start + (end-start)*lerp;
	return res;
}


