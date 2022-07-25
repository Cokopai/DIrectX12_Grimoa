#include <Windows.h>
#include <tchar.h>
#include <string>
#include <d3d12sdklayers.h>
#include <tchar.h>
#include <DirectXMath.h>
#include<vector>

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
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE; //�^�C���A�E�g�Ȃ�
	cmdQueueDesc.NodeMask = 0;							//�A�_�v�^�[��1�����Ȃ��Ƃ��́A0�ł悢
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;//�v���C�I���e�B�͎w��Ȃ�
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	sts = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));
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

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2; //�X���b�v�`�F�[���̕\���œ��
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	sts = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_rtvHeaps));
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateRTVHeaps Error!"), _T("error"), MB_OK);
	}
	
	
	//�V�F�[�_�[���猩����悤��
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�^�C�v:SRV
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 1;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	sts = _dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&_texDescHeap));
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateDescriptorHeap Error!"), _T("error"), MB_OK);
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
		nullptr,//�C���N���[�h�͂Ȃ�
		D3D_COMPILE_STANDARD_FILE_INCLUDE,//�C���N���[�h�̓f�t�H���g
		"BasicVS", "vs_5_0",//�֐���BasicVS,�Ώۂ�vs_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_vsBlob, &errorBlob
	);

	if (FAILED(sts)) {

		if (sts == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			OutputDebugStringA("�t�@�C����������܂���");
			return 0;//exit�ȂǂŏI���������s���悤��蒼��
		}
		else {

			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			//�G���[�����R�s�[
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
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
		"BasicPS", "ps_5_0",//�֐���BasicPS,�Ώۂ�ps_5_0
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_psBlob, &errorBlob
	);

	if (FAILED(sts)) {

		if (sts == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
			OutputDebugStringA("�t�@�C����������܂���");
			return 0;//exit�ȂǂŏI���������s���悤��蒼��
		}
		else {

			std::string errstr;
			errstr.resize(errorBlob->GetBufferSize());
			//�G���[�����R�s�[
			std::copy_n((char*)errorBlob->GetBufferPointer(),
				errorBlob->GetBufferSize(),
				errstr.begin());
			errstr += '\n';

			OutputDebugStringA(errstr.c_str());//�f�o�b�N�o��
		}
	}
	////////////////////////////////////////////////////////////////
	//PixelShader Compile	End
	////////////////////////////////////////////////////////////////

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
		{	//UV���
		"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,
		D3D12_APPEND_ALIGNED_ELEMENT,
		D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0
		}
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

	////////////////////////////////////////////////////////////////
	//���[�g�V�O�l�N�`��	START
	////////////////////////////////////////////////////////////////

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};

	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descTblRange = {};
	descTblRange.NumDescriptors = 1;//�e�N�X�`��1��
	descTblRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//���:SRV
	descTblRange.BaseShaderRegister = 0;//0�ԃX���b�g����
	descTblRange.OffsetInDescriptorsFromTableStart =
		D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootparam = {};
	rootparam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	//�s�N�Z���V�F�[�_�[���猩����
	rootparam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	rootparam.DescriptorTable.pDescriptorRanges = &descTblRange;
	//�f�B�X�N���v�^�����W��
	rootparam.DescriptorTable.NumDescriptorRanges = 1;

	rootSignatureDesc.pParameters = &rootparam;
	rootSignatureDesc.NumParameters = 1;

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
		&errorBlob
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

	//���[�g�V�O�l�N�`���ݒ�
	_pipeline.pRootSignature = _rootSignature;

	sts = _dev->CreateGraphicsPipelineState(&_pipeline, IID_PPV_ARGS(&_pipelinestate));

	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics pipeline Error!"), _T("error"), MB_OK);
	}
	////////////////////////////////////////////////////////////////
	//�O���t�B�b�N�p�C�v���C���X�e�[�g	End
	////////////////////////////////////////////////////////////////

	textureData.resize(256 * 256);

	//RandomTexture(textureData);
	for (auto& rgba : textureData) {
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
		rgba.A = 255;
	}
	return sts;
}

bool DirectX12_Graphics::BeforeRender()
{
	HRESULT sts;
	
	//WriteToSubResource �Ńe�N�X�`������]�����邽�߂̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES heapProp = {};
	//�J�X�^���ݒ�
	heapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	//���C�g�o�b�N�ŏ�������
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	//�]����L0,�܂�CPU�����璼�ڍs��
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//�P��A�_�v�^�[�̂��� 0
	heapProp.CreationNodeMask = 0;
	heapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC resdesc = {};
	resdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;//RGBA�t�H�[�}�b�g
	resdesc.Width = 256;
	resdesc.Height = 256;
	resdesc.DepthOrArraySize = 1;
	resdesc.SampleDesc.Count = 1;
	resdesc.SampleDesc.Quality = 0;
	resdesc.MipLevels = 1;
	resdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	resdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	ID3D12Resource* texBuffer = nullptr;
	sts = _dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texBuffer)
	); 
	if (sts != S_OK) {
		MessageBox(nullptr, _T("textureBuffer Create Error!"), _T("error"), MB_OK);
	}

	sts = texBuffer->WriteToSubresource(
		0,
		nullptr,//�S�̈�փR�s�[
		textureData.data(),//���f�[�^�T�C�Y
		sizeof(TexRGBA) * 256,//1���C���T�C�Y
		sizeof(TexRGBA) * textureData.size()//�S�T�C�Y
	);
	if (sts != S_OK) {
		MessageBox(nullptr, _T("textureBuffer Copy Texture Error!"), _T("error"), MB_OK);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂łP

	_dev->CreateShaderResourceView(
		texBuffer,
		&srvDesc,
		_texDescHeap->GetCPUDescriptorHandleForHeapStart()
	);

	return sts;
}

void DirectX12_Graphics::Draw()
{
	HRESULT sts = 0;

	Vertex vertices[]{
		{{-0.4f,-0.7f,0.0f},{0.0f,1.0f}},
		{{-0.4f,0.7f,0.0f},{0.0f,0.0f}},
		{{0.4f, -0.7f, 0.0f},{1.0f,1.0f}},
		{{0.4f,0.7f,0.0f},{1.0f,0.0f}}
	};

	unsigned short indeces[]{
		0,1,2,
		2,1,3
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

	//�����_�[�^�[�Q�b�g���w��
	auto rtvH = _rtvHeaps->GetCPUDescriptorHandleForHeapStart();
	rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
	_cmdList->OMSetRenderTargets(1, &rtvH, false, nullptr);

	_cmdList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

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

	ID3D12Resource* vertexBuffer = nullptr;

	ID3D12Device* _dev = DirectX12_Graphics::GetInstance()->GetDXDevice();
	sts = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)
	);
	if (sts != S_OK) {
		MessageBox(nullptr, _T("VertexBuffer Create Error!"), _T("error"), MB_OK);
	}

	Vertex* vertMap = nullptr;

	sts = vertexBuffer->Map(0, nullptr, (void**)&vertMap);

	std::copy(std::begin(vertices), std::end(vertices), vertMap);

	vertexBuffer->Unmap(0, nullptr);

	D3D12_VERTEX_BUFFER_VIEW vbView = { };

	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = sizeof(vertices);
	vbView.StrideInBytes = sizeof(vertices[0]);

	ID3D12Resource* indexBuffer = nullptr;
	resdesc.Width = sizeof(indeces);

	sts = _dev->CreateCommittedResource(
		&heapprop,
		D3D12_HEAP_FLAG_NONE,
		&resdesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)
	);
	if (sts != S_OK) {
		MessageBox(nullptr, _T("IndexBuffer Create Error!"), _T("error"), MB_OK);
	}

	unsigned short* mappedIdx = nullptr;
	indexBuffer->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(indeces), std::end(indeces), mappedIdx);
	indexBuffer->Unmap(0, nullptr);

	D3D12_INDEX_BUFFER_VIEW idView = { };

	idView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	idView.Format = DXGI_FORMAT_R16_UINT;
	idView.SizeInBytes = sizeof(indeces);

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
	_cmdList->IASetVertexBuffers(0, 1, &vbView);
	_cmdList->IASetIndexBuffer(&idView);

	_cmdList->SetGraphicsRootSignature(_rootSignature);
	_cmdList->SetDescriptorHeaps(1, &_texDescHeap);
	_cmdList->SetGraphicsRootDescriptorTable(0, _texDescHeap->GetGPUDescriptorHandleForHeapStart());

	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
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
	SAFE_RELEASE(_pipelinestate);
	SAFE_RELEASE(_rootSignature);
	SAFE_RELEASE(_texDescHeap);
	SAFE_RELEASE(_rtvHeaps);
}
