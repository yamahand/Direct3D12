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
	float3 cameraPosition : packoffset(c2);
}

cbuffer MaterialBuffer : register(b2) {
	float3 diffuse : packoffset(c0);
	float alpha : packoffset(c0.w);
	float3 specular : packoffset(c1);
	float shininess : packoffset(c1.w);
}

SamplerState colorSmp : register(s0);
Texture2D colorMap : register(t0);

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;

	float3 n = normalize(input.normal);
	float3 l = normalize(lightPosition - input.worldPos.xyz);
	float3 v = normalize(cameraPosition - input.worldPos.xyz);

	float3 r = normalize(-reflect(v, n));

	float4 c = colorMap.Sample(colorSmp, input.texCoord);
	float3 d = lightColor * diffuse * saturate(dot(l, n));
	float3 s = lightColor * specular * pow(saturate(dot(l,r)), shininess);

	output.color = float4(c.rgb * (d + s), c.a * alpha);

	return output;
}