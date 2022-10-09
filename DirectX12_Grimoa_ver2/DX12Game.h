#pragma once
#include "DX12Game.h"
#include "DirectX12_Graphics.h"
#include "ModelData.h"
using Microsoft::WRL::ComPtr;

class DX12Game
{
	DX12Game() { };

	ComPtr<PMDActor> rin_model;

public:
	DX12Game(const DX12Game&) = delete;
	DX12Game& operator=(const DX12Game&) = delete;
	DX12Game(DX12Game&&) = delete;
	DX12Game& operator=(DX12Game&&) = delete;


	~DX12Game() {
		Dispose();
	}

	static DX12Game* GetInstance() {
		static DX12Game instance;
		return &instance;
	}

	//‰Šú‰»ˆ—
	void Init(HWND hWnd, unsigned int Width, unsigned int Height, bool fullscreen);
	//XVˆ—
	void Update();
	//•`‰æˆ—
	void Draw();
	//I—¹ˆ—
	void Dispose();
};

