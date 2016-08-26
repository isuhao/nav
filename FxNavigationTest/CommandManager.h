#pragma once
#include <sstream>
#include <map>
#include <list>



class NaviVector3;

class CommandTask
{
public:
	bool			Parse(std::stringstream& strm,float fStart = -1.0,float fTime=-1.f,  int nCount=0);
	bool			UpdateTime(float fTime);
	bool			HasFinished() const { return m_nExecutedTimes>=m_nCount; }

	virtual void	OnPathFindDone(const char* szName, int res, NaviVector3* nodes, unsigned int count){}
	virtual void	OnBotSpeedChange(const char* szName, const NaviVector3& curSpeed, const NaviVector3& preSpeed, const NaviVector3& desireDest){}
	virtual void	OnBotArrive(const char* szName, const NaviVector3& curPos, const NaviVector3& desPos ){}
protected:
	virtual bool	ParseCommand(std::stringstream& strm) { return true; }

	bool			ParseVector(std::string& strPos, NaviVector3&out);

private:
	virtual bool	Execute() = 0;
	float		m_fStartTime;
	float		m_fTime;
	int			m_nCount;

	int			m_nExecutedTimes;
	float		m_fLeftTime;

};


class CommandTaskFactory
{
	static std::map<std::string, CommandTaskFactory*> m_creator;
	CommandTaskFactory(const CommandTaskFactory& rhs);
public:

	CommandTaskFactory(const char* name){ m_creator[name]=this; }
	static CommandTaskFactory* GetFactory(const char* name) 
	{
		std::map<std::string, CommandTaskFactory*>::iterator it = m_creator.find(name);
		return it!= m_creator.end()?it->second : NULL;
	}

	virtual CommandTask* CreateTask() = 0;
};

template<class TaskType>
class NaviTaskFactory : public CommandTaskFactory
{
	static NaviTaskFactory<TaskType> m_hInstance;

public:
	NaviTaskFactory(const char* name):CommandTaskFactory(name){}
	virtual CommandTask* CreateTask(){ return new TaskType; }
};


class CommandManager : public INavigationObserver
{
	std::list<CommandTask*> m_taskList;
	CommandManager(const CommandManager&);
public:
	CommandManager(void);
	~CommandManager(void);


	void Step(float fTime);
	bool ExecuteTask(const char* str);
	void OnPathFindDone(const char* szName, int res, NaviVector3* nodes, unsigned int count);
	void OnBotSpeedChange(const char* szName, const NaviVector3& curSpeed, const NaviVector3& preSpeed, const NaviVector3& desireDest);
	void OnBotArrive(const char* szName, const NaviVector3& curPos, const NaviVector3& desPos );
};


CommandManager& GetCommManager();