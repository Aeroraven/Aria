struct VSInput
{
    [[vk::location(0)]] float4 Position : POSITION0;
	[[vk::location(1)]] float4 Rotation : POSITION1;
    [[vk::location(2)]] float4 Translation : POSITION2;
	[[vk::location(3)]] float4 Color : COLOR0;
};

struct Uniform
{
	float4x4 Proj;
	float4x4 View;
	float4x4 Model;
};

cbuffer ubo : register(b0, space0)
{ 
	Uniform ubo;
}

struct VSOutput
{
	float4 Position : SV_Position;
	[[vk::location(0)]] float4 Color : COLOR0;
};

float4x4 RotationMatrix(float3 axis,float angle)
{
	float4 q = float4(axis.xyz * sin(angle / 2), cos(angle / 2));
	float4x4 m = float4x4(
		float4(1 - 2 * q.y * q.y - 2 * q.z * q.z, 2 * q.x * q.y - 2 * q.z * q.w, 2 * q.x * q.z + 2 * q.y * q.w, 0),
		float4(2 * q.x * q.y + 2 * q.z * q.w, 1 - 2 * q.x * q.x - 2 * q.z * q.z, 2 * q.y * q.z - 2 * q.x * q.w, 0),
		float4(2 * q.x * q.z - 2 * q.y * q.w, 2 * q.y * q.z + 2 * q.x * q.w, 1 - 2 * q.x * q.x - 2 * q.y * q.y, 0),
		float4(0, 0, 0, 1)
	);
	return m;

}

VSOutput main(VSInput sIn)
{
	VSOutput VSOut;
	float4x4 instTransform = RotationMatrix(sIn.Rotation.xyz, sIn.Rotation.w);
    float4 localVert =mul(instTransform, float4(sIn.Position.xyz, 1.0)); //
    localVert += float4(sIn.Translation.xyz, 0.0);
	
    VSOut.Position = mul(ubo.Proj, mul(ubo.View, mul(ubo.Model, localVert)));
    VSOut.Color = sIn.Color;
	return VSOut;
}