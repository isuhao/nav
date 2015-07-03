//------------------------------------------------------------------------------
// �ļ���:		rcWorldObject.h
// ��  ��:		
// ˵  ��:		
// ��������:	2014��6��30��
// ������:		����־
// ��ע: 
// ��Ȩ����:	������ţ�������޹�˾
//------------------------------------------------------------------------------
#ifndef _RCWORLDOBJECT_H_
#define _RCWORLDOBJECT_H_
#include <string>

class rcWorldObject
{
	rcWorldObject(const rcWorldObject&);
	rcWorldObject& operator = (const rcWorldObject&);
	rcWorldObject();
public:
	rcWorldObject(const char* name, const char* worldName):m_strName(name),m_strWorldName(worldName){}
	virtual ~rcWorldObject(){}

	const char*	GetName() const { return m_strName.c_str(); }
	const char*	GetWorldName() const { return m_strWorldName.c_str(); }
private:

	std::string		m_strName;
	std::string		m_strWorldName;
};
#endif	//_RCWORLDOBJECT_H_
