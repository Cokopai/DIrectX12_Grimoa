#pragma once
#pragma comment (lib,"DirectXTex.lib")
#pragma comment (lib,"d3d12.lib")
#pragma comment (lib,"dxgi.lib")
#pragma comment (lib,"d3dcompiler.lib")

#include	<d3d12.h>
#include	<DirectXTex.h>
#include	<dxgi1_6.h>
#include	<wrl/client.h>
#include	<vector>
#include	<array>
#include	<d3dcompiler.h>

#include "vertex.h"
#include "ModelData.h"

using Microsoft::WRL::ComPtr;

class DirectX12_Graphics {
private:
	DirectX12_Graphics() {}

	//デバイス
	ComPtr<ID3D12Device>				 _dev = nullptr;
	//ファクトリー
	ComPtr<IDXGIFactory6>				 _dxgiFactory = nullptr;
	//スワップチェーン
	ComPtr<IDXGISwapChain4>			  	 _swapchain = nullptr;

	//コマンドリスト
	ComPtr<ID3D12CommandAllocator>		 _cmdAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList>	 _cmdList = nullptr;
	ComPtr<ID3D12CommandQueue>			 _cmdQueue = nullptr;

	ComPtr<ID3DBlob>					 _errorBlob;
	ComPtr<ID3DBlob>					 _vsBlob;
	ComPtr<ID3DBlob>					 _psBlob;
	//バックバッファ
	std::array<ComPtr<ID3D12Resource>, 2>_backBuffers;
	//深度バッファ
	ComPtr<ID3D12Resource>				 _depthBuffer;

	//フェンス
	ComPtr<ID3D12Fence>					 _fence = nullptr;
	UINT64 _fenceVal = 0;

	ComPtr<ID3D12PipelineState>			_pipelinestate = nullptr;
	ComPtr<ID3D12RootSignature>			_rootSignature = nullptr;
	ID3D12DescriptorHeap*				_rtvHeaps	= nullptr;//RenderTargetView Heaps
	ComPtr<ID3D12DescriptorHeap>		_dsvHeap	= nullptr;//RenderTargetView Heaps

	//画面クリア　カラー
	float clearColor[4] = { 0.6f,1.0f,0.8f,1.0f };

	//std::vector<TexRGBA> textureData;
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
	HRESULT CreateDevice();
	HRESULT CreateDXGIFactory();
	HRESULT CreateCommand();

	HRESULT CreateFence();
	HRESULT CreateRTV();
	HRESULT CreateDSV();

	HRESULT CreateRootSigature();

	unsigned const short swapBufferCount = 2;
	HRESULT CreateSwapChain(HWND hWnd,unsigned int window_width, unsigned int window_height);

	HRESULT CreateGraphicPipeline();

	//更新処理
	bool BeforeRender();
	void Draw();
	bool AfterRender();

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

	ID3D12GraphicsCommandList* GetCommandList()const {
		return _cmdList.Get();
	}

	void UpdateBufferSize(unsigned char* vertices);
};

