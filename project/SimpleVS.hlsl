struct VSInput {
	float3 position : POSITION;
	float4 color : COLOR;
};

struct VSOutput {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

cbuffer Transform : register(b0) {
	float4x4 world : packoffset(c0);
	float4x4 view  : packoffset(c4);
	float4x4 proj  : packoffset(c8);
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	float4 localPos = float4(input.position, 1.0);
	float4 worldPos = mul(world, localPos);
	float4 viewPos = mul(view, worldPos);
	float4 projPos = mul(proj, viewPos);

	output.position = projPos;
	output.color = input.color;

	return output;
}