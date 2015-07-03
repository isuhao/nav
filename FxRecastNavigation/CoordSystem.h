//------------------------------------------------------------------------------
// 文件名:		CoordSystem.h
// 内  容:		坐标系统转换
// 说  明:		
// 创建日期:	2014年4月25日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#pragma once

class CoordSystem
{
public:
	CoordSystem(void);
	~CoordSystem(void);
	void	ClientToRecastNavPos(float* client);
	float*	ClientToRecastNavPos(const float* client, float* res);
	void	RecastNavToClientPos(float* pos);
	float*	RecastNavToClientPos(const float* pos, float* res);

};
