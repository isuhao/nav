//------------------------------------------------------------------------------
// 文件名:		INavigationManager.h
// 内  容:		navigation寻路模块接口
// 说  明:		
// 创建日期:	2013年12月30日
// 创建人:		余祖志
// 备注: 
// 版权所有:	苏州蜗牛电子有限公司
//------------------------------------------------------------------------------
#ifndef __INAVIGATIONMANAGER_H_
#define __INAVIGATIONMANAGER_H_

#if defined (_WIN32)
#define NAVI_DLL_EXPORT __declspec(dllexport) 
#else
#define NAVI_DLL_EXPORT
#endif

typedef unsigned long NAVIGEOCOLOR;

typedef struct
{
	float x;	/*!< x coordinate */
	float y;	/*!< y coordinate */
	float z;	/*!< z coordinate */
} NAVIVECTOR3;



typedef struct NAVIMATRIX
{
#ifdef __cplusplus
public:
	float* operator [] ( const int Row )
	{
		return &f[Row<<2];
	}
#endif //__cplusplus
	union
	{
		struct {
			float        _11, _12, _13, _14;
			float        _21, _22, _23, _24;
			float        _31, _32, _33, _34;
			float        _41, _42, _43, _44;

		};
		float m[4][4];
		float f[16];	/*!< Array of float */
	};
}NaviMatrix4, *PNaviMatrix4;


class NaviVector3 : public NAVIVECTOR3
{
public:
	NaviVector3(){ x=y=z=0.0f; }
	NaviVector3(float fX,float fY, float fZ){ x=fX; y=fY; z=fZ; }
};

class NaviFileIO
{
public:
	virtual ~NaviFileIO(){}
	virtual bool openForWrite(const char* path) = 0;
	virtual bool openForRead(const char* path) = 0;
	virtual bool seek(long offset, int orign) = 0;//orign:0-begin;1-curr;2-end
	virtual long tell() = 0;
	virtual bool isWriting() const = 0;
	virtual bool isReading() const = 0;
	virtual bool write(const void* ptr, const long size) = 0;
	virtual bool read(void* ptr, const long size) = 0;
	virtual void close() = 0;
};

struct NaviBotInitConfig
{
	NaviBotInitConfig(){SetDefault();}
	void SetDefault()
	{
		m_fSpeed = 2.f;
		m_fRadius = 0.7f;
		m_fCheckPointRadius = 0.05f;
		m_bEnableAvoidence = false;
		m_bEnableShortcut = false;
		m_bEnableSlowingDown = true;
		m_bEnableStop = false;

	}
	NaviVector3 m_vecPos;	//position
	float		m_fSpeed;	//move speed
	float		m_fRadius;	//radius
	float		m_fCheckPointRadius;
	bool		m_bEnableAvoidence;	//avoidence
	bool		m_bEnableShortcut;	//short cut dynamic
	bool		m_bEnableSlowingDown;
	bool		m_bEnableStop;
};

struct NaviBotGotoConfig
{
	NaviVector3 m_destPosition;	//dest
	float		m_fSpeed;		//speed
};

class NaviBodyInitConfig
{
public:
	NaviBodyInitConfig(const char* shapeName):m_szShapeName(shapeName),m_bTriggerTagVolume(false){}
	const char* GetShape() const { return m_szShapeName; }

	bool m_bTriggerTagVolume;

private:
	NaviBodyInitConfig();
	const char* m_szShapeName;
};


class NaviCylinderBodyInitConfig : public NaviBodyInitConfig
{
public:
	NaviCylinderBodyInitConfig():NaviBodyInitConfig("Cylinder"),
		m_Position(0.f,0.f,0.f),m_linearVelocity(0.f,0.f,0.f),m_fRadius(1.f),m_fHeight(1.f){}
	NaviVector3 m_Position;
	NaviVector3 m_linearVelocity;
	float		m_fRadius;
	float		m_fHeight;
};

class NaviBoxBodyInitConfig: public NaviBodyInitConfig
{
public:
	NaviBoxBodyInitConfig():NaviBodyInitConfig("Box"),
		m_Position(0.f,0.f,0.f),m_linearVelocity(0.f,0.f,0.f),m_angularVelocity(0.f,0.f,0.f),m_boxHalfExtents(1.f,1.f,1.f){}
	NaviVector3 m_Position;
	NaviVector3 m_rotate;
	NaviVector3 m_linearVelocity;
	NaviVector3 m_angularVelocity;
	NaviVector3 m_boxHalfExtents;
};


struct NaviBodyGotoConfig
{
	NaviVector3 m_destPosition;	//dest
	float		m_linearSpeed;	//linear speed
	NaviVector3 m_destOrign;	//angle
	float		m_angularSpeed;	//angular speed
};

//
class INavigationObserver
{
public:
	virtual void OnPathFindDone(const char* szName, int res, NaviVector3* nodes, unsigned int count) = 0;
	virtual void OnBotSpeedChange(const char* szName, 
		const NaviVector3& curSpeed, const NaviVector3& preSpeed, const NaviVector3& desireDest) = 0;
	virtual void OnBotArrive(const char* szName, const NaviVector3& curPos, const NaviVector3& desPos ) = 0;
	virtual void OnLogMessage(const char* szLogContent){}
};


/**/
class NaviWorldInitConfig
{
public:
	NaviWorldInitConfig():m_naviDataCount(0),m_nMaxBotCount(100){}
	~NaviWorldInitConfig(){Clear();}
	static const int	MAX_PATHLEN		= 512;
	static const int	NAVIDATA_COUNT	= 20;
	NAVI_DLL_EXPORT 
	bool		AddNaviDataPath(const char* szPathFile);
	int			GetCount() const { return m_naviDataCount; }
	const char* GetNaviDataPath(int idx) const { return m_strNaviData[idx]; }
	NAVI_DLL_EXPORT 
	void		Clear();
	int			m_nMaxBotCount;	//max bot count(0 or more and default is 100)
private:
	char* m_strNaviData[NAVIDATA_COUNT];//[MAX_PATHLEN];
	int m_naviDataCount;
};


/*debug geometry rendering*/
class NaviGeoRender
{
public:
	enum {
		GEOTYPE_NAVIMESH	= 1,			//navmesh
		GEOTYPE_BODY		= 1<<1,			//body
		GEOTYPE_BOT			= 1<<2,			//bot
		GEOTYPE_TRAJECTORY	= 1<<3,			//trajectory line
		GEOTYPE_ALL			= 0xFFFFFFF,	//all

	};

	struct Vertex
	{
		NaviVector3	pos;
		NAVIGEOCOLOR		color; 
	};

	struct Face
	{
		Vertex vec1,vec2,vec3;
	};

					NaviGeoRender(int filter) : m_filter(filter){}
	virtual void	OnGeometry(const NaviMatrix4&, const Face*, unsigned int count, unsigned int stride) const {}
	virtual void	OnLineStrip(const NaviMatrix4&, const Vertex*, unsigned int count, unsigned int stride) const {}
	unsigned int	GetFilter() const { return m_filter; }
private:
	unsigned int m_filter;
};

struct NaviMgrInitConfig
{
	NaviMgrInitConfig():m_pObserver(0),m_pFileIO(0),m_bVisualDebug(false){}
	INavigationObserver*	m_pObserver;
	NaviFileIO*				m_pFileIO;
	bool						m_bVisualDebug;
};

class INavigationManager
{
public:
	virtual bool	Initialize(const NaviMgrInitConfig& conf) = 0;
	virtual bool	IsInitialized() = 0;

	virtual void	ShowVisualDebug(bool bShow) = 0;
	virtual bool	HasWorld( const char* name ) = 0;
	virtual bool	CreateWorld(const char* world, const NaviWorldInitConfig& conf) = 0;
	virtual void	DestroyWorld(const char* world) = 0;
	virtual bool	AddNaviData(const char* worldName, const char* szNaviFile) = 0;
	virtual bool	RemoveNaviData( const char* worldName, const char* szNaviFile) = 0;

	virtual void	Update(float fSecond) = 0;
	virtual void	Clear() = 0;
	virtual void	Realize(const NaviGeoRender* render) = 0;

	virtual bool	BotCreate(const char* world, const char* name, const NaviBotInitConfig& conf) = 0;
	virtual void	BotRemove(const char* name) = 0;
	virtual bool	BotHas(const char* name) = 0;
	virtual bool	BotCango(const char* name, const NaviVector3& dest) = 0;
	virtual bool	BotGoto(const char* name, const NaviBotGotoConfig& conf) = 0;
	virtual bool	BotStop(const char* name) = 0;

	virtual void	BotSetPosition(const char* name, const NaviVector3& pos) = 0;
	virtual bool	BotGetPosition(const char* name, NaviVector3& pos) = 0;
	virtual bool	BotGetSpeed(const char* name, NaviVector3& speed) = 0;
	virtual bool	BotSetSpeed(const char* name, float fSpeed) = 0;
	virtual bool	BotPauseMoving(const char*name) = 0;
	virtual bool	BotResumMoving(const char*name) = 0;
	virtual void	BotSetEnableAvoidance(const char* name, bool bEnable) = 0;

	virtual bool	BodyCreate(const char* world, const char* name, const NaviBodyInitConfig& conf) = 0;
	virtual void	BodyRemove(const char* name) = 0;
	virtual bool	BodyHas(const char* name) = 0;
	virtual void	BodyGoto(const char* name, const NaviBodyGotoConfig& conf) = 0;
	virtual void	BodySetPosition(const char* name, const NaviVector3& pos) = 0;
	virtual void	BodySetSpeed(const char* name, const NaviVector3& linerspeed, const NaviVector3& angularSpeed ) = 0;

	virtual bool	QueryCanGo(const char* world, const NaviVector3& from, const NaviVector3& to, float propagationBoxExtent, NaviVector3**pathout = 0, int* nNodeCount = 0) = 0;
	virtual bool	QueryRayCanGo(const char* world, const NaviVector3& from, const NaviVector3& to, NaviVector3* hitPos=0) = 0;
	virtual bool	QueryNearestPosOnGround(const char* world, const NaviVector3& pos, NaviVector3& precisePos) = 0;

};

NAVI_DLL_EXPORT INavigationManager* GetNavigationManager(int user = 0);


#endif	//__INAVIGATIONMANAGER_H_
