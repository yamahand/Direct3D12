struct VSInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 tangent : TANGENT;
};

struct VSOutput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float4 worldPos : WORLD_POS;
	float3x3 tangentBasis : TANGENT_BASIS;
	float3x3 invTangentBasis : INV_TANGENT_BASIS;
};

cbuffer Transform : register(b0) {
	float4x4 world : packoffset(c0);
	float4x4 view  : packoffset(c4);
	float4x4 proj  : packoffset(c8);
};

VSOutput main(VSInput input )
{
	VSOutput output = (VSOutput)0;

	float4 localPos = float4(input.position, 1.0);
	float4 worldPos = mul(world, localPos);
	float4 viewPos = mul(view, worldPos);
	float4 projPos = mul(proj, viewPos);

	output.position = projPos;
	output.texCoord = input.texCoord;
	output.worldPos = worldPos;

	// ‹K’èƒxƒNƒgƒ‹
	float3 n = normalize(mul((float3x3)world, input.normal));
	float3 t = normalize(mul((float3x3)world, input.tangent));
	float3 b = normalize(cross(n, t));

	output.tangentBasis = float3x3(t, b, n);
	output.invTangentBasis = transpose(float3x3(t,b,n));

	return output;
}