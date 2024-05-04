struct Camera {
	float4x4 mvp;
};
struct OccludeePos{
	float4 pos;
};

static const int TOTAL_LEVELS = 12;
ConstantBuffer<Camera> camera : register(b0);
RWTexture2D<float> depths[TOTAL_LEVELS] : register(u0, space1);
RWStructuredBuffer<OccludeePos> culledPos : register(u0, space2);
RWStructuredBuffer<int> culledPosCounter : register(u1, space2);
RWStructuredBuffer<OccludeePos> orgPos : register(u0, space3);

void appendBuffer(float4 pos){
	OccludeePos occludeePos;
	occludeePos.pos = pos;
	int src=0;
	InterlockedAdd(culledPosCounter[0], 1,src);
	culledPos[src] = occludeePos;
}

[numthreads(1, 1, 1)]
void main(uint3 invId:SV_DispatchThreadID){

	// Occlusion Culling
	float3 pos = orgPos[invId.x].pos.xyz;
	float3 aabbPts[8];
	aabbPts[0] = pos + float3(-1, -1, -1);
	aabbPts[1] = pos + float3(1, -1, -1);
	aabbPts[2] = pos + float3(-1, 1, -1);
	aabbPts[3] = pos + float3(1, 1, -1);
	aabbPts[4] = pos + float3(-1, -1, 1);
	aabbPts[5] = pos + float3(1, -1, 1);
	aabbPts[6] = pos + float3(-1, 1, 1);
	aabbPts[7] = pos + float3(1, 1, 1);

	float4 clipPos[8];
	float2 uv[8];
	float maxz = -1e9;
	float minUvX = 1e9, maxUvX = -1e9, minUvY = 1e9, maxUvY = -1e9;
	bool frustumCulled = true;
	bool atBorder = false;

	for(int i=0;i<8;i++){
		clipPos[i] = mul(camera.mvp, float4(aabbPts[i], 1));
		clipPos[i] /= clipPos[i].w;
		uv[i] = clipPos[i].xy * 0.5 + 0.5;
		maxz = max(maxz, clipPos[i].z);
		minUvX = min(minUvX, uv[i].x);
		maxUvX = max(maxUvX, uv[i].x);
		minUvY = min(minUvY, uv[i].y);
		maxUvY = max(maxUvY, uv[i].y);
		if(uv[i].x >= 0.0f && uv[i].x <= 1.0f && uv[i].y >= 0.0f && uv[i].y <= 1.0f){
			if(clipPos[i].z >= 0.0f){
				frustumCulled = false;
			}
		}else{
			atBorder=true;
		}
	}
	// Frustum Culling
	if (frustumCulled) {
		return;
	}
	if(atBorder){
		appendBuffer(float4(orgPos[invId.x].pos.xyz,1));
		return;
	}

	float mxUv = max(maxUvX - minUvX, maxUvY - minUvY);
	// Find depth level
	int level = 0;
	int mxUvInt = int(mxUv * 2048);
	level = min(TOTAL_LEVELS - 1, (int)log2(mxUvInt)+1);
	for(;level<TOTAL_LEVELS;level++){
		int Xmin = int(minUvX*2048.0f)/(int(1)<<level);
		int Xmax = int(maxUvX*2048.0f)/(int(1)<<level);
		int Ymin = int(minUvY*2048.0f)/(int(1)<<level);
		int Ymax = int(maxUvY*2048.0f)/(int(1)<<level);
		if(Xmin == Xmax && Ymin == Ymax) break;
	}

	// Find depth in Hierarchical Z-Buffer
	int2 hUV = int2(uv[0]*2048.0f)/(int(1)<<level);
	float depth = depths[level][hUV];
	if(maxz > depth) return;

	// Append to culledPos
	appendBuffer(float4(orgPos[invId.x].pos.xyz,1));
	return;
}


