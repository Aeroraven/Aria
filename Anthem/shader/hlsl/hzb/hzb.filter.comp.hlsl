struct Camera {
	float4x4 mvp;
};
struct OccludeePos{
	float4 pos;
};

static const int TOTAL_LEVELS = 12;
ConstantBuffer<Camera> camera : register(b0);
RWTexture2D<float> depths[TOTAL_LEVELS] : register(u0, space1);
AppendStructuredBuffer<OccludeePos> culledPos : register(u0, space2);
RWStructuredBuffer<int> culledPosCounter : register(u1, space2);
RWStructuredBuffer<OccludeePos> orgPos : register(u0, space3);

[numthreads(1, 1, 1)]
void main(uint3 invId:SV_DispatchThreadID){
	if(invId.x==0){
		culledPosCounter[0] = 0;
	}
	GroupMemoryBarrier();
	//Test
	// Append to culledPos
	OccludeePos occludeePos;
	occludeePos.pos = float4(orgPos[invId.x].pos.xyz, 1);
	culledPos.Append(occludeePos);
	return;

	// Occlusion Culling
	float3 pos = orgPos[invId.x].pos.xyz;
	float3 aabbPts[4];
	aabbPts[0] = pos + float3(-1, -1, 0);
	aabbPts[1] = pos + float3(1, -1, 0);
	aabbPts[2] = pos + float3(-1, 1, 0);
	aabbPts[3] = pos + float3(1, 1, 0);

	float4 clipPos[4];
	float2 uv[4];
	float maxz = -1e9;
	bool frustumCulled = true;
	for(int i=0;i<4;i++){
		clipPos[i] = mul(camera.mvp, float4(aabbPts[i], 1));
		clipPos[i] /= clipPos[i].w;
		uv[i] = clipPos[i].xy * 0.5 + 0.5;
		maxz = max(maxz, clipPos[i].z);
		if(uv[i].x >= 0 && uv[i].x <= 1 && uv[i].y >= 0 && uv[i].y <= 1){
			frustumCulled = false;
		}
	}

	// Frustum Culling
	if (frustumCulled) return;

	float uvDx = 0, uvDy = 0, mxUv = 0;
	uvDx = abs(uv[0].x - uv[1].x);
	uvDy = abs(uv[0].y - uv[2].y);
	mxUv = max(uvDx, uvDy);

	// Find depth level
	int level = 0;
	int2 uvInt[4];
	int depTexH, depTexW;
	depths[0].GetDimensions(depTexW, depTexH);
	for(int i=0;i<4;i++){
		uvInt[i] = int2(uv[i] * float2(depTexW, depTexH));
	}
	int mxUvInt = max(abs(uvInt[0].x - uvInt[1].x), abs(uvInt[0].y - uvInt[2].y));
	level = min(TOTAL_LEVELS - 1, (int)log2(mxUvInt)+1);

	// Find depth in Hierarchical Z-Buffer
	int2 hUV = uvInt[0]/(int(1)<<level);
	float depth = depths[level][hUV];
	if(maxz < depth) return;

	// Append to culledPos
	OccludeePos occludeePosX;
	occludeePosX.pos = float4(pos, 1);
	culledPos.Append(occludeePosX);
}


