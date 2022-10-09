struct Output {
	float4 svpos:SV_POSITION;//システム用頂点座標
	float4 pos:POSITION;//システム用頂点座標
	float4 normal:NORMAL0;//法線ベクトル
	float4 vnormal:NORMAL1;//法線ベクトル
	float2 uv:TEXCOORD;//UV値
	float3 ray:VECTOR;//ベクトル
};

cbuffer Material : register(b1) {
	float4 diffuse;
	float4 specular;
	float3 ambient;
}

Texture2D<float4> tex: register(t0);
SamplerState smp : register(s0);
/*
struct Input {
	float4 svpos:SV_POSITION,
	float2 uv : TEXCOORD,
};
*/

