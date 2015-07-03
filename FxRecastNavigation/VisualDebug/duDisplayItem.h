//------------------------------------------------------------------------------
// 文件名:		duDisplayItem.h
// 内  容:		
// 说  明:		
// 创建日期:	2014年5月19日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#pragma once
#include "debugdraw.h"
#include "../VisualDebug.h"


class duDisplayItem : public duDebugDrawItem
{
	float* m_pos;
	unsigned int* m_color;
	int m_size;
	int m_cap;

	bool m_depthMask;
	duDebugDrawPrimitives m_prim;
	float m_primSize;

	void resize(int cap);

public:
	duDisplayItem(int cap = 512);
	~duDisplayItem();
	virtual void depthMask(bool state);
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color);

	virtual void texture(bool state){};
	virtual void vertex(const float* pos, unsigned int color, const float* uv){};
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v){};
	virtual void end();
	virtual void draw(struct duDebugDraw* dd);
	void clear();
};
