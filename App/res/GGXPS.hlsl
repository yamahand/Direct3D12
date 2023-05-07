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

// SchlickÇ…ÇÊÇÈÉtÉåÉlÉãçÄÇÃãﬂéóéÆ
float3 SchlickFresnel(float3 specular, float VH)
{
    return specular + (1.0f - specular) * pow((1.0f - VH), 5.0f);
}

// GGXï™ïzä÷êî
float D_GGX(float m2, float nh)
{
	float f = (nh * m2 - nh) * nh + 1;
	return m2 / (F_PI * f * f);
}

// Height Correlated SmithÇ…ÇÊÇÈäÙâΩå∏êäçÄ
float G2_Smith(float nl, float nv, float m2)
{
	float nl2 = nl * nl;
	float nv2 = nv * nv;

	float lambda_v = (-1.0f + sqrt(m2 * (1.0f - nl2) / nl2 + 1.0f)) * 0.5f;
	float lambda_l = (-1.0f + sqrt(m2 * (1.0f - nv2) / nv2 + 1.0f)) * 0.5f;
	return 1.0f / (1.0f + lambda_v + lambda_l);
}


PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;

	float3 n = normalize(input.normal);
	float3 l = normalize(lightPosition - input.worldPos.xyz);
	float3 v = normalize(cameraPosition -input.worldPos.xyz);
	float3 h = normalize(v + l);

	float nv = saturate(dot(n, v));
	float nh = saturate(dot(n, h));
	float nl = saturate(dot(n, l));
	float vh = saturate(dot(v, h));

	float4 color = colorMap.Sample(colorSmp, input.texCoord);
	float3 kd = baseColor * (1.0f - metallic);
	float3 diffuse = kd * (1.0 / F_PI);
	
    float3 ks = baseColor * metallic;
    float a = roughness * roughness;
	float m2 = a * a;
    float d = D_GGX(m2, nh);
    float g2 = G2_Smith(nl, nv, m2);
    float3 fr = SchlickFresnel(ks, nl);
	
    float3 specular = (d * g2 * fr) / (4.0f * nv * nl);

    output.color = float4(color.rgb * (diffuse + specular) * nl, color.a * alpha);
	return output;
}

