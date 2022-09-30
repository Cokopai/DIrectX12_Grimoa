
#include "DX12Game.h"
#include "PMD_Layout.h"
#include <vector>

void DX12Game::Init(HWND hWnd, unsigned int Width, unsigned int Height, bool fullscreen)
{
	DirectX12_Graphics::GetInstance()->Init(hWnd,Width,Height,fullscreen);

	rin_model = new ModelData();
	rin_model->CreateModel("Model/‹¾‰¹ƒŠƒ“.pmd");
}

void DX12Game::Update()
{

}

void DX12Game::Draw()
{
	DirectX12_Graphics* ins = DirectX12_Graphics::GetInstance();

	ins->BeforeRender();

	ins->Draw(rin_model.Get());
	ins->AfterRender();
}

void DX12Game::Dispose()
{
	DirectX12_Graphics::GetInstance()->Exit();
}
