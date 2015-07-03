//------------------------------------------------------------------------------
// 文件名:		CoordSystem.cpp
// 内  容:		坐标系统转换
// 说  明:		
// 创建日期:	2014年4月25日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#include "CoordSystem.h"

const float COORD_AXIS_TRANS[] = {1.f,1.f,1.f};
CoordSystem::CoordSystem(void)
{
}

CoordSystem::~CoordSystem(void)
{
}

void CoordSystem::ClientToRecastNavPos( float* client )
{
	client[0] *= COORD_AXIS_TRANS[0];
	client[1] *= COORD_AXIS_TRANS[1];
	client[2] *= COORD_AXIS_TRANS[2];
}

float* CoordSystem::ClientToRecastNavPos( const float* client, float* res )
{
	res[0] = client[0] * COORD_AXIS_TRANS[0];
	res[1] = client[1] * COORD_AXIS_TRANS[1];
	res[2] = client[2] * COORD_AXIS_TRANS[2];
	return res;
}


void CoordSystem::RecastNavToClientPos(float* pos)
{
	pos[0] *= COORD_AXIS_TRANS[0];
	pos[1] *= COORD_AXIS_TRANS[1];
	pos[2] *= COORD_AXIS_TRANS[2];
}

float* CoordSystem::RecastNavToClientPos(const float* pos, float* res)
{
	res[0] = pos[0] * COORD_AXIS_TRANS[0];
	res[1] = pos[1] * COORD_AXIS_TRANS[1];
	res[2] = pos[2] * COORD_AXIS_TRANS[2];
	return res;
}