#include <tchar.h>
#include "Mesh.h"
#include "macro.h"
#include "d3dx12.h"
#include "PMD_Layout.h"

using namespace DirectX;

Mesh::Mesh(ID3D12Device* dev,std::vector<unsigned char>& vertices, std::vector<unsigned short>& indices)
	:m_vertices(vertices),m_indices(indices)
{
	Mesh::Create(dev,vertices,indices);
}

bool Mesh::Create(ID3D12Device* dev, std::vector<unsigned char>& vertices, std::vector<unsigned short> indices)
{
	HRESULT sts;
	TestMeshInit();

	m_vertices = vertices;

	mtx = XMMatrixIdentity();

	
	sts = DirectX::LoadFromWICFile(
		_T("img/textest.png"), WIC_FLAGS_NONE,
		&metaData, scratchImg
	);
	
	auto img = scratchImg.GetImage(0,0,0);
	

	D3D12_HEAP_PROPERTIES buffheapprop = { };

	buffheapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	buffheapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	buffheapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	/////////////////////////////////////////////////////////////
	//VertexBuffer初期化	Start
	/////////////////////////////////////////////////////////////
	D3D12_RESOURCE_DESC buffdesc = { };

	buffdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buffdesc.Width = sizeof(m_vertices);
	buffdesc.Height = 1;
	buffdesc.DepthOrArraySize = 1;
	buffdesc.MipLevels = 1;
	buffdesc.Format = DXGI_FORMAT_UNKNOWN;
	buffdesc.SampleDesc.Count = 1;
	buffdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	buffdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_vertices.size());

	sts = dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer)
	);
	if (sts != S_OK) {
		MessageBox(nullptr, _T("VertexBuffer Create Error!"), _T("error"), MB_OK);
	}

	unsigned char* vertMap = nullptr;

	sts = _vertexBuffer->Map(0, nullptr, (void**)&vertMap);

	std::copy(std::begin(m_vertices), std::end(m_vertices), vertMap);

	_vertexBuffer->Unmap(0, nullptr);

	vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = m_vertices.size();
	vbView.StrideInBytes = pmd_vertex_size;
	/////////////////////////////////////////////////////////////
	//VertexsBuffer初期化	End
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	//IndexBuffer初期化	Start
	/////////////////////////////////////////////////////////////
	buffdesc.Width = sizeof(m_indices);

	resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_indices.size() * sizeof(indices[0]));

	sts = dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_indexBuffer)
	);
	if (sts != S_OK) {
		MessageBox(nullptr, _T("IndexBuffer Create Error!"), _T("error"), MB_OK);
	}

	unsigned short* mappedIdx = nullptr;
	_indexBuffer->Map(0, nullptr, (void**)&mappedIdx);
	std::copy(std::begin(m_indices), std::end(m_indices), mappedIdx);
	_indexBuffer->Unmap(0, nullptr);

	idView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	idView.Format = DXGI_FORMAT_R16_UINT;
	idView.SizeInBytes = indices.size() * sizeof(m_indices[0]);
	/////////////////////////////////////////////////////////////
	//IndexBuffer初期化	End
	/////////////////////////////////////////////////////////////
	
	/////////////////////////////////////////////////////////////
	//DescRiptorHeap初期化	Start
	/////////////////////////////////////////////////////////////
	//シェーダーから見えるように
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//タイプ:SRV
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 2;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	sts = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&_basicDescHeap));
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateDescriptorHeap Error!"), _T("error"), MB_OK);
	}

	auto basicHeapHandle = _basicDescHeap->GetCPUDescriptorHandleForHeapStart();
	/////////////////////////////////////////////////////////////
	//DescRiptorHeap初期化	End
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	//ShaderResourceView初期化	Start
	/////////////////////////////////////////////////////////////
	//WriteToSubResource でテクスチャ情報を転送するためのヒープ設定
	D3D12_HEAP_PROPERTIES _texheapProp = {};
	//カスタム設定
	_texheapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	//ライトバックで書き込み
	_texheapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	//転送はL0,つまりCPU側から直接行う
	_texheapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//単一アダプターのため 0
	_texheapProp.CreationNodeMask = 0;
	_texheapProp.VisibleNodeMask = 0;

	D3D12_RESOURCE_DESC texdesc = {};
	texdesc.Format = metaData.format;
	texdesc.Width = metaData.width;
	texdesc.Height = metaData.height;
	texdesc.DepthOrArraySize = metaData.arraySize;
	texdesc.SampleDesc.Count = 1;
	texdesc.SampleDesc.Quality = 0;
	texdesc.MipLevels = metaData.mipLevels;
	texdesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metaData.dimension);
	texdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	texdesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	sts = dev->CreateCommittedResource(
		&_texheapProp,
		D3D12_HEAP_FLAG_NONE,
		&texdesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&_texBuffer)
	);
	if (sts != S_OK) {
		MessageBox(nullptr, _T("textureBuffer Create Error!"), _T("error"), MB_OK);
	}

	sts = _texBuffer->WriteToSubresource(
		0,
		nullptr,//全領域コピー
		img->pixels,
		img->rowPitch,
		img->slicePitch
	);

	if (sts != S_OK) {
		MessageBox(nullptr, _T("textureBuffer Copy Texture Error!"), _T("error"), MB_OK);
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = metaData.format;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので１

	dev->CreateShaderResourceView(
		_texBuffer.Get(),
		&srvDesc,
		basicHeapHandle
	);
	/////////////////////////////////////////////////////////////
	//ShaderResourceView初期化	End
	/////////////////////////////////////////////////////////////

	basicHeapHandle.ptr += dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	/////////////////////////////////////////////////////////////
	//ConstBuffer初期化	Start
	/////////////////////////////////////////////////////////////
	
	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(XMMATRIX) + 0xff) & ~0xff);

	dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_constBuffer)
		);

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _constBuffer ->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = _constBuffer.Get()->GetDesc().Width;
	
	dev->CreateConstantBufferView(&cbvDesc, basicHeapHandle);
	/////////////////////////////////////////////////////////////
	//ConstBuffer初期化	End
	/////////////////////////////////////////////////////////////
	return sts;
}

void Mesh::TestMeshInit()
{
	/*
	m_vertices.resize(4);
	Vertex vertices[]{
		{{-1.0f,-1.0f,0.0f},{0.0f,1.0f}},
		{{-1.0f,1.0f,0.0f},{0.0f,0.0f}},
		{{1.0f, -1.0f, 0.0f},{1.0f,1.0f}},
		{{1.0f,1.0f,0.0f},{1.0f,0.0f}}
	};
	std::copy(std::begin(vertices), std::end(vertices), m_vertices.begin());

	m_indeces.resize(6);
	unsigned short indeces[]{
		0,1,2,
		2,1,3
	};
	std::copy(std::begin(indeces), std::end(indeces), m_indeces.begin());
	*/
}

void Mesh::Draw()
{
	angle += 0.01f;
	mtx = XMMatrixRotationY(angle);

	XMFLOAT3 eye(0, 10, -15);
	XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);

	mtx *= XMMatrixLookAtLH(
		XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));

	mtx *= XMMatrixPerspectiveFovLH(
		XM_PIDIV2,//画角は90°
		static_cast<float>(CLIENT_WIDTH) / static_cast<float>(CLIENT_HEIGHT),//アスペクト比
		1.0f,//ニアクリップ
		100.0f//ファークリップ
	);

	XMMATRIX* mapMatrix;
	_constBuffer->Map(0, nullptr, (void**)&mapMatrix);
	*mapMatrix = mtx;
}

unsigned long Mesh::Release()
{
	m_vertices.clear();
	SAFE_RELEASE(_basicDescHeap);

	return 0;
}
