//Adapted from: 
//https://learnopengl.com/code_viewer_gh.php?code=src/8.guest/2022/7.area_lights/2.multiple_area_lights/7.multi_area_light.fs

struct VSOutput
{
    [[vk::location(0)]] float4 worldPos : POSITION0;
    [[vk::location(1)]] float3 normal : NORMAL0;
    [[vk::location(2)]] float inst : POSITION1;
    [[vk::location(3)]] float2 tex : TEXCOORD0;
};

struct Camera
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float4 camPos;
};

struct LightVertex
{
    float4 verts[4];
};

Texture2D texLtc[2] : register(t0, space1);
SamplerState sampLtc[2] : register(s0, space1);
ConstantBuffer<Camera> cam : register(b0, space0);
ConstantBuffer<LightVertex> lit : register(b0, space2);

float3 integrateEdge(float3 v1, float3 v2)
{
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206 * y) * y;
    float b = 3.4175940 + (4.1616724 + y) * y;
    float v = a / b;
    float theta_sintheta = (x > 0.0) ? v : 0.5 * rsqrt(max(1.0 - x * x, 1e-7)) - v;
    return cross(v1, v2) * theta_sintheta;
}

float3 ltcEval(float3 n, float3 v, float3 p,float3x3 mInv,float4 pts[4])
{
    if (p.z > 0.0)
        return float3(0, 0, 0);
    
    float LSCALE = 1.0 / 64;
    float LBIAS = 0.5 / 64;
    
    float3 t = normalize(v - n * dot(n, v));
    float3 b = cross(n, t);
    float3x3 iv = float3x3(t, b, n);
    float3x3 mInvX = mul(mInv, iv);
    
    float3 lp[4];
    int i;
    for (i = 0; i < 4; i++)
        lp[i] = normalize(mul(mInvX, (pts[i].xyz - p)));
    
    float3 vInt = float3(0, 0, 0);
    for (i = 0; i < 4; i++)
    {
        vInt += integrateEdge(lp[i], lp[(i + 1) % 4]);
    }
    
    float len = length(vInt);
    float z = -vInt.z / len;

    float2 uv = float2(z * 0.5f + 0.5f, len); 
    uv = uv * LSCALE + LBIAS;
    float scale = texLtc[1].Sample(sampLtc[1], uv).w;
    float sum = len;
    return float3(sum, sum, sum);;
}

float3x3 identityMat()
{
    return float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
}

float3 gammaCorrection(float3 color)
{
    return pow(color, float3(1 / 2.2, 1 / 2.2, 1 / 2.2));
}
float4 main(VSOutput vsOut) : SV_Target
{
    const float ROUGHNESS = 0.5;
    
    if (vsOut.inst > 0.5)
    {
        return float4(1, 1, 1, 1);
    }
    else
    {
        float3x3 mInvD = identityMat();
        float3 p = vsOut.worldPos.xyz;
        float3 v = normalize(cam.camPos.xyz - p);
        float3 n = normalize(vsOut.normal);
        
        float rough = 0.5;
        float nv = 1.0 - dot(n, v);
        float2 uv = float2(rough, sqrt(nv));
        
        float3 diffuse = ltcEval(n, v, p, mInvD, lit.verts);
        return float4(gammaCorrection(diffuse * 2.0), 1);
        
    }
    return float4(1, 0, 0, 0);
}