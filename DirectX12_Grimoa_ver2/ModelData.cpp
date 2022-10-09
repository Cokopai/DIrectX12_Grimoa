#include <tchar.h>
#include "ModelData.h"
#include "macro.h"
#include "d3dx12.h"
#include "DirectX12_Graphics.h"
#include "PMD_Layout.h"

using namespace DirectX;

bool PMDActor::Create()
{
	HRESULT sts;

	FILE* fp;
	fopen_s(&fp, "Model/��������.pmd", "rb");

	char signature[3] = {};
	fread(signature, sizeof(signature), 1, fp);
	fread(&m_pmdHeader, sizeof(m_pmdHeader), 1, fp);
	
	//Vertex�ǂݎ��
	fread(&m_vertNum, sizeof(m_vertNum), 1, fp);
	_vertices.resize(m_vertNum * pmd_vertex_size);
	fread(_vertices.data(), _vertices.size(), 1, fp);
	
	//Index�ǂݎ��
	fread(&m_idxNum, sizeof(m_idxNum), 1, fp);
	_indices.resize(m_idxNum);
	fread(_indices.data(), _indices.size() * sizeof(_indices[0]), 1, fp);

	fread(&m_materialNum,sizeof(m_materialNum),1,fp);
	_pmdMaterials.resize(m_materialNum);
	fread(_pmdMaterials.data(), _pmdMaterials.size() * sizeof(PMDMaterial), 1, fp);

	//std::vector<unsigned char> pmd_vertices;
	//pmd_vertices.resize(vertexNum);
	fclose(fp);

	XMStoreFloat4x4(&m_mtx,XMMatrixIdentity());

	MaterialTransport(m_materialNum);
	sts = CreateMaterialBuffer();

	auto dev = DirectX12_Graphics::GetInstance()->GetDXDevice();

	D3D12_HEAP_PROPERTIES buffheapprop = { };

	buffheapprop.Type = D3D12_HEAP_TYPE_UPLOAD;
	buffheapprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	buffheapprop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	/////////////////////////////////////////////////////////////
	//VertexBuffer������	Start
	/////////////////////////////////////////////////////////////
	D3D12_RESOURCE_DESC buffdesc = { };

	buffdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	buffdesc.Width = sizeof(_vertices);
	buffdesc.Height = 1;
	buffdesc.DepthOrArraySize = 1;
	buffdesc.MipLevels = 1;
	buffdesc.Format = DXGI_FORMAT_UNKNOWN;
	buffdesc.SampleDesc.Count = 1;
	buffdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	buffdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(_vertices.size());

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

	std::copy(std::begin(_vertices), std::end(_vertices), vertMap);

	_vertexBuffer->Unmap(0, nullptr);

	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vbView.SizeInBytes = _vertices.size();
	_vbView.StrideInBytes = pmd_vertex_size;
	/////////////////////////////////////////////////////////////
	//VertexsBuffer������	End
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	//IndexBuffer������	Start
	/////////////////////////////////////////////////////////////
	buffdesc.Width = sizeof(_indices);

	resDesc = CD3DX12_RESOURCE_DESC::Buffer(_indices.size() * sizeof(_indices[0]));

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
	std::copy(std::begin(_indices), std::end(_indices), mappedIdx);
	_indexBuffer->Unmap(0, nullptr);

	_idView.BufferLocation = _indexBuffer->GetGPUVirtualAddress();
	_idView.Format = DXGI_FORMAT_R16_UINT;
	_idView.SizeInBytes = _indices.size() * sizeof(_indices[0]);
	/////////////////////////////////////////////////////////////
	//IndexBuffer������	End
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	//DescRiptorHeap������	Start
	/////////////////////////////////////////////////////////////
	//�V�F�[�_�[���猩����悤��
	D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc = {};
	descHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�^�C�v:SRV
	descHeapDesc.NodeMask = 0;
	descHeapDesc.NumDescriptors = 2;
	descHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	sts = dev->CreateDescriptorHeap(&descHeapDesc, IID_PPV_ARGS(&_basicDescHeap));
	if (sts != S_OK) {
		MessageBox(nullptr, _T("DirectX12_Graphics CreateDescriptorHeap Error!"), _T("error"), MB_OK);
	}

	auto basicHeapHandle = _basicDescHeap->GetCPUDescriptorHandleForHeapStart();
	/////////////////////////////////////////////////////////////
	//DescRiptorHeap������	End
	/////////////////////////////////////////////////////////////

	/*
	/////////////////////////////////////////////////////////////
	//ShaderResourceView������	Start
	/////////////////////////////////////////////////////////////
	
	//WriteToSubResource �Ńe�N�X�`������]�����邽�߂̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES _texheapProp = {};
	//�J�X�^���ݒ�
	_texheapProp.Type = D3D12_HEAP_TYPE_CUSTOM;
	//���C�g�o�b�N�ŏ�������
	_texheapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	//�]����L0,�܂�CPU�����璼�ڍs��
	_texheapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	//�P��A�_�v�^�[�̂��� 0
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
		nullptr,//�S�̈�R�s�[
		img->pixels,
		img->rowPitch,
		img->slicePitch
	);

	if (sts != S_OK) {
		MessageBox(nullptr, _T("textureBuffer Copy Texture Error!"), _T("error"), MB_OK);
	}
	*/
	/*
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	srvDesc.Format = metaData.format;
	srvDesc.Shader4ComponentMapping =
		D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂łP

	dev->CreateShaderResourceView(
		_materialBuffer.Get(),
		&srvDesc,
		basicHeapHandle
	);
	*/
	
	/////////////////////////////////////////////////////////////
	//ShaderResourceView������	End
	/////////////////////////////////////////////////////////////

	//basicHeapHandle.ptr += dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	/////////////////////////////////////////////////////////////
	//ConstBuffer������	Start
	/////////////////////////////////////////////////////////////

	heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	resDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(SceneData) + 0xff) & ~0xff);

	sts = dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_constBuffer)
	);

	if (FAILED(sts)) {
		MessageBox(nullptr, _T("ConstantBuffer Create Error!"), _T("error"), MB_OK);
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = _constBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = _constBuffer.Get()->GetDesc().Width;

	dev->CreateConstantBufferView(&cbvDesc, basicHeapHandle);


	/////////////////////////////////////////////////////////////
	//ConstBuffer������	End
	/////////////////////////////////////////////////////////////
	return sts;
}

void PMDActor::TestMeshInit()
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

void PMDActor::Update()
{
	angle += 0.01f;
	XMMATRIX world_mtx = XMMatrixRotationY(angle);

	XMFLOAT3 eye(0, 10, -15);
	XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);

	//�r���[�ϊ�
	XMMATRIX view_mtx = XMMatrixLookAtLH(
		XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	//�v���W�F�N�V�����ϊ�
	XMMATRIX proj_mtx = XMMatrixPerspectiveFovLH(
		XM_PIDIV2,//��p��90��
		static_cast<float>(CLIENT_WIDTH) / static_cast<float>(CLIENT_HEIGHT),//�A�X�y�N�g��
		1.0f,//�j�A�N���b�v
		100.0f//�t�@�[�N���b�v
	);

	SceneData* mapMatrix;

	_constBuffer->Map(0, nullptr, (void**)&mapMatrix);
	XMStoreFloat4x4(&mapMatrix->world,world_mtx);
	XMStoreFloat4x4(&mapMatrix->view,view_mtx);
	XMStoreFloat4x4(&mapMatrix->proj,proj_mtx);
	mapMatrix->eye = eye;
}

void PMDActor::Draw(ID3D12Device* dev)
{
	auto _cmdList = DirectX12_Graphics::GetInstance()->GetCommandList();

	_cmdList->IASetVertexBuffers(0, 1, &_vbView);
	_cmdList->IASetIndexBuffer(&_idView);

	_cmdList->SetDescriptorHeaps(1, &_basicDescHeap);
	_cmdList->SetGraphicsRootDescriptorTable(0, _basicDescHeap->GetGPUDescriptorHandleForHeapStart());

	auto HeapHandle = _basicDescHeap->GetGPUDescriptorHandleForHeapStart();
	
	HeapHandle.ptr += dev->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
	);
	
	/*
	//�}�e���A��
	_cmdList->SetDescriptorHeaps(1, &_materialDescHeap);

	auto materialH = _materialDescHeap->GetGPUDescriptorHandleForHeapStart();
	unsigned int idxOffset = 0;

	auto cbvsrvIncSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (auto& m : _materials) {
		_cmdList->SetGraphicsRootDescriptorTable(1, materialH);
		_cmdList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
		materialH.ptr += cbvsrvIncSize;
		idxOffset += m.indicesNum;
	}

	*/
	
	_cmdList->SetGraphicsRootDescriptorTable(1, HeapHandle);

	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_cmdList->DrawIndexedInstanced(m_idxNum, 1, 0, 0, 0);
	
}

unsigned long PMDActor::Release()
{
	_vertices.clear();
	_indices.clear();
	SAFE_RELEASE(_basicDescHeap);
	SAFE_RELEASE(_materialDescHeap);

	return 0;
}

void PMDActor::MaterialTransport(unsigned int matNum)
{
	_materials.resize(matNum);
	for (int i = 0; i < matNum; ++i) {
		_materials[i].indicesNum = _pmdMaterials[i].indecesNum;
		_materials[i].material.diffuse = _pmdMaterials[i].diffuse;
		_materials[i].material.alpha = _pmdMaterials[i].alpha;
		_materials[i].material.specular = _pmdMaterials[i].specular;
		_materials[i].material.specularity = _pmdMaterials[i].specularity;
		_materials[i].material.ambient = _pmdMaterials[i].ambient;
		_materials[i].additional.toonIdx = _pmdMaterials[i].toonIdx;
	}

}

HRESULT PMDActor::CreateMaterialBuffer()
{
	HRESULT sts;
	auto dev = DirectX12_Graphics::GetInstance()->GetDXDevice();

	auto materialBufferSize = sizeof(MaterialForHlsl);
	materialBufferSize = (materialBufferSize + 0xff) & ~0xff;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(
		materialBufferSize * m_materialNum);

	sts = dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_materialBuffer)
	);

	if (FAILED(sts)) {
		MessageBox(nullptr, _T("MaterialBuffer Create Error!"), _T("error"), MB_OK);
	}

	sts = _materialBuffer->Map(0,nullptr,(void**)&_mapMaterial);

	for (auto& m : _materials) {
		*((MaterialForHlsl*)_mapMaterial) = m.material;
		_mapMaterial += materialBufferSize;//���̃A���C�����g�ʒu�܂Ői�߂�i256�̔{���j
	}

	D3D12_DESCRIPTOR_HEAP_DESC materialDescHeapDesc = {};
	materialDescHeapDesc.NumDescriptors = m_materialNum;//�}�e���A�����Ԃ�(�萔1�A�e�N�X�`��3��)
	materialDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	materialDescHeapDesc.NodeMask = 0;

	materialDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���
	sts = dev->CreateDescriptorHeap(&materialDescHeapDesc, IID_PPV_ARGS(&_materialDescHeap));//����

	if (FAILED(sts)) {
		MessageBox(nullptr, _T("materialDescHeap Create Error!"), _T("error"), MB_OK);
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = _materialBuffer->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = static_cast<UINT>(materialBufferSize);

	auto matDescHeapH = _materialDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto incSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (size_t i = 0; i < m_materialNum; ++i) {
		//�}�e���A���Œ�o�b�t�@�r���[
		dev->CreateConstantBufferView(&matCBVDesc, matDescHeapH);
		matDescHeapH.ptr += incSize;
		matCBVDesc.BufferLocation += materialBufferSize;

		/*
		if (textureResources[i] == nullptr) {
			srvDesc.Format = whiteTex->GetDesc().Format;
			_dev->CreateShaderResourceView(whiteTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = textureResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(textureResources[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (sphResources[i] == nullptr) {
			srvDesc.Format = whiteTex->GetDesc().Format;
			_dev->CreateShaderResourceView(whiteTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = sphResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(sphResources[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (spaResources[i] == nullptr) {
			srvDesc.Format = blackTex->GetDesc().Format;
			_dev->CreateShaderResourceView(blackTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = spaResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(spaResources[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;


		if (toonResources[i] == nullptr) {
			srvDesc.Format = gradTex->GetDesc().Format;
			_dev->CreateShaderResourceView(gradTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = toonResources[i]->GetDesc().Format;
			_dev->CreateShaderResourceView(toonResources[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;
		*/
	}

	return sts;
}
