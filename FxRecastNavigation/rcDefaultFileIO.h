//------------------------------------------------------------------------------
// 文件名:		rcDefaultFileIO.h
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月5日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
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

