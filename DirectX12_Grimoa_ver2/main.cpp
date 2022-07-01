#include "Application.h"

//#include "dx12util.h"

#ifdef  _DEBUG
#include <iostream>
#endif  _DEBUG

using namespace std;

//@brief �R���\�[����ʂɃt�H�[�}�b�g���̕������\��
//@param format �t�H�[�}�b�g�i%d,%f�Ȃǁj
//@remarks ���̊֐��̓f�o�b�N�p�ł��f�o�b�N���ɂ������삵�܂���

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
	Application* App = Application::Instance();		// �C���X�^���X�擾
	App->Init(h_hInst);

	// �E�C���h�E��\������
	ShowWindow(App->GetHWnd(), h_nWinMode);
	UpdateWindow(App->GetHWnd());

	// ���C�����[�v
	long ret = App->MainLoop();

	// �A�v���P�[�V�����I������
	App->Dispose(h_hInst);

	return ret;
}
