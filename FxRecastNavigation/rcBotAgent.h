//------------------------------------------------------------------------------
// 文件名:		rcBotAgent.h
// 内  容:		
// 说  明:		
// 创建日期:	2014年4月26日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "DetourCrowd.h"
#include "DetourTileCache.h"
#include "rcWorldObject.h"
#include <string>
#pragma once

class rcBotAgent;
class IBotAgentObserver
{
public:
	virtual void Release(){}

	virtual void	OnBotPathQueryDone(rcBotAgent*, const float*nodeBuff, long nNodeCount) = 0;
	virtual void	OnBotSpeedChange(rcBotAgent*, const float* curSpeed, const float* preSpeed,const float* desrDest) = 0;
	virtual void	OnBotArrive(rcBotAgent*, const float* curPos, const float* destPos) = 0;
};


class rcBotAgent : public rcWorldObject
{
public:
	enum{NAME_LENGTH=255};
	static const float VECTOR_ZERO[];
	enum{ INVALID_AGENT_HANDLE = -1 };
	typedef int AgentHandle;

	struct InitConfig
	{
		InitConfig():fRadius(0),fSpeed(0),fCheckPointRadius(0.2f),
			bEnableAvoidance(false),bEnableShortcut(false),pObserver(0),bEnableSlowingDown(false),bEnableStop(false){}
		float			Position[3];
		float			fRadius;
		float			fSpeed;
		float			fCheckPointRadius;
		bool			bEnableAvoidance;
		bool			bEnableShortcut;
		bool			bEnableSlowingDown;
		bool			bEnableStop;

		IBotAgentObserver* pObserver;
	};

	rcBotAgent();
public:
	rcBotAgent(const char* name, const char* worldName, dtNavMesh* mesh, dtNavMeshQuery* query, dtCrowd* crowd, dtTileCache* tileCache, const InitConfig&);
	~rcBotAgent(void);

	void			Update(float fSeconds);
	bool			Goto(const float* dest, float fSpeed, const float* queryExtents = 0);
	void			Stop();
	void			PauseMoving();
	void			ResumMoving();
	void			SetPosition(const float* pos);
	const float*	GetPosition() const;
	void			SetSpeed(float fSpeed);
	const float*	GetSpeed()const;
	void			SetEnableAvoidence(bool bEnable);


protected:
	bool			HasArrvie() const;
	void			CreateCrowdAgent();
	bool			IsMovingOnLivePath();
private:

	dtNavMesh*			m_navMesh;
	dtNavMeshQuery*		m_navQuery;
	dtCrowd*			m_navCrowd;
	dtTileCache*		m_tileCache;
	InitConfig			m_initConf;

	AgentHandle			m_hAgent;
};
