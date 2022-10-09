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
	//�����Ƀp�f�B���O
	unsigned int indecesNum;

	char texFilePath[20];
};
#pragma pack()

struct MaterialForHlsl {
	DirectX::XMFLOAT3 diffuse;
	float alpha;//�f�B�t���[�Y��
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
	unsigned int indicesNum;//�C���f�b�N�X��
	MaterialForHlsl material;
	AdditionalMaterial additional;
};

//�V�F�[�_���ɓn�����߂̊�{�I�Ȋ��f�[�^
struct SceneData {
	DirectX::XMFLOAT4X4 world;//���[���h�s��
	DirectX::XMFLOAT4X4 view;//�r���[�v���W�F�N�V�����s��
	DirectX::XMFLOAT4X4 proj;//
	DirectX::XMFLOAT3 eye;//���_���W
};


class PMD_Layout
{

};

