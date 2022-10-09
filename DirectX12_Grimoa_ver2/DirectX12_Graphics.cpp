#include <Windows.h>
#include <tchar.h>
#include <string>
#include <d3d12sdklayers.h>
#include <tchar.h>
#include <DirectXMath.h>
#include<vector>
#include <d3dx12.h>

#include "DirectX12_Graphics.h"
#include "macro.h"

using namespace DirectX;

bool DirectX12_Graphics::Init(HWND hWnd, unsigned int window_width, unsigned int window_height, bool fullscreen)
{
	HRESULT sts;

#ifdef  _DEBUG
	//�f�o�b�N���C���[
	ComPtr<ID3D12Debug>		_debugLayer = nullptr;
	sts = D3D12GetDebugInterface(IID_PPV_ARGS(_debugLayer.ReleaseAndGetAddressOf()));
	_debugLayer->EnableDebugLayer();

	//_debugLayer->Release();
#endif //  _DEBUG
	sts = CreateDevice();

	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateDevice Error!"), _T("error"), MB_OK);
	}

#ifdef _DEBUG
	sts = CreateDXGIFactory2(
		DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory)
	);
#else
	sts = CreateDXGIFactory1(
		IID_PPV_ARGS(&_dxgiFactory)
	);
#endif // DEBUG

	sts = CreateDXGIFactory();
	sts = CreateCommand();
	sts = CreateSwapChain(hWnd,window_width,window_height);
	sts = CreateFence();
	sts = CreateRTV();
	sts = CreateDSV();

	////////////////////////////////////////////////////////////////
	//VertexShader Compile	Start
	////////////////////////////////////////////////////////////////
	sts = D3DCompileFromFile(
		L"BasicVertexShader.hlsl", //filename
		nullptr,//�C���N���[�h�͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//�C���N���[�h�̓f�t�H���g
		"BasicVS", "vs_5_1",//�֐���BasicVS,�Ώۂ�vs_5_1
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_vsBlob, &_errorBlob
	);

	if (FAILED(sts)) {
		if (sts == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			OutputDebugStringA("�t�@�C����������܂���");
			return 0;//exit�ȂǂŏI���������s���悤��蒼��
		}
		else {

			std::string errstr;
			errstr.resize(_errorBlob->GetBufferSize());
			//�G���[�����R�s�[
			std::copy_n((char*)_errorBlob->GetBufferPointer(),
				_errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += '\n';

			OutputDebugStringA(errstr.c_str());//�f�o�b�N�o��
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
		nullptr,//�C���N���[�h�͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//�C���N���[�h�̓f�t�H���g
		"BasicPS", "ps_5_1",//�֐���BasicPS,�Ώۂ�ps_5_1
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob, &_errorBlob
	);

	if (FAILED(sts)) {

		if (sts == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			OutputDebugStringA("�t�@�C����������܂���");
			return 0;//exit�ȂǂŏI���������s���悤��蒼��
		}
		else {

			std::string errstr;
			errstr.resize(_errorBlob->GetBufferSize());
			//�G���[�����R�s�[
			std::copy_n((char*)_errorBlob->GetBufferPointer(),
				_errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += '\n';

			OutputDebugStringA(errstr.c_str());//�f�o�b�N�o��
		}
	}
	////////////////////////////////////////////////////////////////
	//PixelShader Compile	End
	////////////////////////////////////////////////////////////////

	sts = CreateRootSigature();
	sts = CreateGraphicPipeline();
	// 
	return sts;
}

HRESULT DirectX12_Graphics::CreateDevice() {
	HRESULT sts;
	sts = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_12_1,
		IID_PPV_ARGS(&_dev)
	);
	return sts;
}

HRESULT DirectX12_Graphics::CreateDXGIFactory() {
	std::vector<ComPtr<IDXGIAdapter>> adapters;
	ComPtr<IDXGIAdapter> tmpAdapter = nullptr;

	HRESULT sts;

	for (int i = 0;
		sts = _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND;
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
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateDXGIFactory Error!"), _T("error"), MB_OK);
	}

	return sts;
};

HRESULT DirectX12_Graphics::CreateCommand(){
	HRESULT sts;

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
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; //�^�C���A�E�g�Ȃ�
	cmdQueueDesc.NodeMask = 0;							//�A�_�v�^�[��1�����Ȃ��Ƃ��́A0�ł悢
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//�v���C�I���e�B�͎w��Ȃ�
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	sts = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateCommandQueue Error!"), _T("error"), MB_OK);
	}

	return sts;
}

HRESULT DirectX12_Graphics::CreateSwapChain(HWND hWnd,unsigned int window_width, unsigned int window_height) {
	HRESULT sts;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};

	swapChainDesc.Width = window_width;
	swapChainDesc.Height = window_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount = swapBufferCount;

	//�o�b�N�o�b�t�@�͐L�яk�݉\
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	//�t���b�v��͑��₩�ɔj��
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//���Ɏw��Ȃ�
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	//�t���X�N���[���ƃE�B���h�E��؂�ւ��\
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
	return sts;
}

HRESULT DirectX12_Graphics::CreateFence() {
	HRESULT sts;
	sts = _dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics Fence Error!"), _T("error"), MB_OK);
	}
	return sts;
}

HRESULT DirectX12_Graphics::CreateRTV() {
	HRESULT sts;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2; //�X���b�v�`�F�[���̕\���œ��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	sts = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_rtvHeaps));
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateRTVHeaps Error!"), _T("error"), MB_OK);
	}

	D3D12_RENDER_TARGET_VIEW_DESC _rtvDesc = {};
	_rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;//�K���}�l�␳����(sRGB)
	_rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE handle
		= _rtvHeaps->GetCPUDescriptorHandleForHeapStart();

	for (int idx = 0; idx < swapBufferCount; ++idx) {

		sts = _swapchain->GetBuffer(static_cast<UINT>(idx), IID_PPV_ARGS(&_backBuffers[idx]));

		if (sts != S_OK) {
			MessageBox(nullptr, _T("DirectX12_Graphics RTVBuffer Error!"), _T("error"), MB_OK);
			break;
		}

		_dev->CreateRenderTargetView(
			_backBuffers[idx].Get(),
			&_rtvDesc,
			handle
		);

		handle.ptr += _dev->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		);
	}

	return sts;
}

HRESULT DirectX12_Graphics::CreateDSV()
{
	HRESULT sts;

	DXGI_SWAP_CHAIN_DESC1 desc = {};
	sts = _swapchain->GetDesc1(&desc);
	//�[�x�o�b�t�@�쐬
	//�[�x�o�b�t�@�̎d�l
	//auto depthResDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT,
	//	desc.Width, desc.Height,
	//	1, 0, 1, 0,
	//	D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);


	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resdesc.DepthOrArraySize = 1;
	resdesc.Width = desc.Width;
	resdesc.Height = desc.Height;
	resdesc.Format = DXGI_FORMAT_D32_FLOAT;
	resdesc.SampleDesc.Count = 1;
	resdesc.SampleDesc.Quality = 0;
	resdesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resdesc.MipLevels = 1;
	resdesc.Alignment = 0;

	//�f�v�X�p�q�[�v�v���p�e�B
	auto depthHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	CD3DX12_CLEAR_VALUE depthClearValue(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	sts = _dev->CreateCommittedResource(
		&depthHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, //�f�v�X�������݂Ɏg�p
		&depthClearValue,
		IID_PPV_ARGS(_depthBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(sts)) {
		//�G���[����
		return sts;
	}

	//�[�x�̂��߂̃f�X�N���v�^�q�[�v�쐬
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};//�[�x�Ɏg����Ƃ��������킩��΂���
	dsvHeapDesc.NumDescriptors = 1;//�[�x�r���[1�̂�
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;//�f�v�X�X�e���V���r���[�Ƃ��Ďg��
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;


	sts = _dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(_dsvHeap.ReleaseAndGetAddressOf()));

	//�[�x�r���[�쐬
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;//�f�v�X�l��32bit�g�p
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;//�t���O�͓��ɂȂ�
	_dev->CreateDepthStencilView(_depthBuffer.Get(), &dsvDesc, _dsvHeap->GetCPUDescriptorHandleForHeapStart());
	return E_NOTIMPL;
}

HRESULT DirectX12_Graphics::CreateRootSigature() {

	HRESULT sts;
	////////////////////////////////////////////////////////////////
	//���[�g�V�O�l�N�`��	START
	////////////////////////////////////////////////////////////////

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descTblRange[2] = {};
	//�e�N�X�`���p : ���W�X�^�[[0]
	descTblRange[0].NumDescriptors = 1;//�e�N�X�`��1��
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//���:SRV
	descTblRange[0].BaseShaderRegister = 0;//0�ԃX���b�g����
	descTblRange[0].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//�萔�p : ���W�X�^�[[0]
	descTblRange[1].NumDescriptors = 1;//�萔1��
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//���:�萔
	descTblRange[1].BaseShaderRegister = 0;//0�ԃX���b�g����
	descTblRange[1].OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	/*
	���W�X�^�[�̎�ʂ��Ⴄ����0�Ԃŏd�����Ă��Ă����Ȃ�
	*/

	D3D12_ROOT_PARAMETER rootparam[2] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;//�f�B�X�N���v�^�����W��
	rootparam[0].ShaderVisibility =
		D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_�[���猩����

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRange[1];
	rootparam[1].DescriptorTable.NumDescriptorRanges = 1;//�f�B�X�N���v�^�����W��
	rootparam[1].ShaderVisibility =
		D3D12_SHADER_VISIBILITY_VERTEX;//���_�V�F�[�_�[���猩����

	rootSignatureDesc.pParameters = rootparam;
	rootSignatureDesc.NumParameters = 2;

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor =
		D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ShaderVisibility =
		D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_�[���猩����
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//���T���v���C�Y���Ȃ�

	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	ID3DBlob* rootSigBlob = nullptr;

	sts = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSigBlob,
		&_errorBlob
	);
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics rootSigBlobError!"), _T("error"), MB_OK);
	}

	sts = _dev->CreateRootSignature(
		0,//nodemask 0�ł悢
		rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&_rootSignature)
	);
	////////////////////////////////////////////////////////////////
	//���[�g�V�O�l�N�`��	END
	////////////////////////////////////////////////////////////////

	return sts;
}

HRESULT DirectX12_Graphics::CreateGraphicPipeline() {
	HRESULT sts;

	////////////////////////////////////////////////////////////////
	//�O���t�B�b�N�p�C�v���C���X�e�[�g	Start
	////////////////////////////////////////////////////////////////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC _pipeline = {};
	_pipeline.pRootSignature = nullptr;

	_pipeline.VS.pShaderBytecode = _vsBlob->GetBufferPointer();
	_pipeline.VS.BytecodeLength = _vsBlob->GetBufferSize();
	_pipeline.PS.pShaderBytecode = _psBlob->GetBufferPointer();
	_pipeline.PS.BytecodeLength = _psBlob->GetBufferSize();

	//�f�t�H���g�̃T���v���}�X�N(0xffffffff)
	_pipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	//�A���`�G�C���A�X
	_pipeline.RasterizerState.MultisampleEnable = false;

	_pipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	_pipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//����h��Ԃ�
	_pipeline.RasterizerState.DepthClipEnable = true; //�[�x�����̃N���b�s���O:�L��

	_pipeline.DepthStencilState.DepthEnable = true;
	_pipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	_pipeline.DepthStencilState.DepthFunc =
		D3D12_COMPARISON_FUNC_LESS;
	_pipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	_pipeline.BlendState.AlphaToCoverageEnable = false;
	_pipeline.BlendState.IndependentBlendEnable = false;

	//�����_�[�^�[�Q�b�g�f�X�N�̓K��
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = { };
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.LogicOpEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	_pipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;

	//���̓��C�A�E�g�̓K��
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{	//���W���
		"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{	//�@�����
		"NORMAL",0,DXGI_FORMAT_R32G32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
		{	//UV���
		"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
			{	//�{�[���ԍ�
		"BONE_NO",0,DXGI_FORMAT_R32G32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
			{	//�E�F�C�g���
		"WEIGHT",0,DXGI_FORMAT_R32G32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},
			{	//�ӂ���
		"EDGE_FLG",0,DXGI_FORMAT_R32G32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		},

	};

	_pipeline.InputLayout.pInputElementDescs = inputLayout;
	_pipeline.InputLayout.NumElements = _countof(inputLayout);

	_pipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	//�O�p�`�ō\��
	_pipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	//�����_�[�^�[�Q�b�g�̐ݒ�
	_pipeline.NumRenderTargets = 1;
	_pipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;// 0 ~ 1�ɐ��K�����ꂽRGBA
	//�A���`�G�C���A�V���O�̐ݒ�
	_pipeline.SampleDesc.Count = 1;  //�T���v�����O��1�s�N�Z���ɂ� 1
	_pipeline.SampleDesc.Quality = 0;//�N�I���e�B:�Œ�
	//���[�g�V�O�l�N�`���ݒ�
	_pipeline.pRootSignature = _rootSignature.Get();

	sts = _dev->CreateGraphicsPipelineState(&_pipeline, IID_PPV_ARGS(&_pipelinestate));

	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics pipeline Error!"), _T("error"), MB_OK);
	}
	////////////////////////////////////////////////////////////////
	//�O���t�B�b�N�p�C�v���C���X�e�[�g	End
	////////////////////////////////////////////////////////////////
	// 
	return sts;
}
bool DirectX12_Graphics::BeforeRender()
{
	HRESULT sts = 0;

	auto bbIdx = _swapchain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER BarrierDesc = {};
	BarrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	BarrierDesc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	BarrierDesc.Transition.pResource = _backBuffers[bbIdx].Get();
	BarrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	BarrierDesc.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	BarrierDesc.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	_cmdList->ResourceBarrier(1, &BarrierDesc);

	//�����_�[�^�[�Q�b�g���w��
	auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	_cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);
	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

	//DSV�Ƀ����_�[�^�[�Q�b�g���֘A�t����
	auto dsvH = _dsvHeap->GetCPUDescriptorHandleForHeapStart();
	_cmdList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
	_cmdList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

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

	_cmdList->SetPipelineState(_pipelinestate.Get());

	_cmdList->RSSetViewports(1, &_viewPort);
	_cmdList->RSSetScissorRects(1, &_scissorrect);
	_cmdList->SetGraphicsRootSignature(_rootSignature.Get());

	return sts;
}

void DirectX12_Graphics::Draw()
{

}

bool DirectX12_Graphics::AfterRender()
{
	HRESULT sts;

	//���߂̃N���[�Y
	_cmdList->Close();

	//�R�}���h���X�g�̎��s
	ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
	_cmdQueue->ExecuteCommandLists(1, cmdlists);
	////�҂�
	_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

	if (_fence->GetCompletedValue() != _fenceVal) {
		auto event = CreateEvent(nullptr, false, false, nullptr);
		_fence->SetEventOnCompletion(_fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);
	}
	_cmdAllocator->Reset();//�L���[���N���A
	sts = _cmdList->Reset(_cmdAllocator.Get(), nullptr);//�ĂуR�}���h���X�g�����߂鏀��

	//�t���b�v
	_swapchain->Present(1, 0);
	return sts;
}

void DirectX12_Graphics::Exit()
{
	SAFE_RELEASE(_rtvHeaps);
}
