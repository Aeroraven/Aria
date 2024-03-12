struct PSOutput
{
	float4 color : SV_TARGET0;
};

struct VSOutput
{
	[[vk::location(0)]] float4 color : COLOR0;
};

PSOutput main(VSOutput vsOut)
{
	PSOutput psOut;
	psOut.color = vsOut.color;
	return psOut;
}