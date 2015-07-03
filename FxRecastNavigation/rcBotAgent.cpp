#include "rcBotAgent.h"
#include "Recast.h"
#include <string.h>
const float rcBotAgent::VECTOR_ZERO[] = {0.f, 0.f, 0.f};

rcBotAgent::rcBotAgent( const char* name, const char* worldName, dtNavMesh* mesh, dtNavMeshQuery* query, dtCrowd* crowd, dtTileCache* tileCache, const InitConfig& conf):
rcWorldObject(name, worldName)
,m_navMesh(mesh)
,m_navQuery(query)
,m_navCrowd(crowd)
,m_tileCache(tileCache)
,m_hAgent(INVALID_AGENT_HANDLE)
{
	memcpy(&m_initConf, &conf, sizeof(InitConfig));
	CreateCrowdAgent();
}


void rcBotAgent::CreateCrowdAgent()
{
	static const char AGENT_OBSTACLEAVOIDANCETYPE = 3;

	dtCrowd* crowd = m_navCrowd;

	dtCrowdAgentParams ap;
	memset(&ap, 0, sizeof(ap));
	ap.radius = m_initConf.fRadius;
	ap.slowdownRadius = m_initConf.fRadius*0.5f;
	ap.height = 1.5f;
	ap.maxAcceleration = 1000000.f;
	ap.maxSpeed = m_initConf.fSpeed;
	ap.collisionQueryRange = ap.radius * 12.0f;
	ap.pathOptimizationRange = ap.radius * 30.0f;
	ap.updateFlags = 0; 
	
	if(m_initConf.bEnableShortcut)
		ap.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
	ap.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
	ap.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
	if(m_initConf.bEnableAvoidance)
		ap.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
	//ap.updateFlags |= DT_CROWD_SEPARATION;
	ap.obstacleAvoidanceType = AGENT_OBSTACLEAVOIDANCETYPE;
	//ap.separationWeight = 2.0f;

	m_hAgent = crowd->addAgent(m_initConf.Position, &ap);
}

rcBotAgent::~rcBotAgent(void)
{
	m_navCrowd->removeAgent(m_hAgent);
	m_navMesh	= 0;
	m_navQuery	= 0;
	m_navCrowd	= 0;
	m_tileCache = 0;
	memset(&m_initConf, 0, sizeof(InitConfig));
}

void rcBotAgent::Update( float fSeconds )
{
	const dtCrowdAgent* agentPtr = m_navCrowd->getAgent(m_hAgent);

	//stop if arrvied
	if ( IsMovingOnLivePath() && HasArrvie() ) 
	{
		const dtCrowdAgent* agentPtr = m_navCrowd->getAgent(m_hAgent);
		if(m_initConf.pObserver)
			m_initConf.pObserver->OnBotArrive(this, agentPtr->npos, agentPtr->targetPos);
		Stop();
	}
}

bool rcBotAgent::Goto( const float* dest, float fSpeed, const float* queryExtents/* = 0*/)
{
	const dtCrowdAgent* agent = m_navCrowd->getAgent(m_hAgent);
	dtCrowdAgentParams param;
	memcpy(&param, &agent->params, sizeof(dtCrowdAgentParams));
	param.maxSpeed = fSpeed;
	m_navCrowd->updateAgentParameters(m_hAgent, &param);

	const float* findPosQueryExtents = queryExtents!=0?queryExtents:m_navCrowd->getQueryExtents();

	dtPolyRef refTarget;
	float targetPos[3];
	if(dtStatusFailed(m_navQuery->findNearestPoly(dest, 
		findPosQueryExtents, m_navCrowd->getFilter(),&refTarget, targetPos)))
		return false;


	return m_navCrowd->requestMoveTarget(m_hAgent, refTarget, targetPos);
}

void rcBotAgent::Stop()
{
	m_navCrowd->requestMoveVelocity(m_hAgent, VECTOR_ZERO);
	//m_navCrowd->resetMoveTarget(m_hAgent);
}

void rcBotAgent::PauseMoving()
{

}

void rcBotAgent::ResumMoving()
{

}

void rcBotAgent::SetPosition( const float* pos )
{
	const dtCrowdAgent* agentPtr = m_navCrowd->getAgent(m_hAgent);
	dtCrowdAgentParams agentParam;
	memcpy(&agentParam, &agentPtr->params, sizeof(dtCrowdAgentParams));

	m_navCrowd->removeAgent(m_hAgent);
	m_hAgent = m_navCrowd->addAgent(pos, &agentParam);
}

const float* rcBotAgent::GetPosition() const
{
	const dtCrowdAgent* agentPtr = m_navCrowd->getAgent(m_hAgent);

	return agentPtr->npos;
}

void rcBotAgent::SetSpeed( float fSpeed )
{
	const dtCrowdAgent* agent = m_navCrowd->getAgent(m_hAgent);
	dtCrowdAgentParams param;
	memcpy(&param, &agent->params, sizeof(dtCrowdAgentParams));
	param.maxSpeed = fSpeed;
	m_navCrowd->updateAgentParameters(m_hAgent, &param);
}
const float* rcBotAgent::GetSpeed() const
{
	const dtCrowdAgent* agentPtr = m_navCrowd->getAgent(m_hAgent);
	return agentPtr->vel;
}

//#include <stdio.h>
bool rcBotAgent::HasArrvie() const
{
	const dtCrowdAgent* agentPtr = m_navCrowd->getAgent(m_hAgent);
	float dist[3];
	rcVsub(dist, agentPtr->targetPos, agentPtr->npos);
	return (dist[0]*dist[0]+dist[2]*dist[2]) < m_initConf.fCheckPointRadius*m_initConf.fCheckPointRadius;
}

bool rcBotAgent::IsMovingOnLivePath()
{
	const dtCrowdAgent* agentPtr = m_navCrowd->getAgent(m_hAgent);
	return (agentPtr->targetState==DT_CROWDAGENT_TARGET_VALID);
}

void rcBotAgent::SetEnableAvoidence( bool bEnable )
{
	const dtCrowdAgent* agent = m_navCrowd->getAgent(m_hAgent);
	dtCrowdAgentParams param;
	memcpy(&param, &agent->params, sizeof(dtCrowdAgentParams));
	param.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
	m_navCrowd->updateAgentParameters(m_hAgent, &param);
}