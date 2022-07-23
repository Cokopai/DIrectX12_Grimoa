#include <Windows.h>
#include <tchar.h>
#include <string>
#include <d3d12sdklayers.h>
#include <tchar.h>
#include <DirectXMath.h>

#include "DirectX12_Graphics.h"
#include "macro.h"

using namespace DirectX;

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

		sts = _swapchain->GetBuffer(static_cast<UINT>(idx), IID_PPV_ARGS(_backBuffers[idx].GetAddressOf()));

		if (sts != S_OK) {
			MessageBox(nullptr, _T("DirectX12_Graphics RTVBuffer Error!"), _T("error"), MB_OK);
		}

		_dev->CreateRenderTargetView(
			_backBuffers[idx].Get(),
			nullptr,
			handle
		);

		handle.ptr += _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		);
	}
	
	ID3DBlob* errorBlob;


	////////////////////////////////////////////////////////////////
	//VertexShader Compile	Start
	////////////////////////////////////////////////////////////////
	sts = D3DCompileFromFile(
		L"BasicVertexShader.hlsl", //filename
		nullptr,//インクルードはなし
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//インクルードはデフォルト
		"BasicVS", "vs_5_0",//関数はBasicVS,対象はvs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_vsBlob,&errorBlob
	);

	if (FAILED(sts)) {

		if (sts == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			OutputDebugStringA("ファイルが見つかりません");
			return 0;//exitなどで終了処理を行うよう作り直す
		}
		else {

			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			//エラー文をコピー
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += '\n';

			OutputDebugStringA(errstr.c_str());//デバック出力
		}
	}	
	////////////////////////////////////////////////////////////////
	//VertexShader Compile	End
	////////////////////////////////////////////////////////////////
	

	////////////////////////////////////////////////////////////////
	//PixelShader Compile	Start
	////////////////////////////////////////////////////////////////
	sts = D3DCompileFromFile(
		L"BasicPixelShader.hlsl", //filename
		nullptr,//インクルードはなし
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//インクルードはデフォルト
		"BasicPS", "ps_5_0",//関数はBasicPS,対象はps_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob,&errorBlob
	);

	if (FAILED(sts)) {

		if (sts == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			OutputDebugStringA("ファイルが見つかりません");
			return 0;//exitなどで終了処理を行うよう作り直す
		}
		else {

			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			//エラー文をコピー
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += '\n';

			OutputDebugStringA(errstr.c_str());//デバック出力
		}
	}	
	////////////////////////////////////////////////////////////////
	//PixelShader Compile	End
	////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////
	//グラフィックパイプラインステート	Start
	////////////////////////////////////////////////////////////////

	D3D12_GRAPHICS_PIPELINE_STATE_DESC _pipeline = {};
	_pipeline.pRootSignature = nullptr;

	_pipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	_pipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	_pipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	_pipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

	//デフォルトのサンプルマスク(0xffffffff)
	_pipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//アンチエイリアス
	_pipeline.RasterizerState.MultisampleEnable = false;

	_pipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	_pipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//中を塗りつぶす
	_pipeline.RasterizerState.DepthClipEnable = true; //深度方向のクリッピング:有効

	_pipeline.BlendState.AlphaToCoverageEnable = false;
	_pipeline.BlendState.IndependentBlendEnable = false;

	//レンダーターゲットデスクの適応
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = { };
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	_pipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	//入力レイアウトの適応
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{
	"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
	D3D12_APPEND_ALIGNED_ELEMENT,
	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
	};

	_pipeline.InputLayout.pInputElementDescs = inputLayout;
	_pipeline.InputLayout.NumElements = _countof(inputLayout);

	_pipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	//三角形で構成
	_pipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//レンダーターゲットの設定
	_pipeline.NumRenderTargets = 1;
	_pipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;// 0 ~ 1に正規化されたRGBA
	//アンチエイリアシングの設定
	_pipeline.SampleDesc.Count = 1;  //サンプリングは1ピクセルにつき 1
	_pipeline.SampleDesc.Quality = 0;//クオリティ:最低

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* rootSigBlob = nullptr;

	sts = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&errorBlob
	);
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics rootSigBlobError!"), _T("error"), MB_OK);
	}

	sts = _dev->CreateRootSignature(
		0,//nodemask 0でよい
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&_rootSignature)
	);

	_pipeline.pRootSignature = _rootSignature;

	sts = _dev->CreateGraphicsPipelineState(&_pipeline, IID_PPV_ARGS(&_pipelinestate));

	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics pipeline Error!"), _T("error"), MB_OK);
	}
	////////////////////////////////////////////////////////////////
	//グラフィックパイプラインステート	End
	////////////////////////////////////////////////////////////////

	return sts;
}

bool DirectX12_Graphics::BeforeRender()
{
	HRESULT sts = 0;

	XMFLOAT3 vertices[]{
		{-0.5f,-0.7f,0.0f},
		{0.0f,0.7f,0.0f},
		{0.5f,-0.7f,0.0f}
	};

	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = _backBuffers[bbIdx].Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_cmdList->ResourceBarrier(1, &BarrierDesc);

	//レンダーターゲットを指定
	auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	_cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	_cmdList->ResourceBarrier(1, &BarrierDesc);

	D3D12_HEAP_PROPERTIES heapprop = { };

	heapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resdesc = { };

	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resdesc.Width = sizeof(vertices);
	resdesc.Height = 1;
	resdesc.DepthOrArraySize = 1;
	resdesc.MipLevels = 1;
	resdesc.Format = DXGI_FORMAT_UNKNOWN;
	resdesc.SampleDesc.Count = 1;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	ID3D12Resource* verBuffer = nullptr;

	ID3D12Device* _dev = DirectX12_Graphics::GetInstance()->GetDXDevice();
	sts = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&verBuffer)
	);

	if (sts != S_OK) {
		MessageBox(nullptr, _T("VertexBuffer Create Error!"), _T("error"), MB_OK);
	}

	XMFLOAT3* vertMap = nullptr;

	sts = verBuffer->Map(0, nullptr, (void**)&vertMap);

	std::copy(std::begin(vertices), std::end(vertices), vertMap);

	verBuffer->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = { };

	vbView.BufferLocation = verBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vertices);
	vbView.StrideInBytes = sizeof(vertices[0]);

	_cmdList->IASetVertexBuffers(0, 1, &vbView);

	return sts;
}

void DirectX12_Graphics::Draw()
{
	D3D12_VIEWPORT _viewPort = {};
	_viewPort.Width = CLIENT_WIDTH;
	_viewPort.Height = CLIENT_HEIGHT;
	_viewPort.TopLeftX = 0;
	_viewPort.TopLeftY = 0;
	_viewPort.MaxDepth = 1.0f;
	_viewPort.MinDepth = 0.0f;

	D3D12_RECT _scissorrect = {};
	_scissorrect.top = 0;
	_scissorrect.left = 0;
	_scissorrect.right = _scissorrect.left + CLIENT_WIDTH;
	_scissorrect.bottom = _scissorrect.top + CLIENT_HEIGHT;

	_cmdList->SetPipelineState(_pipelinestate);

	_cmdList->RSSetViewports(1, &_viewPort);
	_cmdList->RSSetScissorRects(1, &_scissorrect);

	_cmdList->SetGraphicsRootSignature(_rootSignature);
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_cmdList->DrawInstanced(3, 1, 0, 0);
}

bool DirectX12_Graphics::AfterRender()
{
	HRESULT sts;

	//命令のクローズ
	_cmdList->Close();

	//コマンドリストの実行
	ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);
	////待ち
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

	if (_fence->GetCompletedValue() != _fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	_cmdAllocator->Reset();//キューをクリア
	sts = _cmdList->Reset(_cmdAllocator.Get(), nullptr);//再びコマンドリストをためる準備

	//フリップ
	_swapchain->Present(1, 0);
	return sts;
}

void DirectX12_Graphics::Exit()
{
	SAFE_RELEASE(_pipelinestate);
	SAFE_RELEASE(_rootSignature);
}
