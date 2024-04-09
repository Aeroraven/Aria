
static const float W_DOMAIN_WARP = 1;
static const int USED_WAVES = 32;
static const float WAVE_HEIGHT = 1;

struct Constants
{
    float4x4 proj;
    float4x4 view;
    float4x4 model;
    float4 timing;
    float4 freqAmpl;
    float4 sunDir;
    float4 camPos;
    float4 warpWaves;
    float4 wave[32];
};


float3 calcWave(float2 p,float t,float4 waves[32], float baseAmpl, float baseFreq, float fbmAmpl, float fbmFreq,float warpStr,int useWaves)
{
    //F(p,t,i)=A(i)exp(sin(F(i)D(i)*p + P(i)*t))
    //dF/dx = F*cos(FD*p+Pt)*F*Dx
    
    float Ca = baseAmpl, Cf = baseFreq;
    float ans = 0;
    float2 dp = p;
    float accAmpl = 0;
    float2 accDerv = float2(0, 0);
    for (int i = 0; i < useWaves; i++)
    {
        float2 dir = normalize(waves[i].xy);
        float wave = Ca * exp(WAVE_HEIGHT*sin(Cf * dot(dir, dp) + waves[i].z * t));
        float2 derv = WAVE_HEIGHT*wave * cos(Cf * dot(dir, dp) + waves[i].z * t) * Cf * dir;
        ans += Ca * sin(Cf * dot(dir, dp) + waves[i].z * t);
        dp += -derv * Ca * warpStr;
        accAmpl += Ca;
        accDerv += derv;
        Ca *= fbmAmpl;
        Cf *= fbmFreq;
    }
    float3 ret = float3(accDerv.x, ans, accDerv.y) / accAmpl * baseAmpl;
    return ret;
}
