#pragma once
#include "debugdraw.h"
#include <list>
#include <vector>

class duDebugDrawPool :	public duDebugDraw
{
	struct Vertex
	{
		float pos[3];
		unsigned int color;
	};
	struct Geometry
	{
		duDebugDrawPrimitives type;
		unsigned int start;
	};
public:
	virtual void depthMask(bool state);
	virtual void texture(bool state);
	virtual void begin(duDebugDrawPrimitives prim, float size = 1.0f);
	virtual void vertex(const float* pos, unsigned int color);
	virtual void vertex(const float x, const float y, const float z, unsigned int color);
	virtual void vertex(const float* pos, unsigned int color, const float* uv);
	virtual void vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v);
	virtual void end();
private:
	std::vector<Vertex> m_vertes;
	std::list<Geometry> m_geometrys;

};
