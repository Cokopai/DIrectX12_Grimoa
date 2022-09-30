#pragma once
#include <DirectXMath.h>

struct PMDHeader {
	float version;
	char model_name[20];
	char comment[256];
};

struct PMDVertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	unsigned short boneNo[2];
	unsigned char boneWeight;
	unsigned char edgeFlg;
};
constexpr size_t pmd_vertex_size = 38;

class PMD_Layout
{

};

