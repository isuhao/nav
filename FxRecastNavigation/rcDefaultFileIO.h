//------------------------------------------------------------------------------
// �ļ���:		rcDefaultFileIO.h
// ��  ��:		
// ˵  ��:		
// ��������:	2014��5��5��
// ������:		����־
// ��ע: 
// ��Ȩ����:	������ţ�������޹�˾
//------------------------------------------------------------------------------
#pragma once
#include "../Interface/INavigationManager.h"
#include <fstream>

class rcDefaultFileIO : public NaviFileIO
{
public:
	~rcDefaultFileIO();
	virtual bool openForWrite(const char* path);
	virtual bool openForRead(const char* path);
	virtual bool isWriting() const{return false;}
	virtual bool isReading() const{return false;}
	virtual bool seek(long offset, int orign);
	virtual long tell();
	virtual bool write(const void* ptr, const long size);
	virtual bool read(void* ptr, const long size);
	virtual void close();

private:
	void		 clear();
	std::fstream m_fstream;

	bool		 m_forWrite;

};

