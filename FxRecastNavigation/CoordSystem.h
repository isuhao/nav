//------------------------------------------------------------------------------
// �ļ���:		CoordSystem.h
// ��  ��:		����ϵͳת��
// ˵  ��:		
// ��������:	2014��4��25��
// ������:		����־
// ��ע: 
// ��Ȩ����:	������ţ�������޹�˾
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
