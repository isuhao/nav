//------------------------------------------------------------------------------
// 文件名:		rcDefaultFileIO.cpp
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月5日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#include "rcDefaultFileIO.h"

#if _WIN32
rcDefaultFileIO::~rcDefaultFileIO(){ clear(); }
void rcDefaultFileIO::clear(){ close(); }


bool rcDefaultFileIO::openForWrite( const char* path )
{
	clear();
	m_forWrite = true;
	m_fstream.open(path, std::ios_base::out|std::ios_base::binary);
	return (!m_fstream.fail());
}

bool rcDefaultFileIO::openForRead( const char* path )
{
	clear();
	m_forWrite = false;
	m_fstream.open(path, std::ios_base::in|std::ios_base::binary);
	return (!m_fstream.fail());
}

bool rcDefaultFileIO::write( const void* ptr, const long size )
{
	if( !m_fstream.is_open() )
		return false;

	m_fstream.write((const char*)ptr, size);
	return (!m_fstream.fail());
}

bool rcDefaultFileIO::read( void* ptr, const long size )
{
	if( !m_fstream.is_open() )
		return false;
	m_fstream.read((char*)ptr, size);
	return (!m_fstream.fail());
}

void rcDefaultFileIO::close()
{
	m_fstream.close();
	m_fstream.clear(); 
}

bool rcDefaultFileIO::seek( long offset, int orign )
{
	if(m_forWrite)
	{
		if(orign==0)
			m_fstream.seekp(offset, std::ios::beg);
		else if(orign==1)
			m_fstream.seekp(offset, std::ios::cur);
		else if(orign==2)
			m_fstream.seekp(offset, std::ios::end);
	}
	else
	{
		if(orign==0)
			m_fstream.seekg(offset, std::ios::beg);
		else if(orign==1)
			m_fstream.seekg(offset, std::ios::cur);
		else if(orign==2)
			m_fstream.seekg(offset, std::ios::end);
	}

	return true;
}

long rcDefaultFileIO::tell()
{
	if(m_forWrite)
		return m_fstream.tellp();
	else
		return m_fstream.tellg();
}
#endif