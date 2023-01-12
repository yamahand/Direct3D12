struct VSOutput {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD;
};

struct PSOutput {
	float4 color : SV_TARGET0;
};

SamplerState colorSmp : register(s0);
Texture2D colorMap : register(t0);

PSOutput main(VSOutput input)
{
	PSOutput output = (PSOutput)0;
	output.color = colorMap.Sample(colorSmp, input.texCoord);

	return output;
}