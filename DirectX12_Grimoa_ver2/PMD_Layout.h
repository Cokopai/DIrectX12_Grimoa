#pragma once
#include <DirectXMath.h>
#include <string>

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

#pragma pack(1)
struct PMDMaterial {
	DirectX::XMFLOAT3 diffuse;
	float alpha;
	float specularity;
	DirectX::XMFLOAT3 specular;
	DirectX::XMFLOAT3 ambient;
	unsigned char toonIdx;
	unsigned char edgeFg;
	//ここにパディング
	unsigned int indecesNum;

	char texFilePath[20];
};
#pragma pack()

struct MaterialForHlsl {
	DirectX::XMFLOAT3 diffuse;
	float alpha;//ディフューズα
	DirectX::XMFLOAT3 specular;
	float specularity;
	DirectX::XMFLOAT3 ambient;
};

struct AdditionalMaterial {
	std::string texPath;
	int toonIdx;
	bool edgeFg;
};

struct Material {
	unsigned int indicesNum;//インデックス数
	MaterialForHlsl material;
	AdditionalMaterial additional;
};

//シェーダ側に渡すための基本的な環境データ
struct SceneData {
	DirectX::XMFLOAT4X4 world;//ワールド行列
	DirectX::XMFLOAT4X4 view;//ビュープロジェクション行列
	DirectX::XMFLOAT4X4 proj;//
	DirectX::XMFLOAT3 eye;//視点座標
};


class PMD_Layout
{

};

