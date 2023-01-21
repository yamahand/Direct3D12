struct VSOutput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
	float4 worldPos : WORLD_POS;
	float3x3 tangentBasis : TANGENT_BASIS;
	float3x3 invTangentBasis : INV_TANGENT_BASIS;
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
Texture2D normalMap : register(t1);

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;

#if 0
	// 接戦空間上でライティングする時

	// ライトベクトル
	float3 l = normalize(lightPosition - input.worldPos.xyz);
	l = mul(input.tangentBasis, l);

	// 視線ベクトル
	float3 v = normalize(cameraPosition - input.worldPos.xyz);
	v = mul(input.tangentBasis, v);

	float3 n = normalMap.Sample(colorSmp, input.texCoord).xyz * 2.0 - 1.0;
#else
	// ワールド空間上でライティングする時

	// ライトベクトル
	float3 l = normalize(lightPosition - input.worldPos.xyz);

	// 視線ベクトル
	float3 v = normalize(cameraPosition - input.worldPos.xyz);

	float3 n = normalMap.Sample(colorSmp, input.texCoord).xyz * 2.0 - 1.0;
	n = mul(input.invTangentBasis, n);

#endif

	float3 r = normalize(-reflect(v,n));

	float4 c = colorMap.Sample(colorSmp, input.texCoord);
	float3 d = diffuse * lightColor * saturate(dot(l, n));
	float3 s = specular * lightColor * pow(saturate(dot(l, r)), shininess);

	output.color = float4(c.rgb * (d + s), c.a * alpha);

	return output;
}