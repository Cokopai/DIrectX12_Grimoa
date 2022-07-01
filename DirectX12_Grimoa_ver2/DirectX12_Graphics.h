#pragma once
#pragma comment (lib,"d3d12.lib")
#pragma comment (lib,"dxgi.lib")

#include	<d3d12.h>
#include	 <dxgi1_6.h>
#include	<wrl/client.h>

using Microsoft::WRL::ComPtr;

class DirectX12_Graphics {
private:
	DirectX12_Graphics() {}

	//デバイス
	ComPtr<ID3D12Device>	 _dev = nullptr;
	//ファクトリー
	ComPtr<IDXGIFactory6>	 _dxgiFactory = nullptr;
	//スワップチェーン
	ComPtr<IDXGISwapChain4>	 _swapchain = nullptr;

	//コマンドリスト
	ComPtr<ID3D12CommandAllocator> _cmdAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> _cmdList = nullptr;
	ComPtr<ID3D12CommandQueue> _cmdQueue = nullptr;

	//ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> _rtvHeaps = nullptr;//RenderTargetView Heaps

public:
	DirectX12_Graphics(const DirectX12_Graphics&) = delete;
	DirectX12_Graphics& operator=(const DirectX12_Graphics&) = delete;
	DirectX12_Graphics(DirectX12_Graphics&&) = delete;
	DirectX12_Graphics& operator=(DirectX12_Graphics&&) = delete;


	~DirectX12_Graphics() {
		Exit();
	}

	static DirectX12_Graphics* GetInstance() {
		static DirectX12_Graphics instance;
		return &instance;
	}

	// 初期処理
	bool Init(HWND hWnd,unsigned int Width, unsigned int Height, bool fullscreen);
	//
	bool Update();

	// 終了処理
	void Exit();

	// デバイスGET
	ID3D12Device*	GetDXDevice() const {
		return _dev.Get();
	}

	// SWAPチェインGET
	IDXGISwapChain* GetSwapChain()const {
		return _swapchain.Get();
	}

};