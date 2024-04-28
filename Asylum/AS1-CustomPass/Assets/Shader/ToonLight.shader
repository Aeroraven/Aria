Shader "Unlit/ToonLight"
{
    Properties
    {
        [MainTexture] _BaseColorMap("BaseColorMap", 2D) = "red" {}
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            // make fog work
            #pragma multi_compile_fog

            #include "UnityCG.cginc"
            #include "UnityLightingCommon.cginc"

            struct appdata
            {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float3 normal : NORMAL;
                float4 vertex : SV_POSITION;
            };

            sampler2D _BaseColorMap;
            float4 _BaseColorMap_ST;

            float3 _SunDirection;
            float4 _ToonBaseColor;
            float4 _ToonShadowColor ;
            float4 _ToonTransitionColor ;
            float _ToonColorRamp1Start;
            float _ToonColorRamp1End ;
            float _ToonColorRamp2Start ;
            float _ToonColorRamp2End ;
            v2f vert (appdata v)
            {
                v2f o; 
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.normal = UnityObjectToWorldNormal(v.normal);
                o.uv = TRANSFORM_TEX(v.uv, _BaseColorMap);
                return o;
            }

            fixed4 frag (v2f i) : SV_Target
            {
                // sample the texture
                float ndotl = max(0,dot(-i.normal, _SunDirection.xyz));
                float4 baseColor = tex2D(_BaseColorMap, i.uv);

                float ramp1Weight = smoothstep(_ToonColorRamp1Start, _ToonColorRamp1End, ndotl);
                float ramp2Weight = smoothstep(_ToonColorRamp2Start, _ToonColorRamp2End, ndotl);
                float ramp0Weight = 1 - ramp1Weight - ramp2Weight;

                float4 avgWeight = ramp0Weight * _ToonShadowColor + ramp1Weight * _ToonBaseColor + ramp2Weight * _ToonTransitionColor;


                fixed4 col = baseColor*avgWeight;
                return col;
            }
            ENDCG
        }
    }
}
