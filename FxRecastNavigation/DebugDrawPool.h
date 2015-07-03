#pragma once
#include "DebugDraw.h"


class NaviGeoRender;
class DebugDrawPool : public duDebugDraw
{
public:
	DebugDrawPool(int);
	~DebugDrawPool(void);

	virtual void depthMask(bool state);
	virtual void texture(bool state){};
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv){}
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v){};
	virtual void end();

	void		draw(class NaviGeoRender* render);

private:
	void		resize(int cap);
	void		clear();
	float*					m_pos;
	unsigned int*			m_color;
	int						m_size;
	int						m_cap;
	bool					m_depthMask;
	duDebugDrawPrimitives	m_prim;
	float					m_primSize;


};