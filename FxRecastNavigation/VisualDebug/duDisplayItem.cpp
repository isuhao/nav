//------------------------------------------------------------------------------
// �ļ���:		duDisplayItem.cpp
// ��  ��:		
// ˵  ��:		
// ��������:	2014��5��19��
// ������:		����־
// ��ע: 
// ��Ȩ����:	������ţ�������޹�˾
//------------------------------------------------------------------------------
#include "duDisplayItem.h"
#include <string.h>


duDisplayItem::duDisplayItem(int cap) :
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

duDisplayItem::~duDisplayItem()
{
	delete [] m_pos;
	delete [] m_color;
}

void duDisplayItem::resize(int cap)
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

void duDisplayItem::clear()
{
	m_size = 0;
}

void duDisplayItem::depthMask(bool state)
{
	m_depthMask = state;
}

void duDisplayItem::begin(duDebugDrawPrimitives prim, float size)
{
	clear();
	m_prim = prim;
	m_primSize = size;
}

void duDisplayItem::vertex(const float x, const float y, const float z, unsigned int color)
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

void duDisplayItem::vertex(const float* pos, unsigned int color)
{
	vertex(pos[0],pos[1],pos[2],color);
}

void duDisplayItem::end()
{
}

void duDisplayItem::draw(struct duDebugDraw* dd)
{
	if (!dd) return;
	if (!m_size) return;
	dd->depthMask(m_depthMask);
	dd->begin(m_prim, m_primSize);
	for (int i = 0; i < m_size; ++i)
		dd->vertex(&m_pos[i*3], m_color[i]);
	dd->end();
}

