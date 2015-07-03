#pragma once
#include "DetourTileCache.h"
#include "rcWorldObject.h"


class rcObstacle : public rcWorldObject
{
	enum{ INVALID_OBSTACLE_HANDLE = 0 };
	rcObstacle(const rcObstacle&);
	rcObstacle& operator =(const rcObstacle&);
public:
	rcObstacle(const char* szType, const char* name, const char* worldName, dtTileCache*tileCache);
	virtual ~rcObstacle(void);

	const char*	GetType() const { return m_szType; }
protected:
	dtObstacleRef	m_dtRef;
	dtTileCache*	m_dtTileCache;
	const char*		m_szType;
};


class rcCylinderObstacle : public rcObstacle
{
public:
	rcCylinderObstacle(const char* name, const char* worldName, dtTileCache* tileCache, float* pos, float fRadius, float fHeight);
	~rcCylinderObstacle();
};

class rcBoxObstacle : public rcObstacle
{
public:
	rcBoxObstacle(const char* name, const char* worldName, dtTileCache* tileCache, float* pos, float*boxHalfExtents, float fRotate);
	~rcBoxObstacle();
};