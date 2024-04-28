Shader "Hidden/Shader/ToonOutline"
{
    //Outline Shader Referred From: https://roystan.net/articles/outline-shader/
    Properties
    {
        _MainTex("Main Texture", 2DArray) = "grey" {}
        _Scale("Scale",float) = 0 
        _DepthTreshold("Depth Treshold",float) = 0.5
        _NormalThreshold("Normal Treshold",float) = 0.5
        _DepthNormalThreshold("Depth Normal Threshold",float) = 0.5
        _DepthNormalThresholdScale("Depth Normal Threshold Scale",float) = 7.0
        _ColorMultiplier("Color Multiplier",float) = 0.5
    }

    HLSLINCLUDE

    #pragma target 4.5
    #pragma only_renderers d3d11 playstation xboxone xboxseries vulkan metal switch

    #include "Packages/com.unity.render-pipelines.core/ShaderLibrary/Common.hlsl"
    #include "Packages/com.unity.render-pipelines.core/ShaderLibrary/Color.hlsl"
    #include "Packages/com.unity.render-pipelines.high-definition/Runtime/ShaderLibrary/ShaderVariables.hlsl"
    #include "Packages/com.unity.render-pipelines.high-definition/Runtime/PostProcessing/Shaders/FXAA.hlsl"
    #include "Packages/com.unity.render-pipelines.high-definition/Runtime/PostProcessing/Shaders/RTUpscale.hlsl"

    struct Attributes
    {
        uint vertexID : SV_VertexID;
        UNITY_VERTEX_INPUT_INSTANCE_ID
    };

    struct Varyings
    {
        float4 positionCS : SV_POSITION;
        float2 texcoord   : TEXCOORD0;
        UNITY_VERTEX_OUTPUT_STEREO
    };

    Varyings Vert(Attributes input)
    {
        Varyings output;
        UNITY_SETUP_INSTANCE_ID(input);
        UNITY_INITIALIZE_VERTEX_OUTPUT_STEREO(output);
        output.positionCS = GetFullScreenTriangleVertexPosition(input.vertexID);
        output.texcoord = GetFullScreenTriangleTexCoord(input.vertexID);
        return output;
    }
    float _Scale;
    float _DepthTreshold;
    float _NormalThreshold;
    float4x4 _InvProj;
    float _DepthNormalThreshold;
    float _DepthNormalThresholdScale;
    float _ColorMultiplier;

    TEXTURE2D_X(_MainTex);
    TEXTURE2D(_NormalMap);
    TEXTURE2D(_DepthMap);
    float2 _MainTex_TexelSize;
    

    float GetDepth(float2 uv){
        return SAMPLE_TEXTURE2D(_DepthMap, s_linear_clamp_sampler, uv).r;
    }
    
    float3 GetNormal(float2 uv){
        return SAMPLE_TEXTURE2D(_NormalMap, s_linear_clamp_sampler, uv).rgb;
    }
    float4 CustomPostProcess(Varyings input) : SV_Target
    {
        UNITY_SETUP_STEREO_EYE_INDEX_POST_VERTEX(input);

        float2 uv =  ClampAndScaleUVForBilinearPostProcessTexture(input.texcoord.xy);
        float3 color = SAMPLE_TEXTURE2D_X(_MainTex, s_linear_clamp_sampler,uv).xyz;
        float3 normal = SAMPLE_TEXTURE2D(_NormalMap, s_linear_clamp_sampler, uv).xyz;
        float depth = SAMPLE_TEXTURE2D(_DepthMap, s_linear_clamp_sampler, uv).r;

        // Depth/Normal Thresholding

        float2 tS = _MainTex_TexelSize;
        float hsF = floor(_Scale*0.5);
        float hsC = ceil(_Scale*0.5);

        float2 uvLB = uv - tS * hsF;
        float2 uvRT = uv + tS * hsC;
        float2 uvLT = uv + float2(-tS.x*hsF,tS.y*hsC);
        float2 uvRB = uv + float2(tS.x*hsC,-tS.y*hsF);

        float dLB = GetDepth(uvLB);
        float dRT = GetDepth(uvRT);
        float dLT = GetDepth(uvLT);
        float dRB = GetDepth(uvRB);

        float3 nLB = GetNormal(uvLB);
        float3 nRT = GetNormal(uvRT);
        float3 nLT = GetNormal(uvLT);
		float3 nRB = GetNormal(uvRB);

        // Viewspace Ray & Threshold
        float4 ndc = float4(uv*2.0-1.0,1.0,1.0);
        float3 vsRay = normalize(mul(_InvProj, float4(ndc)).xyz);
        float ndotl = 1-dot(vsRay,normal);
        float dnThTemp = saturate((ndotl-_DepthNormalThreshold)/(1-_DepthNormalThreshold));
        float depthNormalThreshold = 1 + _DepthNormalThresholdScale * dnThTemp;

        
        float d1 = dRT-dLB;
        float d2 = dRB-dLT;
        float3 n1 = nRT-nLB;
        float3 n2 = nRB-nLT;

        float depthThreshold = depthNormalThreshold * dLB * _DepthTreshold;

        float resultd = sqrt(pow(d1,2)+pow(d2,2));
        float resultn = sqrt(dot(n1,n1)+dot(n2,n2));

        resultd = max(0,sign(resultd - depthThreshold));
        resultn = max(0,sign(resultn - _NormalThreshold));

        float result = max(resultn,resultd);
        float3 edgeColor = color * _ColorMultiplier;

        //Blending 
        float3 retv = lerp(0, edgeColor, result); 

        return float4(retv, 1);

    }

    ENDHLSL

    SubShader
    {
        Tags{ "RenderPipeline" = "HDRenderPipeline" }
        Pass
        {
            Name "ToonOutline"

            ZWrite Off
            ZTest Always
            Blend Off
            Cull Off

            HLSLPROGRAM
                #pragma fragment CustomPostProcess
                #pragma vertex Vert
            ENDHLSL
        }
    }
    Fallback Off
}
