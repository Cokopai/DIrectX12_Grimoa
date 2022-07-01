#include "Application.h"

//#include "dx12util.h"

#ifdef  _DEBUG
#include <iostream>
#endif  _DEBUG

using namespace std;

//@brief コンソール画面にフォーマット月の文字列を表示
//@param format フォーマット（%d,%fなど）
//@remarks この関数はデバック用ですデバック時にしか動作しません

void DebugOutputFormatString(const char* format, ...) {
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

int WINAPI WinMain(HINSTANCE  h_hInst,HINSTANCE  h_hPrevInst,LPSTR h_lpszArgs,int h_nWinMode)
{
	Application* App = Application::Instance();		// インスタンス取得
	App->Init(h_hInst);

	// ウインドウを表示する
	ShowWindow(App->GetHWnd(), h_nWinMode);
	UpdateWindow(App->GetHWnd());

	// メインループ
	long ret = App->MainLoop();

	// アプリケーション終了処理
	App->Dispose(h_hInst);

	return ret;
}
