#include "rcObstacle.h"

rcObstacle::rcObstacle(const char* szType, const char* name, const char* worldName, dtTileCache*tileCache):
rcWorldObject(name, worldName)
,m_szType(szType)
,m_dtRef(0)
,m_dtTileCache(tileCache){}

rcObstacle::~rcObstacle(void)
{
	m_dtTileCache->removeObstacle(m_dtRef);
}

rcCylinderObstacle::rcCylinderObstacle(const char* name, const char* worldName,  dtTileCache* tileCache, float* pos, float fRadius, float fHeight ):
rcObstacle("Cylinder", name, worldName, tileCache)
{
	m_dtTileCache->addObstacle(pos, fRadius, fHeight, &m_dtRef);
}

rcCylinderObstacle::~rcCylinderObstacle(){}

rcBoxObstacle::rcBoxObstacle(const char* name, const char* worldName,  dtTileCache* tileCache, float* pos, float*boxHalfExtents, float fRotate ):
rcObstacle("Box", name, worldName, tileCache)
{
	m_dtTileCache->addObstacle(pos, boxHalfExtents, fRotate, &m_dtRef);
}

rcBoxObstacle::~rcBoxObstacle(){}