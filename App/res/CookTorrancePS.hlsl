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
	float3 baseColor  : packoffset(c0);
	float alpha : packoffset(c0.w);
	float roughness : packoffset(c1.x);
	float metallic : packoffset(c1.y);
}

SamplerState colorSmp : register(s0);
Texture2D colorMap : register(t0);

static const float F_PI = 3.141596535f;

// Schlickによるフレネル項の近似式
float3 SchlickFresnel(float3 specular, float VH)
{
    return specular + (1.0f - specular) * pow((1.0f - VH), 5.0f);
}

// Beckmann分布関数
float D_Beckmann(float m, float NH)
{
    float c2 = NH * NH;
    float c4 = c2 * c2;
    float m2 = m * m;

    return (1.0f / (m2 * c4)) * exp((-1.0f / m2) * (1.0f - c2) / c2);
}

// v-cavityによるシャドウイングーマスキング関数
float G2_Vcavity(float NH, float NV, float NL, float VH)
{
    return min(1.0f, min(2.0f * NH * NV / VH, 2.0f * NH * NL / VH));
}


PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;

	float3 n = normalize(input.normal);
	float3 l = normalize(lightPosition - input.worldPos.xyz);
	float3 v = normalize(cameraPosition -input.worldPos.xyz);
	float3 r = normalize(-v + 2.0f * dot(n, v) * n);
	float3 h = normalize(v + l);

	float nh = saturate(dot(n, h));
	float nv = saturate(dot(n, v));
	float nl = saturate(dot(n, l));
	float vh = saturate(dot(v, h));

	float4 color = colorMap.Sample(colorSmp, input.texCoord);
	float3 kd = baseColor * (1.0f - metallic);
	float3 diffuse = kd * (1.0 / F_PI);
	
    float3 ks = baseColor * metallic;
    float a = roughness * roughness;
    float d = D_Beckmann(a, nh);
    float g2 = G2_Vcavity(nh, nv, nl, vh);
    float3 fr = SchlickFresnel(ks, nl);
	
    float3 specular = (d * g2 * fr) / (4.0f * nv * nl);
    output.color = float4(lightColor * color.rgb * (diffuse + specular) * nl, color.a * alpha);
	return output;
}

