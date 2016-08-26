// FxNavigationTest.cpp : 定义应用程序的入口点。
//
#include "stdafx.h"
#include "main.h"
#include "INavigationManager.h"
#include "CommandManager.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <math.h>

#define USE_RECASTNAV
#ifdef USE_RECASTNAV
#pragma comment(lib, "FxRecastNavigation.lib")
#else
//#pragma comment(lib, "FxNavigation.lib")
#endif

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
HWND g_hWnd;

bool g_bQuite = false;

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

#ifdef USE_RECASTNAV
const char* PATH_GROUP[] = {
"D:/cygwin/Res/mobile/res/map/path/forest_s1_3_shourenmu/navmesh.input",
};
#else
const char* PATH_GROUP[] = 
{
// 	"F:\\res\\Client\\res\\map\\path\\F_aigeshanlu\\NavData\\zone_1_1",
// 	"F:\\res\\Client\\res\\map\\path\\F_aigeshanlu\\NavData\\zone_1_2",
// 	"F:\\res\\Client\\res\\map\\path\\F_aigeshanlu\\NavData\\zone_0_2",

// 	"F:\\res\\Client\\res\\map\\path\\F_aigeshanlu\\NavData\\zone_0_2",
// 	"F:\\res\\Client\\res\\map\\path\\F_aigeshanlu\\NavData\\zone_1_1",
// 	"F:\\res\\Client\\res\\map\\path\\F_aigeshanlu\\NavData\\zone_1_2",
	//"D:\\cygwin\\Res\\mobile\\res\\map\\path\\snowfield_s5_1_talashanmai\\NavData\\zone_0_0",
	//"D:\\cygwin\\Res\\mobile\\res\\map\\path\\forest_s0_0_xinshou\\NavData\\zone_0_1",
	//"D:\\cygwin\\Res\\mobile\\res\\map\\path\\forest_s0_0_xinshou\\NavData\\zone_1_1",
	"D:\\cygwin\\Res\\client\\res\\map\\path\\desert_s2_1_zhalakenhuangmo\\NavData\\zone_0_0",


	//"D:\\qqwe\\navigation\\win32_vc8_src\\solutions\\StudyTest\\NaviData\\Sector_A",
};
#endif

class NavigationObserver : public INavigationObserver
{
	virtual void OnPathFindDone(const char* szName, int res, NaviVector3* nodes, unsigned int count)
	{
		//std::cout<<"RoleBot "<<szName<<" Calcu path Success!"<<std::endl;
	}
	virtual void OnBotSpeedChange(const char* szName, 
		const NaviVector3& curSpeed, const NaviVector3& preSpeed, const NaviVector3& desireDest){}
	virtual void OnBotArrive(const char* szName, const NaviVector3& curPos, const NaviVector3& desPos )
	{
		//std::cout<<"RoleBot "<<szName<<" OnBotArrive!"<<std::endl;
		printf("RoleBot %s OnBotArrive! (%f,%f,%f)(%f,%f,%f)", szName, curPos.x, curPos.y, curPos.z, desPos.x, desPos.y, desPos.z);
	}
};


bool g_bPause = false;


DWORD WINAPI ThreadFun(LPVOID pM)  
{  
	do 
	{
		GetNavigationManager(1)->Update(0.03f);
	} while (true);
    return 0;  
}  


int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	AllocConsole();
 	freopen("conout$","w", stdout);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_FXNAVIGATIONTEST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
 	if (!InitInstance (hInstance, nCmdShow))
 	{
 		return FALSE;
 	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FXNAVIGATIONTEST));

#if 0
	static NavigationObserver Observer;
	NaviMgrInitConfig conf;
	conf.m_bVisualDebug = true;
	conf.m_pObserver = &Observer;
	GetNavigationManager()->Initialize(conf);
	conf.m_bVisualDebug = true;

	char szScenePath[255], szSceneName[255], szPathFileName[MAX_PATH];
	GetPrivateProfileStringA("recast", "scenepath", "", szScenePath, 255, "./FxNavigatoinTest.ini");
	GetPrivateProfileStringA("recast", "scenename", "", szSceneName, 255, "./FxNavigatoinTest.ini");
	SetDlgItemTextA(g_hWnd, IDC_EDIT_SCENEPATH, szScenePath);
	SetDlgItemTextA(g_hWnd, IDC_EDIT_SCENENAME, szSceneName);
	if ( szScenePath[0]!=0 && szSceneName!=0 )
	{
		sprintf_s(szPathFileName, "%s/%s/navmesh.input",szScenePath, szSceneName);
		NaviWorldInitConfig worldConf;
		worldConf.AddNaviDataPath(szPathFileName);
		GetNavigationManager()->CreateWorld(szSceneName, worldConf);
		//GetNavigationManager(1)->CreateWorld(szSceneName, worldConf);
	}
#else

	std::ifstream fileOpen;
	fileOpen.open("task.conf");
	if (!fileOpen.fail())
	{
		char szCommandBuff[2018];
		while (!fileOpen.eof())
		{
			fileOpen.getline(szCommandBuff, sizeof(szCommandBuff));
			GetCommManager().ExecuteTask(szCommandBuff);
		}
	}
#endif

	//CreateThread(NULL, 0, ThreadFun, NULL, 0, NULL);


	::SendDlgItemMessageA(g_hWnd, IDC_COMB_CREATEBODYTYPE, CB_ADDSTRING, 0, (LPARAM)("Box"));
	::SendDlgItemMessageA(g_hWnd, IDC_COMB_CREATEBODYTYPE, CB_ADDSTRING, 0, (LPARAM)("Cylinder"));
	::SendDlgItemMessageA(g_hWnd, IDC_COMB_CREATEBODYTYPE, CB_SETCURSEL, 0, 0);

	int nPreTickCount = GetTickCount();
	float fSecond = 0;
	// 主消息循环:
	while (!g_bQuite)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		if (WM_QUIT == msg.message)
			break;

		if(g_bPause)
			fSecond = 0;

		GetCommManager().Step(fSecond);
		GetNavigationManager()->Update(fSecond);
		static NaviGeoRender render(NaviGeoRender::GEOTYPE_ALL);

		{
			char buff255[255], szName[255];
			NaviVector3 speed;
			GetDlgItemTextA(g_hWnd, IDC_EDIT_BOTQUERYVALUE_NAME, szName, 255);
			if(GetNavigationManager()->BotGetSpeed(szName, speed))	{
				sprintf_s(buff255, "%f", sqrtf(speed.x*speed.x + speed.x*speed.x + speed.x*speed.x));			
				SetDlgItemTextA(g_hWnd, IDC_STATIC_QUERYRES, buff255);
			}
		}



		//GetNavigationManager()->Realize(&render);
		int timeDelta = GetTickCount() - nPreTickCount;
		if (20-timeDelta>0)Sleep(20-timeDelta);

		fSecond = (GetTickCount()-nPreTickCount) / 1000.0f;
		nPreTickCount = GetTickCount();
	}

	GetNavigationManager()->Clear();

	return (int) msg.wParam;
}
//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FXNAVIGATIONTEST));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_FXNAVIGATIONTEST);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中
 
//     hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
//        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), 0, About);

   if (!hWnd)
   {
      return FALSE;
   }

   g_hWnd = hWnd;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


NaviVector3& ParseStringToPos(const char* str, NaviVector3& out)
{
	float pos[3];
	char delm[255];
	std::stringstream strm(str);
	int index = 0;
	while(strm.getline(delm,255, ','))
		pos[index++] = (float)atof(delm);

#ifdef USE_RECASTNAV
	out.x = pos[0];
	out.y = pos[1];//*-1.f;
	out.z = pos[2];//*-1.f;
#else
	out.x = pos[0];
	out.y = pos[2];
	out.z = pos[1];
#endif

	return out;
}

std::set<std::string> allBotName;
INT_PTR CALLBACK OnButtonClick( int controlID, HWND hDlg, WPARAM wParam )
{
	switch (controlID)
	{
	case IDOK:
	case IDCANCEL:
		EndDialog(hDlg, LOWORD(wParam));
		return (INT_PTR)TRUE;
		break;
	case IDC_BTN_OPENVDBG:
		{
			char szScenePath[255], szSceneName[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_SCENENAME, szSceneName, 255);
			GetNavigationManager()->ShowVisualDebug(true);
		}
		break;
	case IDC_BTN_DESTROYWORLD:
	case IDC_BTN_CREATEWORLD:
		{
			char szScenePath[255], szSceneName[255], szPathFileName[MAX_PATH];
			GetDlgItemTextA(hDlg, IDC_EDIT_SCENEPATH, szScenePath, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_SCENENAME, szSceneName, 255);
			sprintf_s(szPathFileName, "%s/%s/navmesh.input",szScenePath, szSceneName);
			NaviWorldInitConfig worldConf;
			worldConf.AddNaviDataPath(szPathFileName);
			if (controlID==IDC_BTN_CREATEWORLD)
				GetNavigationManager()->CreateWorld(szSceneName, worldConf);
			else
				GetNavigationManager()->DestroyWorld(szSceneName);

			//GetNavigationManager(1)->CreateWorld("world", worldConf);

			WritePrivateProfileStringA("recast", "scenepath", szScenePath, "./FxNavigatoinTest.ini");
			WritePrivateProfileStringA("recast", "scenename", szSceneName, "./FxNavigatoinTest.ini");
		}
		break;
	case IDC_BTN_GOTO:
		{			
			char szName[255], szPosition[255], szSpeed[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTGOTO_NAME, szName, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTGOTO_POS, szPosition, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTGOTO_SPEED, szSpeed, 255);
			
			NaviBotGotoConfig conf;
			ParseStringToPos(szPosition, conf.m_destPosition);
			conf.m_fSpeed = szSpeed[0]? atof(szSpeed) : 1.f;

			bool res = true;
			if( IsDlgButtonChecked(hDlg, IDC_CHECK_ALLGOTO) )
			{
				std::set<std::string>::iterator it = allBotName.begin();
				for (; it!=allBotName.end(); ++it)
					GetNavigationManager()->BotGoto((*it).c_str(), conf);
			}
			else
			{
				res = GetNavigationManager()->BotGoto(szName, conf);
			}

			OutputDebugStringA(res?"goto true":"goto false\n");
		}
		break;
	case IDC_BTN_CREATEBOT:
		{
			NaviBotInitConfig conf;
			conf.m_fSpeed = 3;
			//conf.m_bEnableShortcut = true;
			conf.m_bEnableAvoidence = true;
			char szName[255], szPosition[255], szSceneName[255],szRadius[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_SCENENAME, szSceneName, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_CREATEBOT_NAME, szName, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_CREATEBOT_POS, szPosition, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_CREATEBOT_RADIUS, szRadius, 255);
			
			conf.m_fRadius = szRadius[0]? atof(szRadius) : 0.2f;
			ParseStringToPos(szPosition, conf.m_vecPos);
			bool res = GetNavigationManager()->BotCreate(szSceneName, szName, conf);
			if(allBotName.find(szName)==allBotName.end())
				allBotName.insert(szName);
		}
		break;
	case IDC_BTN_BOTGETPOSITION:
		{
			char szPosition[255] = "0,0,0", szName[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_CREATEBOT_NAME, szName, 255);
			NaviVector3 curPos;
			GetNavigationManager()->BotGetPosition(szName, curPos);
			printf("%s pos:%f,%f,%f\n", szName, curPos.x, curPos.y, curPos.z);
			int j = 10;
		}
		break;
	case IDC_BTN_REMOVEBOT:
		{
			char szName[255], szPosition[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_CREATEBOT_NAME, szName, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_CREATEBOT_POS, szPosition, 255);
			GetNavigationManager()->BotRemove(szName);
		}
		break;
	case IDC_BTN_PAUSEGOTO:
		{
			char szName[255];//, szPosition[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTGOTO_NAME, szName, 255);
			GetNavigationManager()->BotPauseMoving(szName);
		}
		break;
	case IDC_BTN_BOTSTOPMOVING:
		{
			char szName[255];//, szPosition[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTGOTO_NAME, szName, 255);
			GetNavigationManager()->BotStop(szName);
		}
		break;
	case IDC_BTN_RESUMGOTO:
		{
			char szName[255];//, szPosition[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTGOTO_NAME, szName, 255);
			GetNavigationManager()->BotResumMoving(szName);
		}
		break;
	case IDC_BTN_BODYGOTO:
		{
			char szPos[255], szName[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_BODYGOTO_POS, szPos, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_BODYGOTO_NAME, szName, 255);
			NaviBodyGotoConfig conf;
			conf.m_angularSpeed = 1.5f;
			conf.m_destOrign.y = 90;
			ParseStringToPos(szPos, conf.m_destPosition);
			conf.m_linearSpeed = 1;
			GetNavigationManager()->BodyGoto(szName, conf);			
		}
		break;

	case IDC_BTN_CREATEBODY:
		{
			char szPosition[255] = "0,0,0", szSceneName[255], szName[255], szRotate[255], szType[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_SCENENAME, szSceneName, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_CREATEBODY_POSITION, szPosition, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_CREATEBODY_NAME, szName, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_CREATEBODY_ROTATE, szRotate, 255 );
			GetDlgItemTextA(hDlg, IDC_COMB_CREATEBODYTYPE, szType, 255);
			if (strcmp(szType,"Box")==0)
			{
				NaviBoxBodyInitConfig conf;
				conf.m_boxHalfExtents = NaviVector3(1,1,4);
				conf.m_rotate = NaviVector3(0,atof(szRotate)/180.0f*3.1415926535898f,0);
				conf.m_bTriggerTagVolume = true;
				ParseStringToPos(szPosition, conf.m_Position);
				GetNavigationManager()->BodyCreate(szSceneName, szName, conf);
			}
			else if (strcmp(szType,"Cylinder")==0)
			{
				NaviCylinderBodyInitConfig conf;
				conf.m_fHeight = 1;
				conf.m_fRadius = 2;
				ParseStringToPos(szPosition, conf.m_Position);
				GetNavigationManager()->BodyCreate(szSceneName, szName, conf);
			}
		}
		break;
	case IDC_BTN_REMOVEBODY:
		{
			char szName[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_CREATEBODY_NAME, szName, 255);
			GetNavigationManager()->BodyRemove(szName);
		}
		break;

	case IDC_BTN_ADDSECTOR:
		{
			char szPath[512];
			GetDlgItemTextA(hDlg, IDC_EDIT_ADDSECTOR, szPath, 512);
			const char* navi = "D:\\qqwe\\navigation\\win32_vc8_src\\solutions\\StudyTest\\NaviData\\Sector_B.NavData";
			GetNavigationManager()->AddNaviData("", szPath);
		}
		break;
	case IDC_BTN_REMOVESECTOR:
		{
			char szPath[512];
			GetDlgItemTextA(hDlg, IDC_EDIT_ADDSECTOR, szPath, 512);
			const char* navi = "D:\\qqwe\\navigation\\win32_vc8_src\\solutions\\StudyTest\\NaviData\\Sector_B.NavData";
			GetNavigationManager()->RemoveNaviData("", szPath);
		}
		break;
	case IDC_BTN_BOTSETPOS:
		{
			NaviVector3 pos;
			char szPosition[255] = "0,0,0", szName[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTSETPOS_POS, szPosition, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTSETPOS_NAME, szName, 255);
			ParseStringToPos(szPosition, pos);
			//pos.y += 3;
			GetNavigationManager()->BotSetPosition(szName, pos);

		}
		break;

	case IDC_BTN_QUERYNEARPOS:
		{
			char szPosition[255] = "0,0,0", szName[255], szSceneName[255];
			NaviVector3 pos(-7.38,14.852,-206.34415), resPos;
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTSETPOS_POS, szPosition, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_SCENENAME, szSceneName, 255);
			if(szPosition[0]!=0)
				ParseStringToPos(szPosition, pos);

			if(GetNavigationManager()->QueryNearestPosOnGround(szSceneName, pos, resPos))
			{
				std::cout<<"QueryNearestPosOnGround"<<std::endl;
			}
		}
		break;
	case IDC_BTN_QUERYSPEED:
		{
			char buff255[255], szName[255];
			NaviVector3 speed;
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTQUERYVALUE_NAME, szName, 255);
			GetNavigationManager()->BotGetSpeed(szName, speed);
			sprintf_s(buff255, "%f", sqrtf(speed.x*speed.x + speed.x*speed.x + speed.x*speed.x));			
			SetDlgItemTextA(hDlg, IDC_STATIC_QUERYRES, buff255);
		}
		break;
	case IDC_BTN_SETSPEED:
		{
			char buff255[255], szName[255];
			GetDlgItemTextA(hDlg, IDC_EDIT_SETVALUE, buff255, sizeof(buff255));
			GetDlgItemTextA(hDlg, IDC_EDIT_BOTSETVALUE_NAME, szName, 255);
			float fSpeed = atof(buff255);
			GetNavigationManager()->BotSetSpeed(szName, fSpeed);
		}
		break;
	case IDC_BTN_CANGO:
	case IDC_BTN_RAYCANGO:
	case IDC_BTN_QUERYNEARESTPOS:
		{
			char szPosition[255] = "0,0,0", szSceneName[255];
			NaviVector3 startPos, destPos;
			GetDlgItemTextA(hDlg, IDC_EDIT_SCENENAME, szSceneName, 255);
			GetDlgItemTextA(hDlg, IDC_EDIT_CANGO_START, szPosition, 255);
			ParseStringToPos(szPosition, startPos);
			GetDlgItemTextA(hDlg, IDC_EDIT_CANGO_END, szPosition, 255);
			ParseStringToPos(szPosition, destPos);
			if (controlID==IDC_BTN_CANGO)
			{
				NaviVector3* path = 0;
				int nNodeCount = 0;
				std::cout<<"QueryCanGo "<<
					(GetNavigationManager()->QueryCanGo(szSceneName, startPos, destPos, 20, &path, &nNodeCount)?"true":"false")<<std::endl;
			}
			else if (controlID==IDC_BTN_RAYCANGO)
			{
				NaviVector3 pos;
				std::cout<<"QueryCanGo "<<
					(GetNavigationManager()->QueryRayCanGo(szSceneName,startPos, destPos, &pos)?"true":"false")<<std::endl;
			}
			else if(controlID==IDC_BTN_QUERYNEARESTPOS)
			{
				std::cout<<"QueryNearestPosOnGround "<<
					(GetNavigationManager()->QueryNearestPosOnGround(szSceneName, startPos, destPos)?"true":"false")<<std::endl;
			}
		}
		break;
	case IDC_BTN_CLEAR:
		GetNavigationManager()->Clear();
		break;
	default:break;
	}

	return TRUE;
}
// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_CLOSE:
		g_bQuite = true;
		break;
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		{
			int controlID = LOWORD(wParam);
			int hi = HIWORD(wParam);
			return OnButtonClick(controlID, hDlg, wParam);
		}
		break;
	case WM_RBUTTONUP:
		g_bPause = !g_bPause;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ACTIVATE:
		//std::cout<<"WM_ACTIVATE"<<std::endl;
		break;
	case WM_SETFOCUS:
		//std::cout<<"WM_SETFOCUS"<<std::endl;
		break;
	case WM_KILLFOCUS:
		//std::cout<<"WM_KILLFOCUS"<<std::endl;
		break;
	case WM_MOUSEMOVE:
		if(wParam & MK_LBUTTON)
		{
			static int lbutton = 0;
			//std::cout<<"left button down:"<<lbutton++<<std::endl;
		}
		if (wParam & MK_CONTROL)
		{
			static int control = 0;
			//std::cout<<"ctrl button down:"<<control++<<std::endl;
		}
		if (wParam & MK_MBUTTON)
		{
			static int mbutton = 0;
			//std::cout<<"midd button down:"<<mbutton++<<std::endl;
		}
		if (wParam & MK_RBUTTON)
		{
			static int rbutton = 0;
			//std::cout<<"right button down:"<<rbutton++<<std::endl;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
