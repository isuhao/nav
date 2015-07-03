#include "DetourNavMesh.h"
#include "DetourCrowd.h"
#include "DetourTileCache.h"
#include "../RecastNavMeshBuilder.h"
#pragma once

struct rcToolParamater
{
	dtNavMesh*		navMesh;
	dtNavMeshQuery* navMeshQuery;
	dtTileCache*	navTileCache;
	BuildParamter	buildParamater;
};
class rcDebugDraw;
class rcTool
{
	virtual int type() = 0;
	virtual void init() = 0;
	virtual void reset(rcDebugDraw*) = 0;
	virtual void handleMenu() = 0;
	virtual void handleClick(const float* s, const float* p, bool shift) = 0;
	virtual void handleRender() = 0;
	virtual void handleRenderOverlay(double* proj, double* model, int* view) = 0;
	virtual void handleToggle() = 0;
	virtual void handleStep() = 0;
	virtual void handleUpdate(const float dt) = 0;

	rcToolParamater m_param;
};

class CrowdTool : public rcTool
{
	void updateAgentParams();
	void updateTick(const float dt);

public:
	CrowdTool();
	virtual ~CrowdTool();

	virtual int type() { return 0; }
	virtual void init();
	virtual void reset(rcDebugDraw*);
	virtual void handleMenu();
	virtual void handleClick(const float* s, const float* p, bool shift);
	virtual void handleToggle();
	virtual void handleStep();
	virtual void handleUpdate(const float dt);
	virtual void handleRender();
	virtual void handleRenderOverlay(double* proj, double* model, int* view);
private:
	rcDebugDraw*	m_debugDraw;
	bool			m_expandDebugDraw;
	bool			m_expandSelectedDebugDraw;
};