struct VSInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texCoord : TEXCOORD;
	float3 tangent : TANGENT;
};

struct VSOutput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float4 worldPos : WORLD_POS;
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
	output.normal = normalize(mul((float3x3)world, input.normal));

	return output;
}