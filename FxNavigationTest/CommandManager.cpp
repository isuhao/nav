#include "StdAfx.h"
#include "INavigationManager.h"
#include "CommandManager.h"
#include <string>
#include <sstream>
#include <iostream>
#include <math.h>
#define _Decl_CommandFactory(name) NaviTaskFactory<name> NaviTaskFactory<name>::m_hInstance(#name)





bool CommandTask::UpdateTime( float fTime )
{
	if ( m_fStartTime>=0 )
	{
		m_fStartTime -= fTime;
		return true;
	}

	m_fLeftTime -= fTime;
	if (m_fLeftTime<0)
	{
		m_fLeftTime += m_fTime;
		if(!Execute())
			return false;
		m_nExecutedTimes++;
	}
	return true;
}

bool CommandTask::Execute()
{
	std::stringstream strm("");
	std::string strComm;

	strm>>strComm;


	std::string strWorldName;
	std::string strPosition;
	std::string strHalfBoxSize;


#if 0

	
#endif


	return true;
}

bool CommandTask::ParseVector(std::string& strPos, NaviVector3&out)
{
	return sscanf(strPos.c_str(), "vector(%f,%f,%f)", &out.x, &out.y, &out.z)==3;
}


bool CommandTask::Parse(std::stringstream& strm,float fStart/*=-1.0*/,float fTime/*=-1.f*/,  int nCount/*=0*/)
{
	m_fStartTime	= fStart;
	m_nCount		= nCount<1? 1 : nCount;
	m_fTime			= fTime;


	m_nExecutedTimes = 0;
	m_fLeftTime = fTime;
	if(!ParseCommand(strm))
		return false;

	if (fStart<0)
		return Execute();

	return true;
}


std::map<std::string, CommandTaskFactory*> CommandTaskFactory::m_creator;


class Initialize : public CommandTask
{
	NaviMgrInitConfig conf;
	virtual bool ParseCommand(std::stringstream& strm)
	{
		std::string strVisualDbg;
		conf.m_bVisualDebug = (strm>>strVisualDbg, strVisualDbg)=="true";
		conf.m_pObserver = &GetCommManager();
		return true;
	}

	virtual bool Execute()
	{
		GetNavigationManager()->Initialize(conf);
		return true;
	}

};

class CreateWorld : public CommandTask
{
	std::string			m_strWorldName;
	NaviWorldInitConfig m_conf;

	virtual bool ParseCommand(std::stringstream& strm)
	{
		std::string strWorldPath;

		strm>>m_strWorldName>>strWorldPath;

		m_conf.AddNaviDataPath(strWorldPath.c_str());
		return true;
	}

	virtual bool	Execute()
	{
		return GetNavigationManager()->CreateWorld(m_strWorldName.c_str(), m_conf);
	}

};

class BotCreate : public CommandTask
{
	NaviBotInitConfig m_conf;

	std::string m_strWorldName;
	std::string m_strBotName;
public:

	virtual bool ParseCommand(std::stringstream& strm)
	{
		std::string strPosition;
		strm>>m_strWorldName>>m_strBotName;
		m_conf.m_fSpeed = 3;
		m_conf.m_bEnableAvoidence = true;
		if ( !ParseVector((strm>>strPosition,strPosition), m_conf.m_vecPos) )
		{
			std::cout<<"error:Exec "<<strm.str().c_str()<<" parse vector failed!!!";
			return false;
		}

		return true;
	}

	virtual bool Execute()
	{
		return GetNavigationManager()->BotCreate(m_strWorldName.c_str(), m_strBotName.c_str(), m_conf);
	}
};



class BotGoto : public CommandTask
{
	NaviBotGotoConfig	m_conf;
	std::string			m_strBotName;
	NaviVector3			m_startPos;

public:
	virtual bool ParseCommand(std::stringstream& strm)
	{
		std::string strPosition;
		strm>>m_strBotName;
		strm>>m_conf.m_fSpeed;
		if ( !ParseVector((strm>>strPosition,strPosition), m_conf.m_destPosition) )
		{
			std::cout<<"error:Exec "<<strm.str().c_str()<<" parse vector failed!!!"<<std::endl;
			return false;
		}
		return true;
	}

	virtual bool Execute()
	{
		GetNavigationManager()->BotGetPosition(m_strBotName.c_str(), m_startPos);
		return GetNavigationManager()->BotGoto(m_strBotName.c_str(), m_conf);
	}
	virtual void	OnBotArrive(const char* szName, const NaviVector3& curPos, const NaviVector3& desPos )
	{
		if(m_strBotName!=szName)
			return;
		m_conf.m_destPosition = m_startPos;
		GetNavigationManager()->BotGetPosition(m_strBotName.c_str(), m_startPos);
		bool res = GetNavigationManager()->BotGoto(m_strBotName.c_str(), m_conf);
	}


};
class BotRemove : public CommandTask
{
	std::string strBotName;
	virtual bool ParseCommand(std::stringstream& strm)
	{
		strm>>strBotName;
		return true;
	}
	virtual bool Execute()
	{
		GetNavigationManager()->BotRemove(strBotName.c_str());
		return true;
	}
};

class BotStop : public CommandTask
{
	std::string strBotName;
	virtual bool ParseCommand(std::stringstream& strm)
	{
		strm>>strBotName;
		return true;
	}
	virtual bool Execute()
	{
		GetNavigationManager()->BotStop(strBotName.c_str());
		return true;
	}
};

class BodyRemove : public CommandTask
{
	std::string strBodyName;
	virtual bool ParseCommand(std::stringstream& strm)
	{
		strm>>strBodyName;
		return true;
	}
	virtual bool Execute()
	{
		GetNavigationManager()->BodyRemove(strBodyName.c_str());
		return true;
	}
};




class BodyCreate : public CommandTask
{

	std::string			m_strName;
	std::string			m_strWorldName;
	NaviBodyInitConfig* m_conf;

	virtual bool ParseCommand(std::stringstream& strm)
	{
		m_conf = NULL;
		std::string strShap;
		std::string strPosition;
		std::string strHalfBoxSize;
		strm>>m_strWorldName>>m_strName>>strShap;
		if (strShap=="Cylinder")
		{
			NaviCylinderBodyInitConfig* cylinder = new NaviCylinderBodyInitConfig;
			if( !ParseVector((strm>>strPosition,strPosition), cylinder->m_Position) )
			{
				std::cout<<"error:Exec "<<strm.str().c_str()<<" parse vector failed!!!"<<std::endl;
				return false;
			}
			strm>>cylinder->m_fHeight;
			strm>>cylinder->m_fRadius;

			m_conf = cylinder;
		}
		else if (strShap=="Box")
		{
			NaviBoxBodyInitConfig* box = new NaviBoxBodyInitConfig;
			if( !ParseVector((strm>>strPosition,strPosition), box->m_Position) )
			{
				std::cout<<"error:Exec "<<strm.str().c_str()<<" parse vector failed!!!"<<std::endl;
				return false;
			}
			if( !ParseVector((strm>>strHalfBoxSize,strHalfBoxSize), box->m_boxHalfExtents) )
			{
				std::cout<<"error:Exec "<<strm.str().c_str()<<" parse half box size failed!!!"<<std::endl;
				return false;
			}

			strm>>box->m_rotate.y;
			box->m_rotate.y = box->m_rotate.y/180.0f*3.1415926535898f;

			m_conf = box;
		}
		else
		{
			std::cout<<"error:Exec "<<strm.str().c_str()<<" parse shap failed!!!"<<std::endl;
			return false;
		}

		return true;
	}

	virtual bool	Execute()
	{
		if (m_conf==NULL)
			return false;

		return GetNavigationManager()->BodyCreate(m_strWorldName.c_str(), m_strName.c_str(), *m_conf);
	}


};



_Decl_CommandFactory(Initialize);
_Decl_CommandFactory(CreateWorld);
_Decl_CommandFactory(BotCreate);
_Decl_CommandFactory(BotRemove);
_Decl_CommandFactory(BotGoto);
_Decl_CommandFactory(BotStop);
_Decl_CommandFactory(BodyCreate);
_Decl_CommandFactory(BodyRemove);



CommandManager::CommandManager(void)
{
}

CommandManager::~CommandManager(void)
{
}

bool CommandManager::ExecuteTask( const char* str )
{
	std::stringstream strm(str);
	std::string strComm;
	strm>>strComm;



	float fStart = 0;
	float fTime = 0;
	int Count = 0;

	if(strComm.empty() || strComm.at(0)=='#')
		return true;

	int nTimeArgCount = sscanf(strComm.c_str(),"timer(%f,%f,%d)", &fStart, &fTime, &Count);

	CommandTaskFactory* factoryCreator = CommandTaskFactory::GetFactory( nTimeArgCount==0? strComm.c_str() : (strm>>strComm,strComm.c_str()) );
	if (!factoryCreator)
	{
		std::cout<<"error:can't find "<<strComm<<" command!"<<std::endl;
		return false;
	}
	CommandTask* task = factoryCreator->CreateTask();
	task->Parse(strm, fStart, fTime, Count);
	m_taskList.push_back(task);

	//CommandTaskFactory* factoryCreator = CommandTaskFactory::GetFactory( strComm.c_str() );
	//CommandTask* task = factoryCreator->CreateTask();
	//if (!factoryCreator)
	//{
	//	return false;
	//}
	//task->Parse(strm);
	//delete task;




	return true;
}

void CommandManager::Step( float fTime )
{
	std::list<CommandTask*>::iterator it = m_taskList.begin();
	while (it!=m_taskList.end())
	{
		if( !(*it)->HasFinished() )
		{
			(*it)->UpdateTime(fTime);
		}
		++it;
	}
}


void CommandManager::OnPathFindDone(const char* szName, int res, NaviVector3* nodes, unsigned int count)
{
	std::list<CommandTask*>::iterator it = m_taskList.begin();
	while (it!=m_taskList.end())
	{
		(*it)->OnPathFindDone(szName, res, nodes, count);
		++it;
	}
}
void CommandManager::OnBotSpeedChange(const char* szName, const NaviVector3& curSpeed, const NaviVector3& preSpeed, const NaviVector3& desireDest)
{
	std::list<CommandTask*>::iterator it = m_taskList.begin();
	while (it!=m_taskList.end())
	{
		(*it)->OnBotSpeedChange(szName, curSpeed, preSpeed, desireDest);
		++it;
	}
}
void CommandManager::OnBotArrive(const char* szName, const NaviVector3& curPos, const NaviVector3& desPos )
{
	NaviVector3 dir;
	dir.x = desPos.x - curPos.x;
	dir.y = desPos.y - curPos.y;
	dir.z = desPos.z - curPos.z;
	float length = sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);

	std::cout<<"OnBotArrive "<<szName<<" "<<length<<std::endl;
	std::list<CommandTask*>::iterator it = m_taskList.begin();
	while (it!=m_taskList.end())
	{
		(*it)->OnBotArrive(szName, curPos, desPos );
		++it;
	}
}


CommandManager& GetCommManager()
{
	static CommandManager mgr;
	return mgr;
}
