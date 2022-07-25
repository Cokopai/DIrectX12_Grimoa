#pragma once
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 _pos;
	XMFLOAT2 _uv;

};

struct TexRGBA {
	unsigned char R, G, B, A;
};
