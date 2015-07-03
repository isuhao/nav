#include "rcCrowdTool.h"
#include "rcDebugDraw.h"
#include "imgui.h"

CrowdTool::CrowdTool() : m_debugDraw(0),m_expandDebugDraw(false),m_expandSelectedDebugDraw(false){}
CrowdTool::~CrowdTool(){}

void CrowdTool::init()
{
#if 0
	if (!sample)
		return;

	m_state = (CrowdToolState*)sample->getToolState(type());
	if (!m_state)
	{
		m_state = new CrowdToolState();
		sample->setToolState(type(), m_state);
	}
	m_state->init(sample);
#endif
}

void CrowdTool::reset(rcDebugDraw* dbgDraw){ m_debugDraw = dbgDraw; }

void CrowdTool::handleMenu()
{
	rcDebugDraw* params = m_debugDraw;
	if(!params)
		return;

	if (imguiCollapse("Selected Debug Draw", 0, m_expandSelectedDebugDraw))
		m_expandSelectedDebugDraw = !m_expandSelectedDebugDraw;

	if (m_expandSelectedDebugDraw)
	{
		imguiIndent();
		if (imguiCheck("Show Corners", params->m_showCorners))
			params->m_showCorners = !params->m_showCorners;
		if (imguiCheck("Show Collision Segs", params->m_showCollisionSegments))
			params->m_showCollisionSegments = !params->m_showCollisionSegments;
		if (imguiCheck("Show Path", params->m_showPath))
			params->m_showPath = !params->m_showPath;
		if (imguiCheck("Show VO", params->m_showVO))
			params->m_showVO = !params->m_showVO;
// 		if (imguiCheck("Show Path Optimization", params->m_showOpt))
// 			params->m_showOpt = !params->m_showOpt;
		if (imguiCheck("Show Neighbours", params->m_showNeis))
			params->m_showNeis = !params->m_showNeis;
		imguiUnindent();
	}

	if (imguiCollapse("Debug Draw", 0, m_expandDebugDraw))
		m_expandDebugDraw = !m_expandDebugDraw;

	if (m_expandDebugDraw)
	{
		imguiIndent();
// 		if (imguiCheck("Show Labels", params->m_showLabels))
// 			params->m_showLabels = !params->m_showLabels;
		if (imguiCheck("Show Prox Grid", params->m_showGrid))
			params->m_showGrid = !params->m_showGrid;
		if (imguiCheck("Show Nodes", params->m_showNodes))
			params->m_showNodes = !params->m_showNodes;
// 		if (imguiCheck("Show Perf Graph", params->m_showPerfGraph))
// 			params->m_showPerfGraph = !params->m_showPerfGraph;
		imguiUnindent();
	}
}

void CrowdTool::handleClick(const float* s, const float* p, bool shift)
{
#if 0
	if (!m_sample) return;
	if (!m_state) return;
	InputGeom* geom = m_sample->getInputGeom();
	if (!geom) return;
	dtCrowd* crowd = m_sample->getCrowd();
	if (!crowd) return;

	if (m_mode == TOOLMODE_CREATE)
	{
		if (shift)
		{
			// Delete
			int ahit = m_state->hitTestAgents(s,p);
			if (ahit != -1)
				m_state->removeAgent(ahit);
		}
		else
		{
			// Add
			m_state->addAgent(p);
		}
	}
	else if (m_mode == TOOLMODE_MOVE_TARGET)
	{
		m_state->setMoveTarget(p, shift);
	}
	else if (m_mode == TOOLMODE_SELECT)
	{
		// Highlight
		int ahit = m_state->hitTestAgents(s,p);
		m_state->hilightAgent(ahit);
	}
	else if (m_mode == TOOLMODE_TOGGLE_POLYS)
	{
		dtNavMesh* nav = m_sample->getNavMesh();
		dtNavMeshQuery* navquery = m_sample->getNavMeshQuery();
		if (nav && navquery)
		{
			dtQueryFilter filter;
			const float* ext = crowd->getQueryExtents();
			float tgt[3];
			dtPolyRef ref;
			navquery->findNearestPoly(p, ext, &filter, &ref, tgt);
			if (ref)
			{
				unsigned short flags = 0;
				if (dtStatusSucceed(nav->getPolyFlags(ref, &flags)))
				{
					flags ^= SAMPLE_POLYFLAGS_DISABLED;
					nav->setPolyFlags(ref, flags);
				}
			}
		}
	}
#endif
}

void CrowdTool::handleStep()
{
}

void CrowdTool::handleToggle()
{
	//if (!m_state) return;
	//m_state->setRunning(!m_state->isRunning());
}

void CrowdTool::handleUpdate(const float dt)
{
	rcIgnoreUnused(dt);
}

void CrowdTool::handleRender()
{
}

void CrowdTool::handleRenderOverlay(double* proj, double* model, int* view)
{
	rcIgnoreUnused(model);
	rcIgnoreUnused(proj);
}

