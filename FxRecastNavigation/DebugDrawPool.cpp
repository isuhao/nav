//------------------------------------------------------------------------------
// 文件名:		DebugDrawImpl.cpp
// 内  容:		
// 说  明:		
// 创建日期:	2014年4月25日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#include "DebugDrawPool.h"
#include <string.h>

DebugDrawPool::DebugDrawPool(int cap) :
m_pos(0),
m_color(0),
m_size(0),
m_cap(0),
m_depthMask(true),
m_prim(DU_DRAW_LINES),
m_primSize(1.0f)
{
	if (cap < 8)
		cap = 8;
	resize(cap);
}

DebugDrawPool::~DebugDrawPool()
{
	delete [] m_pos;
	delete [] m_color;
}

void DebugDrawPool::resize(int cap)
{
	float* newPos = new float[cap*3];
	if (m_size)
		memcpy(newPos, m_pos, sizeof(float)*3*m_size);
	delete [] m_pos;
	m_pos = newPos;

	unsigned int* newColor = new unsigned int[cap];
	if (m_size)
		memcpy(newColor, m_color, sizeof(unsigned int)*m_size);
	delete [] m_color;
	m_color = newColor;

	m_cap = cap;
}

void DebugDrawPool::clear()
{
	m_size = 0;
}

void DebugDrawPool::depthMask(bool state)
{
	m_depthMask = state;
}

void DebugDrawPool::begin(duDebugDrawPrimitives prim, float size)
{
	clear();
	m_prim = prim;
	m_primSize = size;
}

void DebugDrawPool::vertex(const float x, const float y, const float z, unsigned int color)
{
	if (m_size+1 >= m_cap)
		resize(m_cap*2);
	float* p = &m_pos[m_size*3];
	p[0] = x;
	p[1] = y;
	p[2] = z;
	m_color[m_size] = color;
	m_size++;
}

void DebugDrawPool::vertex(const float* pos, unsigned int color)
{
	vertex(pos[0],pos[1],pos[2],color);
}

void DebugDrawPool::end()
{
}

void DebugDrawPool::draw(class NaviGeoRender* render)
{
#if 0
	if (!dd) return;
	if (!m_size) return;
	dd->depthMask(m_depthMask);
	dd->begin(m_prim, m_primSize);
	for (int i = 0; i < m_size; ++i)
		dd->vertex(&m_pos[i*3], m_color[i]);
	dd->end();
#endif
}

