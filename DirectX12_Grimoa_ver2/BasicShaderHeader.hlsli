struct Output {
	float4 svpos:SV_POSITION;//�V�X�e���p���_���W
	float4 pos:POSITION;//�V�X�e���p���_���W
	float4 normal:NORMAL0;//�@���x�N�g��
	float4 vnormal:NORMAL1;//�@���x�N�g��
	float2 uv:TEXCOORD;//UV�l
	float3 ray:VECTOR;//�x�N�g��
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

