struct VSOutput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float3 normal : NORMAL;
	float4 worldPos : WORLD_POS;
};

struct PSOutput {
	float4 color : SV_TARGET0;
};

cbuffer LightBuffer : register(b1)
{
	float3 lightPosition : packoffset(c0);
	float3 lightColor : packoffset(c1);
}

cbuffer MaterialBuffer : register(b2) {
	float3 diffuse : packoffset(c0);
	float alpha : packoffset(c0.w);
}

SamplerState colorSmp : register(s0);
Texture2D colorMap : register(t0);

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;

	float3 n = normalize(input.normal);
	float3 l = normalize(lightPosition - input.worldPos.xyz);

	float4 c = colorMap.Sample(colorSmp, input.texCoord);
	float3 d = lightColor * diffuse * saturate(dot(l, n));

	output.color = float4(c.rgb * d, c.a * alpha);

	return output;
}