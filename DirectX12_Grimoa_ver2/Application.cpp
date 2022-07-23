#include "Application.h"
//*****************************************************************************
//!	@file	Application.cpp
//!	@brief	
//!	@note	アプリケーションクラス
//!	@author	
//*****************************************************************************

//-----------------------------------------------------------------------------
//	Include header files.
//-----------------------------------------------------------------------------
#include <Crtdbg.h>
#include <Windows.h>
#include <chrono>
#include <thread>
//#include <cinttypes>

#include "tchar.h"

#include "Application.h"
#include "CWindow.h"
#include "DX12Game.h"

//#include "macro.h"
//#include "game.h"

//-----------------------------------------------------------------------------
// スタティック　メンバー
//-----------------------------------------------------------------------------
const char* Application::WINDOW_TITLE = "DirectX12app";
const char* Application::WINDOW_CLASS_NAME = "win32app";

const uint32_t		Application::WINDOW_STYLE_WINDOWED = (WS_VISIBLE | WS_CAPTION | WS_SYSMENU);
const uint32_t		Application::WINDOW_EX_STYLE_WINDOWED = (0);
const uint32_t		Application::WINDOW_STYLE_FULL_SCREEN = (WS_VISIBLE | WS_POPUP);
const uint32_t		Application::WINDOW_EX_STYLE_FULL_SCREEN = (0);

const uint32_t		Application::CLIENT_WIDTH = 1280;
const uint32_t		Application::CLIENT_HEIGHT = 720;

uint32_t			Application::SYSTEM_WIDTH = 0;
uint32_t			Application::SYSTEM_HEIGHT = 0;

const float			Application::FPS = 60;

//==============================================================================
//!	@fn		CApplication
//!	@brief	コンストラクタ
//!	@param	
//!	@retval	
//==============================================================================
Application::Application() : m_SystemCounter(0)
{}

//==============================================================================
//!	@fn		~Application
//!	@brief	デストラクタ
//!	@param	
//!	@retval	
//==============================================================================
Application :: ~Application()
{

}

//==============================================================================
//!	@fn		GetInstance
//!	@brief	インスタンス取得
//!	@param	
//!	@retval	インスタンス
//==============================================================================
Application* Application::Instance()
{
	static Application Instance;

	return &Instance;
}

//==============================================================================
//!	@fn		InitSystemWH
//!	@brief	システム領域の幅と高さを初期化
//!	@param	
//!	@retval	
//==============================================================================
void Application::InitSystemWH()
{

}

//==============================================================================
//!	@fn		Init
//!	@brief	初期化
//!	@param	インスタンスハンドル
//!	@retval	
//==============================================================================
bool Application::Init(HINSTANCE h_Instance)
{
	// メモリーリークを検出
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// コンソールを割り当てる
	AllocConsole();

	// 標準出力の割り当て
	freopen_s(&m_fp, "CON", "w", stdout);

	// 幅と高さ初期化
	InitSystemWH();

	// ウインドウ作成
	CWindow* window = CWindow::Instance();

	window->SetWindow(h_Instance,
		CLIENT_WIDTH + SYSTEM_WIDTH,
		CLIENT_HEIGHT + SYSTEM_HEIGHT
		);

	// HWND
	m_hWnd = window->GetHandle();

	//
	m_hInst = h_Instance;

	
	return true;
}

//==============================================================================
//!	@fn		Dispose
//!	@brief  終了処理
//!	@param	
//!	@retval	
//==============================================================================
void Application::Dispose(HINSTANCE h_cpInstance)
{
	// 標準出力クローズ
	fclose(m_fp);
	// コンソール開放
	::FreeConsole();

	CWindow* window = CWindow::Instance();
	return;
}

//==============================================================================
//!	@fn		MainLoop
//!	@brief	メインループ
//!	@param	
//!	@retval	メッセージID
//==============================================================================
unsigned long Application::MainLoop()
{
	MSG msg = { };

	ZeroMemory(&msg, sizeof(msg));

	CWindow* window = CWindow::Instance();
	DX12Game* dxGame = DX12Game::GetInstance();

	dxGame->Init(window->GetHandle(),CLIENT_WIDTH,CLIENT_HEIGHT,false);

	while (true) {

		dxGame->Update();
		dxGame->Draw();

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//アプリケーションが終わるときにmessageがWM_QUITになる
		if (msg.message == WM_QUIT) {
			break;
		}
	}
	// ゲームの終了処理
	//GameDispose();
	return window->GetMessage();
}

//==============================================================================
//!	@fn		Input
//!	@brief	キー入力
//!	@param	
//!	@retval	
//==============================================================================
void Application::Input(uint64_t deltataime)
{
}

//==============================================================================
//!	@fn		Update
//!	@brief	更新
//!	@param	
//!	@retval	
//==============================================================================
void Application::Update(uint64_t deltataime)
{
	// システムカウンタ
	m_SystemCounter++;
}

//==============================================================================
//!	@fn		Render
//!	@brief	描画
//!	@param	
//!	@retval	
//==============================================================================
void Application::Render(uint64_t deltatime)
{
}

//==============================================================================
//!	@fn		GetHWnd
//!	@brief	HWND 取得
//!	@param	
//!	@retval	HWND
//==============================================================================
HWND Application::GetHWnd()
{
	return m_hWnd;
}

//==============================================================================
//!	@fn		GetHInst
//!	@brief	HINSTANCE 取得
//!	@param	
//!	@retval	HINSTANCE
//==============================================================================
HINSTANCE Application::GetHInst()
{
	return m_hInst;
}

//******************************************************************************
//	End of file.
//******************************************************************************
