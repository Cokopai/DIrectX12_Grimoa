#include "BasicShaderHeader.hlsli"

cbuffer cbuff0 : register(b0) {
	matrix world;//ワールド変換行列
	matrix view;
	matrix proj;//ビュープロジェクション行列
	float3 eye;
};

Output BasicVS(
	float4 pos : POSITION ,
	float4 normal : NORMAL,
	float2 uv : TEXCOORD,
	min16uint2 boneno : BONE_NO,
	min16uint  weight : WEIGHT)
{
	Output output;//ピクセルシェーダへ渡す値
	pos = mul(world, pos);
	output.svpos = mul(mul(proj, view), pos);//シェーダでは列優先なので注意
	output.pos = mul(view, pos);
	normal.w = 0;//ここ重要(平行移動成分を無効にする)
	output.normal = mul(world, normal);//法線にもワールド変換を行う
	output.vnormal = mul(view, output.normal);
	output.uv = uv;
	output.ray = normalize(pos.xyz - mul(view, eye));//視線ベクトル


	return output;
}