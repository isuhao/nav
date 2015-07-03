#ifndef _VISUAL_DEBUG_H_
#define _VISUAL_DEBUG_H_
//------------------------------------------------------------------------------
// �ļ���:		VisualDebug.h
// ��  ��:		
// ˵  ��:		
// ��������:	2014��5��6��
// ������:		����־
// ��ע: 
// ��Ȩ����:	������ţ�������޹�˾
//------------------------------------------------------------------------------

#include "RecastNavMeshBuilder.h"
#include "DetourNavMeshQuery.h"
#include "DetourCrowd.h"
#include "DebugDraw.h"
#include <vector>
#include <string>
#include <map>

class duDebugDrawItem : public duDebugDraw
{
public:
	virtual void draw(struct duDebugDraw* dd) = 0;

};
class InputTerrainGeom;
class VisualDebug
{
protected:
	struct NaviWorld
	{
		dtNavMesh*			m_navMesh;
		dtNavMeshQuery*		m_navMeshQuery;
		dtCrowd*			m_dtCrowd;
		dtTileCache*		m_dtTileCache;
		InputTerrainGeom*	m_inputGeom;
		BuildParamter*		m_buildParmter;

	};

	typedef std::map<std::string,NaviWorld> WorldByName;
	typedef WorldByName::const_iterator WorldByNameConstIter;
	typedef WorldByName::iterator WorldByNameIter;
public:
	VisualDebug():m_nDisplayCount(0){}
	virtual ~VisualDebug(void){}

	virtual bool			Create() { return true; }
	virtual bool			IsCreated()const { return true; }
	virtual void			Show(bool bShow){}
	virtual void			Update(float fSeconds){}
	virtual void			OnAddShowContent(const char* name){}
	virtual void			OnRemoveShowContent(const char* name){}
	virtual void			AllocDebugDraw(duDebugDrawItem**) = 0;
	int						GetDisplayItemCount(){ return m_nDisplayCount; }
	duDebugDrawItem*		GetDisplayItemByIndex(int i){ return m_displayAry[i]; }
	duDebugDrawItem*		AddDisplayItem()
	{
		if (m_nDisplayCount>=m_displayAry.size()) {
			duDebugDrawItem* debugDraw = 0;
			AllocDebugDraw(&debugDraw);
			if (0==debugDraw)
				return 0;
			m_displayAry.push_back(debugDraw);
		}

		duDebugDrawItem* debugDraw = m_displayAry[m_nDisplayCount++];
		return debugDraw;
	}

	void AddShowContent(const char* name, dtNavMesh*navMesh, 
		dtNavMeshQuery*navMeshQuery, 
		dtCrowd*crowd, 
		dtTileCache*tileCache, 
		InputTerrainGeom*geom, 
		BuildParamter* buildParam)	
	{	
		NaviWorld& world = m_worldByName[name];
		world.m_navMesh			= navMesh;
		world.m_navMeshQuery	= navMeshQuery;
		world.m_dtCrowd			= crowd;
		world.m_dtTileCache		= tileCache;
		world.m_inputGeom		= geom;
		world.m_buildParmter	= buildParam;

		OnAddShowContent(name);
	}

	bool GetShowContent(const char* name, NaviWorld& out) const
	{
		WorldByNameConstIter it = m_worldByName.find(name);
		if ( it==m_worldByName.end() )
			return false;

		out = it->second;
		return true;
	}

	void RemoveShowContent(const char* name)
	{
		WorldByNameIter it = m_worldByName.find(name);
		if ( it==m_worldByName.end() )
			return;

		m_worldByName.erase(it);
		OnRemoveShowContent(name);
	}

protected:

	std::map<std::string,NaviWorld> m_worldByName;


	int					m_nDisplayCount;
	std::vector<duDebugDrawItem*> m_displayAry;
};
#endif
