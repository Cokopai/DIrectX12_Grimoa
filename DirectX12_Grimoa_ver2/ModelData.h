#pragma once
#pragma comment (lib,"DirectXTex.lib")

#include	<wrl/client.h>
#include	<d3d12.h>
#include	<DirectXTex.h>
#include	<vector>
#include	"vertex.h"
#include	"PMD_Layout.h"

using Microsoft::WRL::ComPtr;

struct Texture {

};

class PMDActor
{
private:
	XMFLOAT4X4 m_mtx;// = XMMatrixIdentity();
	float angle = 0;
	unsigned int				 m_vertNum;
	unsigned int				 m_idxNum;
	unsigned int 				 m_materialNum;

	PMDHeader					 m_pmdHeader;

	std::vector<unsigned char>   _vertices;
	std::vector<unsigned short>  _indices;
	std::vector<PMDMaterial>	 _pmdMaterials;//マテリアル読み取り用
	std::vector<Material>		 _materials;//マテリアル保持用

	ComPtr<ID3D12Resource>		 _vertexBuffer = nullptr;
	ComPtr<ID3D12Resource>		 _indexBuffer = nullptr;
	ComPtr<ID3D12Resource>		 _materialBuffer = nullptr;

	//ComPtr<ID3D12Resource>		 _texBuffer = nullptr;

	ComPtr<ID3D12Resource>		 _constBuffer = nullptr;
	ID3D12DescriptorHeap*		 _basicDescHeap = nullptr;
	ID3D12DescriptorHeap*		 _materialDescHeap = nullptr;

	char*						 _mapMaterial = nullptr;

	//DirectX::TexMetadata metaData = {};
	//DirectX::ScratchImage scratchImg = {};
public:
	PMDActor() { };

	PMDActor(const PMDActor&) = delete;
	PMDActor& operator=(const PMDActor&) = delete;
	PMDActor(PMDActor&&) = delete;
	PMDActor& operator=(PMDActor&&) = delete;

	D3D12_VERTEX_BUFFER_VIEW _vbView = { };
	D3D12_INDEX_BUFFER_VIEW _idView = { };

	bool Create();

	void TestMeshInit();
	void Update();
	void Draw(ID3D12Device* dev);
	unsigned long Release();
	ID3D12DescriptorHeap* GetBasicDescHeap() { return _basicDescHeap; };
	void MaterialTransport(unsigned int matNum);
	HRESULT CreateMaterialBuffer();

	void AddRef() { };
	~PMDActor() { Release(); };
};

