
#include "DX12Game.h"

void DX12Game::Init(HWND hWnd, unsigned int Width, unsigned int Height, bool fullscreen)
{
	DirectX12_Graphics::GetInstance()->Init(hWnd,Width,Height,fullscreen);
}

void DX12Game::Update()
{
	DirectX12_Graphics::GetInstance()->BeforeRender();
}

void DX12Game::Draw()
{
	DirectX12_Graphics* ins = DirectX12_Graphics::GetInstance();
	ins->BeforeRender();
	ins->Draw();
	ins->AfterRender();
}

void DX12Game::Dispose()
{

}
