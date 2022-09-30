#include "ModelData.h"
#include "DirectX12_Graphics.h"

void ModelData::CreateModel(const char* fileName)
{
	FILE* fp;
	fopen_s(&fp, "Model/鏡音リン.pmd", "rb");

	char signature[3] = {};
	fread(signature, sizeof(signature), 1, fp);
	fread(&m_pmdHeader, sizeof(m_pmdHeader), 1, fp);

	unsigned int vertexNum;//頂点数
	fread(&vertexNum, sizeof(vertexNum), 1, fp);
	m_vertNum = vertexNum;

	std::vector<unsigned char> vertices(vertexNum * pmd_vertex_size);
	fread(vertices.data(), vertices.size(), 1, fp);

	unsigned int indicesNum;//インデックス数
	fread(&indicesNum, sizeof(indicesNum), 1, fp);
	m_idxNum = indicesNum;

	std::vector<unsigned short> indeices(indicesNum);
	fread(indeices.data(), indeices.size() * sizeof(indeices[0]),1, fp);

	//std::vector<unsigned char> pmd_vertices;
	//pmd_vertices.resize(vertexNum);
	fclose(fp);

	m_Mesh = new Mesh(DirectX12_Graphics::GetInstance()->GetDXDevice(),vertices,indeices);
	//modelData.m_Mesh->Create(DirectX12_Graphics::GetInstance()->GetDXDevice(), pmd_vertices);

	return;
}

unsigned long ModelData::Release()
{
	return 0;
}
