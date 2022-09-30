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

class Mesh
{
private:
	XMMATRIX mtx;// = XMMatrixIdentity();
	float angle = 0;

	std::vector<unsigned char>& m_vertices;
	std::vector<unsigned short>& m_indices;

	ComPtr<ID3D12Resource> _vertexBuffer = nullptr;
	ComPtr<ID3D12Resource> _indexBuffer = nullptr;

	ComPtr<ID3D12Resource>		 _texBuffer = nullptr;
	ComPtr<ID3D12Resource>		 _constBuffer = nullptr;
	ID3D12DescriptorHeap*		 _basicDescHeap = nullptr;

	DirectX::TexMetadata metaData = {};
	DirectX::ScratchImage scratchImg = {};
public:
	Mesh(ID3D12Device* dev,std::vector<unsigned char>&, std::vector<unsigned short>&);

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&&) = delete;
	Mesh& operator=(Mesh&&) = delete;

	D3D12_VERTEX_BUFFER_VIEW vbView = { };
	D3D12_INDEX_BUFFER_VIEW idView = { };

	bool Create(ID3D12Device* dev, std::vector<unsigned char>& vertices, std::vector<unsigned short> indices);
	void TestMeshInit();
	void Draw();
	unsigned long Release();
	ID3D12DescriptorHeap* GetBasicDescHeap() { return _basicDescHeap; };

	void AddRef(){ };
	~Mesh() { Release(); };
};

