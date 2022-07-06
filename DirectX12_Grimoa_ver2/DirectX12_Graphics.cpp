#include <Windows.h>
#include <tchar.h>
#include <string>
#include <d3d12sdklayers.h>
#include "DirectX12_Graphics.h"

bool DirectX12_Graphics::Init(HWND hWnd, unsigned int window_width, unsigned int window_height, bool fullscreen)
{
	HRESULT sts;

#ifdef  _DEBUG
	//デバックレイヤー
	ComPtr<ID3D12Debug>		_debugLayer = nullptr;
	sts = D3D12GetDebugInterface(IID_PPV_ARGS(_debugLayer.ReleaseAndGetAddressOf()));
	_debugLayer->EnableDebugLayer();

	//_debugLayer->Release();
#endif //  _DEBUG


	sts = D3D12CreateDevice(
	nullptr,
		D3D_FEATURE_LEVEL_12_1,
		IID_PPV_ARGS(&_dev)
	);
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateDevice Error!"), _T("error"), MB_OK);
	}

#ifdef _DEBUG
	sts = CreateDXGIFactory2(
		DXGI_CREATE_FACTORY_DEBUG,IID_PPV_ARGS(&_dxgiFactory)
	);
#else
	sts = CreateDXGIFactory1(
		IID_PPV_ARGS(&_dxgiFactory)
	);
#endif // DEBUG

	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateDXGIFactory Error!"), _T("error"), MB_OK);
	}

	std::vector<ComPtr<IDXGIAdapter>> adapters;

	ComPtr<IDXGIAdapter> tmpAdapter = nullptr;

	for (int i = 0;
		_dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
		++i
		) {
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters) {
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc);

		std::wstring strDesc = adesc.Description;

		if (strDesc.find(L"NVIDIA") != std::string::npos) {
			tmpAdapter = adpt;
			break;
		}
	}

	sts = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_cmdAllocator));
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateCommandAllocator Error!"), _T("error"), MB_OK);
	}

	sts = _dev->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		_cmdAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&_cmdList));
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateCommandList Error!"), _T("error"), MB_OK);
	}

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; //タイムアウトなし
	cmdQueueDesc.NodeMask = 0;							//アダプターが1つしかないときは、0でよい
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//プライオリティは指定なし
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	sts = _dev->CreateCommandQueue(&cmdQueueDesc,IID_PPV_ARGS(&_cmdQueue));
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateCommandQueue Error!"), _T("error"), MB_OK);
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

	swapChainDesc.Width = window_width;
	swapChainDesc.Height = window_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount = 2;

	//バックバッファは伸び縮み可能
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	//フリップ後は速やかに破棄
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//特に指定なし
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//フルスクリーンとウィンドウを切り替え可能
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	sts = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue.Get(),
		hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)_swapchain.GetAddressOf()
	);
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateSwapChain Error!"), _T("error"), MB_OK);
	}

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2; //スワップチェーンの表裏で二つ
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	sts = _dev->CreateDescriptorHeap(&heapDesc,IID_PPV_ARGS(&_rtvHeaps));
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateRTVHeaps Error!"), _T("error"), MB_OK);
	}

	sts = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics Fence Error!"), _T("error"), MB_OK);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE handle
		= _rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	_backBuffers.resize(swapChainDesc.BufferCount);

	for (int idx = 0; idx < swapChainDesc.BufferCount; ++idx) {

		sts = _swapchain->GetBuffer(static_cast<UINT>(idx), IID_PPV_ARGS(&_backBuffers[idx]));

		if (sts != S_OK) {
			MessageBox(nullptr, _T("DirectX12_Graphics RTVBuffer Error!"), _T("error"), MB_OK);
		}

		_dev->CreateRenderTargetView(
			_backBuffers[idx],
			nullptr,
			handle
		);

		handle.ptr += _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		);
	}

	return sts;
}

bool DirectX12_Graphics::Update()
{
	HRESULT sts;
	//アロケータークリア
	sts = _cmdAllocator->Reset();

	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

	//リソースバリア
	D3D12_RESOURCE_BARRIER _barrierDesc = {};
	_barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	_barrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	_barrierDesc.Transition.pResource = _backBuffers[bbIdx];
	_barrierDesc.Transition.Subresource = 0;

	_barrierDesc.Transition.StateBefore
		= D3D12_RESOURCE_STATE_PRESENT; //直前は PRESENT 状態
	_barrierDesc.Transition.StateAfter
		= D3D12_RESOURCE_STATE_RENDER_TARGET;

	_cmdList->ResourceBarrier(
		1,
		&_barrierDesc);

	//RTVHandleからCommandListにセットする
	auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	rtvH.ptr += _dev->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV
	);

	_cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

	//画面クリア
	float clearColor[] = { 0.0f,1.0f,1.0f,1.0f };
	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	_barrierDesc.Transition.StateBefore
		= D3D12_RESOURCE_STATE_RENDER_TARGET; //直前は RENDER_TARGET 状態
	_barrierDesc.Transition.StateAfter
		= D3D12_RESOURCE_STATE_PRESENT;

	_cmdList->ResourceBarrier(
		1,
		&_barrierDesc);

	//命令のクローズ
	_cmdList->Close();

	ID3D12CommandList* cmdLists[] = { _cmdList.Get() };

	_cmdQueue->ExecuteCommandLists(1, cmdLists);

	//実行が終了するまでシグナルを送る
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

	UINT fenceVal = _fence.Get();
	UINT fenceComValue = _fence->GetCompletedValue();
	if (_fence->GetCompletedValue() != _fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		//イベントが終了するまで待ち続ける
		WaitForSingleObject(event, INFINITE);

		CloseHandle(event);
	}

	_cmdAllocator->Reset();
	_cmdList->Reset(_cmdAllocator.Get(), nullptr);

	_swapchain->Present(1,0);

	return sts;
}

void DirectX12_Graphics::Exit()
{

}
