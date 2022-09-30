#pragma once

#include "Mesh.h"
#include "PMD_Layout.h"

using Microsoft::WRL::ComPtr;

class ModelData
{
public:
	unsigned int m_vertNum;
	unsigned int m_idxNum;

	ComPtr<Mesh> m_Mesh;
	PMDHeader m_pmdHeader;

	void CreateModel(const char* fileName);

	void AddRef() {};
	unsigned long Release();
};

class ModelLoader {
public:
};

