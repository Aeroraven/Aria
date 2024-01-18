#include "../include/core/renderer/AnthemSimpleToyRenderer.h"
#include "../include/core/drawing/buffer/AnthemShaderStorageBuffer.h"
#include "../include/core/drawing/buffer/AnthemUniformBuffer.h"
#include "../include/core/math/AnthemMathAbbrs.h"
#include "../include/core/math/AnthemLinAlg.h"
#include "../include/components/camera/AnthemCamera.h"


#include <chrono>

#include <thread>
#include <mutex>

using namespace Anthem::Core;
using namespace Anthem::Core::Math;
using namespace Anthem::Core::Math::Abbr;
using namespace Anthem::Components::Camera;

using namespace std::chrono;


struct ExpParams {

	
	// Exp params
	static AtVecf4 centerTranslation;
	static AtVecf4 rotationAxis;
	static int moveFocal;

	// Parallel configurations
	static constexpr int sampleCounts = 524288/4; //4194304
	static constexpr int parallelsXGpu = 16384; // For GPU kernels
	static constexpr int parallelsXCpu = 16; // For CPU threads


	// Visualization
	static constexpr float lineWidth = 2;
	static constexpr float fontSize = 64;
	static constexpr float axisOriginCrd = -1.0;
	static constexpr float axisExtendCrd = 1.0;

	static constexpr float axisOriginCrdZ = -1.0;
	static constexpr float axisExtendCrdZ = 1.0;

	static constexpr float visRot = 0.5;
};

struct ExpCore {
	AnthemSimpleToyRenderer renderer;
	AnthemConfig cfg;
	AnthemCamera camera = AnthemCamera(AT_ACPT_PERSPECTIVE);
}core;

struct FontLib {
	FT_Library ft;
	FT_Face face;

	AnthemImage** glyphTex;
	AnthemDescriptorPool** glyphDescPool;
	unsigned char** glyphBuffer;
	unsigned char** glyphBufferUni;
	int* glyphWidth;
	int* glyphRows;
	int* glyphBearingX;
	int* glyphBearingY;
	int* advance;

	AnthemImage* tableTex;
	AnthemDescriptorPool* tableDescPool;

}font;

struct ComputePipeline {
	AnthemShaderStorageBufferImpl<AtBufVecd4f<1>, AtBufVecd4f<1>>* samples = nullptr;
	AnthemUniformBufferImpl<AnthemUniformVecf<4>, AnthemUniformVecf<4>>* uniform = nullptr;

	AnthemDescriptorPool* descPoolUniform = nullptr;
	AnthemDescriptorPool* descPoolSsbo = nullptr;

	AnthemShaderFilePaths shaderFile;
	AnthemShaderModule* shader = nullptr;
	AnthemComputePipeline* pipeline = nullptr;

	AnthemFence** computeProgress = nullptr;;
	AnthemSemaphore** computeDone = nullptr;

	uint32_t* computeCmdBufIdx = nullptr;
}comp;

struct VisualizationPipeline {
	AnthemIndexBuffer* ix = nullptr;
	AnthemDepthBuffer* depthBuffer = nullptr;

	AnthemShaderFilePaths shaderFile;
	AnthemShaderModule* shader = nullptr;
	AnthemGraphicsPipeline* pipeline = nullptr;
	AnthemSwapchainFramebuffer* framebuffer = nullptr;
	AnthemRenderPass* renderPass = nullptr;

	AnthemGraphicsPipelineCreateProps cprop;
	AnthemSemaphore** firstStageDone = nullptr;


	using uProjMat = AnthemUniformMatf<4>;
	using uViewMat = AnthemUniformMatf<4>;
	using uLocalMat = AnthemUniformMatf<4>;
	AnthemDescriptorPool* descVisUniform;
	AnthemUniformBufferImpl<uProjMat, uViewMat, uLocalMat>* visUniform;
}vis;

struct CoordAxisVisPipeline {
	AnthemVertexBufferImpl<AnthemVAOAttrDesc<float, 4>, AnthemVAOAttrDesc<float, 4>, AnthemVAOAttrDesc<float, 1>>* vxBuf;
	AnthemIndexBuffer* ixBuf;
	AnthemShaderFilePaths shaderFile;
	AnthemShaderModule* shader = nullptr;
	AnthemGraphicsPipeline* pipeline = nullptr;
	AnthemRenderPass* renderPass = nullptr;

	uint32_t* axisCmdBuf = nullptr;
	AnthemFence** drawProgress = nullptr;
	AnthemSemaphore** drawAvailable = nullptr;
	AnthemGraphicsPipelineCreateProps cprop;
}axis;

struct StringDataBuffer {
	using uProj = AnthemUniformMatf<4>;
	using uView = AnthemUniformMatf<4>;
	using uLocal = AnthemUniformMatf<4>;
	AnthemDescriptorPool* descPool;
	AnthemUniformBufferImpl<uProj, uView, uLocal>* uBuf;
	AnthemVertexBufferImpl<AnthemVAOAttrDesc<float, 4>, AnthemVAOAttrDesc<float, 2>>* vxBuf;
	AnthemIndexBuffer* ixBuf;
	float translationX = 0.0f;
	float translationY = 0.0f;
	float totalWidth = 0.0f;
	float firstBearingY = 0.0f;
};

struct FontVisPipeline {
	std::vector<StringDataBuffer> strings;

	AnthemShaderFilePaths shaderFile;
	AnthemShaderModule* shader = nullptr;
	AnthemGraphicsPipeline* pipeline = nullptr;
	AnthemRenderPass* renderPass = nullptr;

	uint32_t* fontCmdBuf = nullptr;
	AnthemFence** drawProgress = nullptr;
	AnthemSemaphore** drawAvailable = nullptr;
	AnthemGraphicsPipelineCreateProps cprop;

	std::vector<int> tickLabelX;
	std::vector<int> tickLabelY;
	std::vector<int> tickLabelZ;

	std::vector<AtVecf4> tickPosX;
	std::vector<AtVecf4> tickPosY;
	std::vector<AtVecf4> tickPosZ;

}textPipe;

struct ColorRampLUT2D {
	AnthemDescriptorPool* lutDesc;
	AnthemImage* lutTex;
}clut;

template<typename T>
std::string to_string_with_prec(T x, int prec) {
	std::stringstream ss;
	ss << std::setprecision(prec) << x;
	return ss.str();
}

inline std::string getShader(auto x) {
	std::string st(ANTH_SHADER_DIR);
	st += "angleReproj\\shader.";
	st += x;
	st += ".spv";
	return st;
}

void prepareCore() {
	core.renderer.setConfig(&core.cfg);
	core.renderer.initialize();
}

void setStringPosition(int i, float tX, float tY, bool center=false,bool right=false,bool alignYTop=false) {
	textPipe.strings.at(i).translationX = tX;
	textPipe.strings.at(i).translationY = tY;
	if (center) {
		textPipe.strings.at(i).translationX -= textPipe.strings.at(i).totalWidth / 2;
	}
	if (right) {
		textPipe.strings.at(i).translationX -= textPipe.strings.at(i).totalWidth;
	}
	if (alignYTop) {
		textPipe.strings.at(i).translationY += textPipe.strings.at(i).firstBearingY;
	}
}

void updateTextPipeUniform() {
	for (auto i : std::ranges::views::iota(0, static_cast<int>(textPipe.strings.size()))) {
		auto proj = AnthemLinAlg::eye<float, 4>();
		auto view = AnthemLinAlg::eye<float, 4>();
		auto local = AnthemLinAlg::eye<float, 4>();

		proj[0][3] = textPipe.strings[i].translationX;
		proj[1][3] = textPipe.strings[i].translationY;

		float projRaw[16], viewRaw[16], localRaw[16];
		proj.columnMajorVectorization(projRaw);
		view.columnMajorVectorization(viewRaw);
		local.columnMajorVectorization(localRaw);

		textPipe.strings[i].uBuf->specifyUniforms(projRaw, viewRaw, localRaw);
		for (auto j : std::ranges::iota_view(0, core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
			textPipe.strings[i].uBuf->updateBuffer(j);
		}
	}
}

int insertString(std::string strs, float scale) {
	StringDataBuffer sdb;
	int stringLength = strs.length();
	core.renderer.createDescriptorPool(&sdb.descPool);
	core.renderer.createUniformBuffer(&sdb.uBuf, 0, sdb.descPool);
	core.renderer.createVertexBuffer(&sdb.vxBuf);
	core.renderer.createIndexBuffer(&sdb.ixBuf);

	// Set locations
	float curX = 0.0f, curY = 0.0f;
	int idx = 0;
	std::vector<unsigned int> indices;
	sdb.vxBuf->setTotalVertices(6 * stringLength);

	int rdW, rdH;
	core.renderer.exGetWindowSize(rdH, rdW);
	float aspect = 1.0 * rdW / rdH;

	for (const auto& ch : strs) {
		if (ch == ' ') {
			curX += 16.0 * scale;
			continue;
		}
		float xp = curX + (font.glyphBearingX[ch]) * scale;
		float yp = curY - ( font.glyphBearingY[ch]) * scale;
		float w = font.glyphWidth[ch] * scale, h = font.glyphRows[ch] * scale;

		ANTH_LOGI("Rows", font.glyphRows[ch], " Bearing", font.glyphBearingY[ch], " Ch", ch, "Advance", font.advance[ch]);

		float lutStX = (ch % 8 * 64) / 1024.0;
		float lutStY = (ch / 8 * 64) / 1024.0;
		float lutEdX = lutStX + 64.0 / 1024.0;
		float lutEdY = lutStY + 64.0 / 1024.0;

		auto tmp = lutEdY;
		lutEdY = lutStY;
		lutStY = tmp;

		xp = xp / aspect;
		w = w / aspect;

		sdb.vxBuf->insertData(idx + 0, { xp,yp + h,0,1 }, { lutStX,lutStY });
		sdb.vxBuf->insertData(idx + 1, { xp,yp,0,1 }, { lutStX,lutEdY });
		sdb.vxBuf->insertData(idx + 2, { xp + w,yp,0,1 }, { lutEdX,lutEdY });
		sdb.vxBuf->insertData(idx + 3, { xp,yp + h,0,1 }, { lutStX,lutStY });
		sdb.vxBuf->insertData(idx + 4, { xp + w,yp,0,1 }, { lutEdX,lutEdY });
		sdb.vxBuf->insertData(idx + 5, { xp + w,yp + h,0,1 }, { lutEdX,lutStY });
		for (auto i : std::ranges::views::iota(idx, idx + 6)) {
			indices.push_back(i);
		}
		curX += (font.advance[ch] >> 6) * scale;
		idx += 6;

		ANTH_LOGI("LP:", xp, " ", yp);
		sdb.firstBearingY = font.glyphBearingY[ch] * scale;
	}
	sdb.ixBuf->setIndices(indices);
	sdb.totalWidth = curX;
	textPipe.strings.push_back(std::move(sdb));
	//throw;
	return textPipe.strings.size() - 1;
}


void prepareComputePipeline() {
	// Create descriptors
	core.renderer.createDescriptorPool(&comp.descPoolSsbo);
	core.renderer.createDescriptorPool(&comp.descPoolUniform);

	// Loading shader
	comp.shaderFile.computeShader = getShader("comp");
	core.renderer.createShader(&comp.shader, &comp.shaderFile);

	// Filling experiment data
	using ssboType = std::remove_cv_t<decltype(comp.samples)>;
	std::function<void(ssboType)> ssboCreateFunc = [&](ssboType w) {
		auto childJob = [&](int segment, int totalSegments)->void {
			auto st = ExpParams::sampleCounts / totalSegments * segment;
			auto ed = ExpParams::sampleCounts / totalSegments * (segment + 1);
			for (auto i : std::ranges::views::iota(st, ed)) {
				auto fx = AnthemLinAlg::randomNumber<float>();
				auto fy = AnthemLinAlg::randomNumber<float>();
				fx = 2.0f * (fx - 0.5f);
				fy = 2.0f * (fy - 0.5f);
				w->setInput(i, { fx, fy, 0.0f, 0.0f }, { fx, fy, 0.0f, 0.0f });
			}
		};
		std::vector<std::thread> jobs;
		for (auto i : std::views::iota(0, ExpParams::parallelsXCpu)) {
			std::thread childExec(childJob, i, ExpParams::parallelsXCpu);
			jobs.push_back(std::move(childExec));
		}
		for (auto& p : jobs) {
			p.join();
		}
		ANTH_LOGI("Random numbers generated");
	};
	core.renderer.createShaderStorageBuffer(&comp.samples, ExpParams::sampleCounts, 0, comp.descPoolSsbo, std::make_optional(ssboCreateFunc));

	// Create Uniform Buffer
	core.renderer.createUniformBuffer(&comp.uniform, 0, comp.descPoolUniform);
	float temp1[4] = { 0,0,5,0 };
	float temp2[4] = { 0,0,0,0 };
	comp.uniform->specifyUniforms(temp1, temp2);


	// Create Pipeline
	AnthemDescriptorSetEntry dseUniformIn = {
		.descPool = comp.descPoolUniform,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseSsboIn = {
		.descPool = comp.descPoolSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseSsboOut = {
		.descPool = comp.descPoolSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> vct = { dseUniformIn,dseSsboIn,dseSsboOut };
	core.renderer.createComputePipelineCustomized(&comp.pipeline, vct, comp.shader);

	// Allocate Command Buffers
	comp.computeCmdBufIdx = new uint32_t[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0,core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.drAllocateCommandBuffer(&comp.computeCmdBufIdx[i]);
	}

	// Create Sync Objects
	comp.computeProgress = new AnthemFence*[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	comp.computeDone = new AnthemSemaphore*[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0,core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.createFence(&comp.computeProgress[i]);
		core.renderer.createSemaphore(&comp.computeDone[i]);
	}
}
void prepareColorRamp() {
	ANTH_LOGI("Preparing Color Map");
	unsigned char* baseMap = new unsigned char[64 * 64];
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			baseMap[i*64+j] = i * 255 / 64;
		}
	}
	cv::Mat srcMap(64, 64, CV_8UC1, baseMap);
	cv::Mat destMap;
	cv::applyColorMap(srcMap, destMap, cv::COLORMAP_JET);
	cv::cvtColor(destMap, destMap, cv::COLOR_BGR2RGBA);

	core.renderer.createDescriptorPool(&clut.lutDesc);
	core.renderer.createTexture(&clut.lutTex, clut.lutDesc, destMap.data, 64, 64, 4, 0, false, false);
}

void updateVisUniform() {
	int rdH, rdW;
	core.renderer.exGetWindowSize(rdH, rdW);
	core.camera.specifyFrustum(AT_PI / 3 * 1, 0.1, 100, 1.0 * rdW / rdH);
	core.camera.specifyPosition(0.0, 1.5, -3.4f);
	core.camera.specifyFrontEyeRay(0, -1.5, 3.4f);

	auto axis = Math::AnthemVector<float, 3>({ 0.0f,1.0f,0.0f });
	auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis, ExpParams::visRot);

	AtMatf4 proj, view, model;
	float projRaw[16], viewRaw[16], modelRaw[16];
	core.camera.getProjectionMatrix(proj);
	core.camera.getViewMatrix(view);
	model = local.multiply(Math::AnthemLinAlg::eye<float, 4>());

	proj.columnMajorVectorization(projRaw);
	view.columnMajorVectorization(viewRaw);
	model.columnMajorVectorization(modelRaw);

	vis.visUniform->specifyUniforms(projRaw, viewRaw, modelRaw);
	for (auto i : std::ranges::views::iota(0, 2)) {
		vis.visUniform->updateBuffer(i);
	}

}

void prepareVisualization() {
	// Create required buffers
	core.renderer.createIndexBuffer(&vis.ix);
	core.renderer.createDepthBuffer(&vis.depthBuffer, false);
	auto ind = std::ranges::views::iota(0,ExpParams::sampleCounts) | std::ranges::to<std::vector<unsigned int>>();
	vis.ix->setIndices(ind);

	// Load shaders
	vis.shaderFile.vertexShader = getShader("vert");
	vis.shaderFile.fragmentShader = getShader("frag");
	core.renderer.createShader(&vis.shader, &vis.shaderFile);

	// Create Uniform Buffer
	core.renderer.createDescriptorPool(&vis.descVisUniform);
	core.renderer.createUniformBuffer(&vis.visUniform, 0, vis.descVisUniform);
	updateVisUniform();

	// Create Pipeline
	core.renderer.setupDemoRenderPass(&vis.renderPass, vis.depthBuffer);
	
	AnthemRenderPassSetupOption opt;
	opt.predefinedClearColor = { 1,1,1,1.0 };
	opt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	opt.msaaType = AT_ARPMT_NO_MSAA;

	core.renderer.setupRenderPass(&vis.renderPass, &opt, vis.depthBuffer);
	core.renderer.createSwapchainImageFramebuffers(&vis.framebuffer, vis.renderPass, vis.depthBuffer);
	
	vis.cprop.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_POINT_LIST;


	// Assemble Pipeline
	AnthemDescriptorSetEntry uniformBufferDescEntryRegPipeline = {
		.descPool = vis.descVisUniform,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry cRamp = {
		.descPool = clut.lutDesc,
		.descSetType = AT_ACDS_SAMPLER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { uniformBufferDescEntryRegPipeline,cRamp };

	core.renderer.createGraphicsPipelineCustomized(&vis.pipeline, descSetEntriesRegPipeline, vis.renderPass, vis.shader,
		comp.samples, &vis.cprop);

	// Create Sync
	vis.firstStageDone = new AnthemSemaphore * [core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0, core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.createSemaphore(&vis.firstStageDone[i]);
	}
}

void prepareTextVis() {
	// Demo string
	insertString("Figure 1",0.0012);

	// Prepare comps
	textPipe.shaderFile.vertexShader = getShader("text.vert");
	textPipe.shaderFile.fragmentShader = getShader("text.frag");
	core.renderer.createShader(&textPipe.shader, &textPipe.shaderFile);

	AnthemRenderPassSetupOption opt;
	opt.renderPassUsage = AT_ARPAA_FINAL_PASS;
	opt.msaaType = AT_ARPMT_NO_MSAA;
	opt.clearColorAttachmentOnLoad[0] = false;
	opt.clearDepthAttachmentOnLoad = false;
	core.renderer.setupRenderPass(&textPipe.renderPass, &opt, vis.depthBuffer);

	// Setup Pipeline
	textPipe.cprop.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_TRIANGLE_LIST;
	textPipe.cprop.blendPreset = AnthemBlendPreset::AT_ABP_DEFAULT_TRANSPARENCY;

	AnthemDescriptorSetEntry uniformBufferDescEntryRegPipeline = {
		.descPool = textPipe.strings[0].descPool,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry samplerReg = {
		.descPool = font.tableDescPool,
		.descSetType = AT_ACDS_SAMPLER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { uniformBufferDescEntryRegPipeline,samplerReg };

	core.renderer.createGraphicsPipelineCustomized(&textPipe.pipeline, descSetEntriesRegPipeline, textPipe.renderPass, textPipe.shader,
		textPipe.strings[0].vxBuf, &textPipe.cprop);

	// Create Sync
	textPipe.fontCmdBuf = new uint32_t[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	textPipe.drawAvailable = new AnthemSemaphore*[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0,core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.drAllocateCommandBuffer(&textPipe.fontCmdBuf[i]);
		core.renderer.createSemaphore(&textPipe.drawAvailable[i]);
	}
}

void prepareAxisVis() {
	// Create req bufs
	core.renderer.createVertexBuffer(&axis.vxBuf);
	
	float axisCrd = -ExpParams::axisOriginCrd;
	float baseZCrd = ExpParams::axisOriginCrdZ, extZCrd = ExpParams::axisExtendCrdZ;
	int density = 5;
	int totl = 6 + 12 * (density );
	axis.vxBuf->setTotalVertices(totl);
	// X Axis
	axis.vxBuf->insertData(0, { -axisCrd, axisCrd, baseZCrd,1.0f }, { -axisCrd, axisCrd, baseZCrd,1.0f }, { 0.0f });
	axis.vxBuf->insertData(1, { -axisCrd, -axisCrd, baseZCrd,1.0f }, { -axisCrd, -axisCrd,baseZCrd,1.0f }, { 0.0f });

	// Y Axis
	axis.vxBuf->insertData(2, { -axisCrd, -axisCrd, baseZCrd,1.0f }, { -axisCrd, -axisCrd, baseZCrd,1.0f }, { 0.0f });
	axis.vxBuf->insertData(3, { axisCrd, -axisCrd, baseZCrd,1.0f }, { axisCrd, -axisCrd,baseZCrd,1.0f }, { 0.0f });
	
	// Z Axis
	axis.vxBuf->insertData(4, { axisCrd, -axisCrd, baseZCrd,1.0f }, { axisCrd, -axisCrd, baseZCrd,1.0f }, { 0.0f });
	axis.vxBuf->insertData(5, { axisCrd, -axisCrd, extZCrd,1.0f }, { axisCrd, -axisCrd, extZCrd,1.0f }, { 0.0f });

	// Back
	auto mix = []<typename T>(T s, T t, T v)->T {
		return v * t + (1 - v) * s;
	};
	for (int i = 0; i < density; i++) {
		auto yp = mix(-axisCrd, axisCrd, (float)i / (float)density);
		axis.vxBuf->insertData(6 + 2 * (i ), { yp, axisCrd, extZCrd,1.0f }, { yp, axisCrd, extZCrd,1.0f }, { 1.0f });
		axis.vxBuf->insertData(6 + 2 * (i ) + 1, { yp, -axisCrd, extZCrd,1.0f }, { yp, -axisCrd,extZCrd,1.0f }, { 1.0f });
	}

	for (int i = 0; i < density; i++) {
		auto yp = mix(-axisCrd, axisCrd, (float)i / (float)density);
		axis.vxBuf->insertData(6 + 2 * (density ) + 2 * (i ), { -axisCrd, yp, extZCrd,1.0f }, { -axisCrd, yp, extZCrd,1.0f }, { 1.0f });
		axis.vxBuf->insertData(6 + 2 * (density ) + 2 * (i ) + 1, { axisCrd, yp, extZCrd,1.0f }, { axisCrd,yp,extZCrd,1.0f }, { 1.0f });
	}

	// Z Sub Axis
	for (int i = 0; i < density; i++) {
		auto yp = mix(-axisCrd, axisCrd, (float)i / (float)density);
		axis.vxBuf->insertData(6 + 4 * (density ) + 2 * (i ), { -axisCrd, yp, baseZCrd,1.0f }, { -axisCrd, yp, baseZCrd,1.0f }, { 1.0f });
		axis.vxBuf->insertData(6 + 4 * (density ) + 2 * (i ) + 1, { -axisCrd, yp, extZCrd,1.0f }, { -axisCrd,yp,extZCrd,1.0f }, { 1.0f });
	}

	for (int i = 0; i < density; i++) {
		auto yp = mix(-axisCrd, axisCrd, (float)i / (float)density);
		axis.vxBuf->insertData(6 + 6 * (density)+2 * (i), { yp , -axisCrd, baseZCrd,1.0f }, { yp, -axisCrd, baseZCrd,1.0f }, { 1.0f });
		axis.vxBuf->insertData(6 + 6 * (density)+2 * (i)+1, { yp, -axisCrd, extZCrd,1.0f }, { yp,-axisCrd,extZCrd,1.0f }, { 1.0f });
	}

	// Other
	for (int i = 0; i < density; i++) {
		auto yp = mix(baseZCrd, extZCrd, (float)i / (float)density);
		axis.vxBuf->insertData(6 + 8 * (density)+2 * (i), { -axisCrd, axisCrd, yp,1.0f }, { -axisCrd, axisCrd, yp,1.0f }, { 1.0f });
		axis.vxBuf->insertData(6 + 8 * (density)+2 * (i)+1, { -axisCrd, -axisCrd, yp,1.0f }, { -axisCrd, -axisCrd,yp,1.0f }, { 1.0f });
	}

	for (int i = 0; i < density; i++) {
		auto yp = mix(baseZCrd, extZCrd, (float)i / (float)density);
		axis.vxBuf->insertData(6 + 10 * (density)+2 * (i), { axisCrd, -axisCrd, yp,1.0f }, { axisCrd, -axisCrd, yp,1.0f }, { 1.0f });
		axis.vxBuf->insertData(6 + 10 * (density)+2 * (i)+1, { -axisCrd, -axisCrd, yp,1.0f }, { -axisCrd, -axisCrd,yp,1.0f }, { 1.0f });
	}


	core.renderer.createIndexBuffer(&axis.ixBuf);
	axis.ixBuf->setIndices(std::views::iota(0u,static_cast<unsigned>(totl))|std::ranges::to<std::vector>());

	// Load shaders
	axis.shaderFile.vertexShader = getShader("axis.vert");
	axis.shaderFile.fragmentShader = getShader("axis.frag");
	core.renderer.createShader(&axis.shader, &axis.shaderFile);

	core.renderer.setupDemoRenderPass(&axis.renderPass, vis.depthBuffer,true);

	axis.cprop.inputTopo = AnthemInputAssemblerTopology::AT_AIAT_LINE;
	AnthemDescriptorSetEntry uniformBufferDescEntryRegPipeline = {
		.descPool = vis.descVisUniform,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { uniformBufferDescEntryRegPipeline };

	core.renderer.createGraphicsPipelineCustomized(&axis.pipeline, descSetEntriesRegPipeline, axis.renderPass, axis.shader,
		axis.vxBuf, &axis.cprop);

	axis.drawProgress = new AnthemFence * [core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	axis.drawAvailable = new AnthemSemaphore* [core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0, core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.createFence(&axis.drawProgress[i]);
		core.renderer.createSemaphore(&axis.drawAvailable[i]);
	}

	// Allocate Command Buffers
	axis.axisCmdBuf = new uint32_t[core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT];
	for (auto i : std::ranges::views::iota(0, core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT)) {
		core.renderer.drAllocateCommandBuffer(&axis.axisCmdBuf[i]);
	}
}


void recordCommandBufferDrwAxis(int i) {
	auto& renderer = core.renderer;
	renderer.drStartRenderPass(axis.renderPass, (AnthemFramebuffer*)(vis.framebuffer->getFramebufferObject(i)), axis.axisCmdBuf[i], false);
	renderer.drSetViewportScissor(axis.axisCmdBuf[i]);
	renderer.drBindGraphicsPipeline(axis.pipeline, axis.axisCmdBuf[i]);
	renderer.drBindVertexBuffer(axis.vxBuf, axis.axisCmdBuf[i]);
	renderer.drBindIndexBuffer(axis.ixBuf, axis.axisCmdBuf[i]);
	renderer.drSetLineWidth(ExpParams::lineWidth, axis.axisCmdBuf[i]);

	AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
		.descPool = vis.descVisUniform,
		.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> descSetEntries = { uniformBufferDescEntryRdw };
	renderer.drBindDescriptorSetCustomizedGraphics(descSetEntries, axis.pipeline, axis.axisCmdBuf[i]);


	renderer.drDraw(axis.ixBuf->getIndexCount(), axis.axisCmdBuf[i]);
	renderer.drEndRenderPass(axis.axisCmdBuf[i]);
}

void recordCommandBufferDrwText(int i) {
	auto& renderer = core.renderer;
	renderer.drStartRenderPass(textPipe.renderPass, (AnthemFramebuffer*)(vis.framebuffer->getFramebufferObject(i)), textPipe.fontCmdBuf[i], false);
	renderer.drSetViewportScissor(textPipe.fontCmdBuf[i]);
	renderer.drBindGraphicsPipeline(textPipe.pipeline, textPipe.fontCmdBuf[i]);
	for (int j = 0; j < textPipe.strings.size(); j++) {
		renderer.drBindVertexBuffer(textPipe.strings[j].vxBuf, textPipe.fontCmdBuf[i]);
		renderer.drBindIndexBuffer(textPipe.strings[j].ixBuf, textPipe.fontCmdBuf[i]);

		AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
			.descPool = textPipe.strings[j].descPool,
			.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
			.inTypeIndex = 0
		};
		AnthemDescriptorSetEntry samplerRdw = {
			.descPool = font.tableDescPool,
			.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_SAMPLER,
			.inTypeIndex = 0
		};
		std::vector<AnthemDescriptorSetEntry> descSetEntries = { uniformBufferDescEntryRdw,samplerRdw };
		renderer.drBindDescriptorSetCustomizedGraphics(descSetEntries, textPipe.pipeline, textPipe.fontCmdBuf[i]);
		renderer.drDraw(textPipe.strings[j].ixBuf->getIndexCount(), textPipe.fontCmdBuf[i]);
	}

	renderer.drEndRenderPass(textPipe.fontCmdBuf[i]);
}

void recordCommandBufferDrw(int i) {
	auto& renderer = core.renderer;
	renderer.drStartRenderPass(vis.renderPass, (AnthemFramebuffer*)(vis.framebuffer->getFramebufferObject(i)), i, false);
	renderer.drSetViewportScissor(i);
	renderer.drBindGraphicsPipeline(vis.pipeline, i);
	renderer.drBindVertexBufferFromSsbo(comp.samples, 0, i);
	renderer.drBindIndexBuffer(vis.ix, i);

	AnthemDescriptorSetEntry uniformBufferDescEntryRdw = {
			.descPool = vis.descVisUniform,
			.descSetType = AnthemDescriptorSetEntrySourceType::AT_ACDS_UNIFORM_BUFFER,
			.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry cRamp = {
		.descPool = clut.lutDesc,
		.descSetType = AT_ACDS_SAMPLER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> descSetEntries = { uniformBufferDescEntryRdw,cRamp };
	renderer.drBindDescriptorSetCustomizedGraphics(descSetEntries, vis.pipeline, i);

	renderer.drDraw(vis.ix->getIndexCount(), i);
	renderer.drEndRenderPass(i);
}


void recordCommandBufferComp(int i) {
	auto& renderer = core.renderer;
	renderer.drBindComputePipeline(comp.pipeline, comp.computeCmdBufIdx[i]);
	AnthemDescriptorSetEntry dseUniform = {
		.descPool = comp.descPoolUniform,
		.descSetType = AT_ACDS_UNIFORM_BUFFER,
		.inTypeIndex = 0
	};
	AnthemDescriptorSetEntry dseSsboIn = {
		.descPool = comp.descPoolSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 1
	};
	AnthemDescriptorSetEntry dseSsboOut = {
		.descPool = comp.descPoolSsbo,
		.descSetType = AT_ACDS_SHADER_STORAGE_BUFFER,
		.inTypeIndex = 0
	};
	std::vector<AnthemDescriptorSetEntry> descSetEntriesRegPipeline = { dseUniform,dseSsboIn,dseSsboOut };
	renderer.drBindDescriptorSetCustomizedCompute(descSetEntriesRegPipeline, comp.pipeline, comp.computeCmdBufIdx[i]);
	renderer.drComputeDispatch(comp.computeCmdBufIdx[i], ExpParams::parallelsXGpu, 1, 1);

}

void recordCommandBufferCompAll() {
	auto& renderer = core.renderer;
	for (int i = 0; i < core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {
		renderer.drStartCommandRecording(comp.computeCmdBufIdx[i]);
		ANTH_LOGI("Start Recording:", comp.computeCmdBufIdx[i]);
		recordCommandBufferComp(i);
		renderer.drEndCommandRecording(comp.computeCmdBufIdx[i]);
	}
}

void recordCommandBufferAll() {
	auto& renderer = core.renderer;
	for (int i = 0; i < core.cfg.VKCFG_MAX_IMAGES_IN_FLIGHT; i++) {

		renderer.drStartCommandRecording(i);
		recordCommandBufferDrw(i);
		renderer.drEndCommandRecording(i);

		renderer.drStartCommandRecording(axis.axisCmdBuf[i]);
		recordCommandBufferDrwAxis(i);
		renderer.drEndCommandRecording(axis.axisCmdBuf[i]);


		renderer.drStartCommandRecording(textPipe.fontCmdBuf[i]);
		recordCommandBufferDrwText(i);
		renderer.drEndCommandRecording(textPipe.fontCmdBuf[i]);
	}
}

void prepareImguiFrame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Hello, world!");                          
	ImGui::Text("This is some useful text.");
	ImGui::End();
}

void drawLoop(int& currentFrame) {
	prepareImguiFrame();
	uint32_t imgIdx;
	
	core.renderer.drPrepareFrame(currentFrame, &imgIdx);

	// Computing
	comp.computeProgress[0]->waitForFence();
	comp.computeProgress[0]->resetFence();

	std::vector<const AnthemSemaphore*> semaphoreToSignal = { comp.computeDone[0] };
	std::vector<AtSyncSemaphoreWaitStage> waitStage = { AtSyncSemaphoreWaitStage::AT_SSW_VERTEX_INPUT };

	std::vector<const AnthemSemaphore*> semaphoreToSignalAxis = { axis.drawAvailable[currentFrame] };
	std::vector<AtSyncSemaphoreWaitStage> waitStageAxis = { AtSyncSemaphoreWaitStage::AT_SSW_COLOR_ATTACH_OUTPUT };
	std::vector<const AnthemSemaphore*> firstStageDone = { vis.firstStageDone[currentFrame] };
	std::vector<const AnthemSemaphore*> secondStageDone = { textPipe.drawAvailable[currentFrame] };

	core.renderer.drSubmitCommandBufferCompQueueGeneral(comp.computeCmdBufIdx[0], nullptr, &semaphoreToSignal, comp.computeProgress[0]);

	// Drawing
	ImGui::Render();
	ImDrawData* drawData = ImGui::GetDrawData();
	core.renderer.exRenderImGui(currentFrame, vis.framebuffer, { 0,0,0,1 }, drawData);
	uint32_t imguiCmdBuf;
	core.renderer.exGetImGuiCommandBufferIndex(currentFrame, &imguiCmdBuf);
	AnthemSemaphore* imguiSemaphore;
	core.renderer.exGetImGuiDrawProgressSemaphore(currentFrame, &imguiSemaphore);
	std::vector<const AnthemSemaphore*> imguiDone = { imguiSemaphore };

	ANTH_ASSERT((vis.firstStageDone[currentFrame] != nullptr),"");
	core.renderer.drSubmitCommandBufferGraphicsQueueGeneral2(currentFrame, imgIdx, &semaphoreToSignal, &waitStage,nullptr,&firstStageDone);
	core.renderer.drSubmitCommandBufferGraphicsQueueGeneral2(imguiCmdBuf, imgIdx, &firstStageDone, &waitStage, nullptr, &imguiDone);
	core.renderer.drSubmitCommandBufferGraphicsQueueGeneral2(textPipe.fontCmdBuf[currentFrame], imgIdx, &imguiDone, &waitStage, nullptr, &secondStageDone);
	core.renderer.drSubmitCommandBufferGraphicsQueueGeneral(axis.axisCmdBuf[currentFrame], imgIdx, &secondStageDone,
		&waitStageAxis);

	core.renderer.drPresentFrame(currentFrame, imgIdx);
	currentFrame++;
	currentFrame %= 2;
}

void initFont() {
	ANTH_ASSERT(!FT_Init_FreeType(&font.ft), "Cannot init font lib");
	ANTH_ASSERT(!FT_New_Face(font.ft, R"(C:\WR\Aria\Anthem\assets\times.ttf)", 0, &font.face), "Cannot init font face");
	FT_Set_Pixel_Sizes(font.face, 0, 48);
	font.glyphBuffer = new unsigned char* [128];
	font.glyphTex = new AnthemImage * [128];
	font.glyphRows = new int[128];
	font.glyphWidth = new int[128];
	font.glyphDescPool = new AnthemDescriptorPool * [128];
	font.glyphBufferUni = new unsigned char* [128];
	font.glyphBearingX = new int[128];
	font.glyphBearingY = new int[128];
	font.advance = new int[128];

	std::vector<std::vector<char>> goals = {
		std::ranges::views::iota('a', 'z' + 1) | std::ranges::to<std::vector>(),
		std::ranges::views::iota('A', 'Z' + 1) | std::ranges::to<std::vector>(),
		std::ranges::views::iota('0', '9' + 1) | std::ranges::to<std::vector>(),
		std::ranges::views::iota('.', '.' + 1) | std::ranges::to<std::vector>(),
		std::ranges::views::iota(' ', ' ' + 1) | std::ranges::to<std::vector>(),
	};
	auto goalsRange = std::ranges::join_view(goals);
	cv::Mat fontTable(1024, 1024, CV_8UC1);
	for (auto i : goalsRange) {
		ANTH_LOGI("Generating Character:", i);
		
		ANTH_ASSERT(!FT_Load_Char(font.face, i, FT_LOAD_RENDER), "Cannot load glyph, ascii=", i);
		size_t bufferSize = font.face->glyph->bitmap.width * font.face->glyph->bitmap.rows;
		using bufferType = std::remove_pointer_t<decltype(font.face->glyph->bitmap.buffer)>;
		if (!bufferSize) {
			font.glyphBuffer[i] = nullptr;
			std::tie(font.glyphRows[i], font.glyphWidth[i]) = std::make_tuple(0, 0);
			ANTH_LOGI("Ignored character:", i);
			continue;
		}

		// Filling in font data
		font.glyphBuffer[i] = new unsigned char[sizeof(bufferType) * bufferSize];
		font.glyphRows[i] = font.face->glyph->bitmap.rows;
		font.glyphWidth[i] = font.face->glyph->bitmap.width;
		font.glyphBearingX[i] = font.face->glyph->bitmap_left;
		font.glyphBearingY[i] = font.face->glyph->bitmap_top;
		font.advance[i] = font.face->glyph->advance.x;

		ANTH_LOGI("Allocated buf:", sizeof(bufferType) * bufferSize, "@", (long long)(font.glyphBuffer[i]));
		memcpy(font.glyphBuffer[i], font.face->glyph->bitmap.buffer, sizeof(bufferType) * bufferSize);

		// Resize
		cv::Mat glyphMat(font.glyphRows[i], font.glyphWidth[i], CV_8UC1, font.glyphBuffer[i]);
		cv::Mat dst;
		cv::resize(glyphMat, dst, cv::Size(ExpParams::fontSize, ExpParams::fontSize));
		font.glyphBufferUni[i] = new unsigned char[sizeof(bufferType) * ExpParams::fontSize * ExpParams::fontSize];
		memcpy(font.glyphBufferUni[i], dst.data, sizeof(bufferType) * ExpParams::fontSize * ExpParams::fontSize);

		// Integrate
		int lutX = i % 8, lutY = i / 8;
		cv::Mat rect(fontTable, cv::Rect(lutX * 64, lutY * 64, 64, 64));
		dst.copyTo(rect);
	}
	core.renderer.createDescriptorPool(&(font.tableDescPool));
	core.renderer.createTexture(&font.tableTex, font.tableDescPool, fontTable.data, 1024, 1024, 1, 0, false, false, AT_IF_R_UINT8);
}

void initText() {
	//Prepare ticks
	int density = 5;
	float scale = 0.0010;

	for (int i = 1; i < density; i++) {
		std::string text = to_string_with_prec(1.0 / density * i,2);
		std::string textZ = std::to_string((int)(1.0 / density * i * 180));

		ANTH_LOGI(text);
		textPipe.tickLabelX.push_back(insertString(text, scale));
		auto w = insertString(textZ, scale);
		textPipe.tickLabelY.push_back(w);
		ANTH_LOGI(w, ",", textZ);
		textPipe.tickLabelZ.push_back(insertString(text, scale));

		float st = ExpParams::axisOriginCrd, stz = ExpParams::axisOriginCrdZ;
		float ed = ExpParams::axisExtendCrd, edz = ExpParams::axisExtendCrdZ;
		auto mix = []<typename T>(T s, T t, T v)->T {
			return v * t + (1 - v) * s;
		};
		float mx = mix(st, ed, 1.0f * i / density), mxz = mix(stz, edz, 1.0f * i / density);
		textPipe.tickPosX.push_back({ mx,st,stz,1.0f });
		textPipe.tickPosY.push_back({ st,mx,stz,1.0f });
		textPipe.tickPosZ.push_back({ ed,st,mxz,1.0f });
	}
	//throw;
}

void updateTextPositions() {
	setStringPosition(0, 0, -0.65, true);
	AtMatf4 proj, view;

	core.camera.getProjectionMatrix(proj);
	core.camera.getViewMatrix(view);
	auto axis = Math::AnthemVector<float, 3>({ 0.0f,1.0f,0.0f });
	auto local = Math::AnthemLinAlg::axisAngleRotationTransform3(axis, ExpParams::visRot);
	AtMatf4 transform = proj.multiply(view).multiply(local);

	for (int i = 0; i < textPipe.tickLabelX.size(); i++) {
		auto ssPx = AnthemLinAlg::linearTransform<float, 4, 4>(transform, textPipe.tickPosX[i]);
		setStringPosition(textPipe.tickLabelX[i], ssPx[0] / ssPx[3], ssPx[1] / ssPx[3],false, true,true);
	}
	for (int i = 0; i < textPipe.tickLabelY.size(); i++) {
		auto ssPx = AnthemLinAlg::linearTransform<float, 4, 4>(transform, textPipe.tickPosY[i]);
		setStringPosition(textPipe.tickLabelY[i], ssPx[0] / ssPx[3], ssPx[1] / ssPx[3],false,true);
	}
	for (int i = 0; i < textPipe.tickLabelZ.size(); i++) {
		auto ssPx = AnthemLinAlg::linearTransform<float, 4, 4>(transform, textPipe.tickPosZ[i]);
		setStringPosition(textPipe.tickLabelZ[i], ssPx[0] / ssPx[3], ssPx[1] / ssPx[3], false, false,true);
	}
}

void setupImgui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.FontGlobalScale = 1.5;
	ImGui::StyleColorsDark();
	core.renderer.exInitImGui();
}


int main() {
	prepareCore();
	setupImgui();

	std::once_flag p1;
	auto startGenTime = std::chrono::steady_clock().now();
	initFont();

	prepareComputePipeline();
	prepareColorRamp();

	prepareVisualization();
	prepareAxisVis();
	prepareTextVis();

	initText();
	
	core.renderer.registerPipelineSubComponents();
	recordCommandBufferAll();
	recordCommandBufferCompAll();

	int currentFrame = 0;
	auto startTime = std::chrono::steady_clock().now();

	core.renderer.setDrawFunction([&]() {


		updateTextPositions();
		updateVisUniform();
		updateTextPipeUniform();

		drawLoop(currentFrame);
		std::call_once(p1, [&]() {

			auto endTime = std::chrono::steady_clock().now();
			auto durationGen = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startGenTime);
			auto durationCalc = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
			ANTH_LOGI("Time elapsed:");
			ANTH_LOGI("w/o Init:", durationCalc.count()," ms");
			ANTH_LOGI("w/ Init:", durationGen.count(), " ms");
			ANTH_LOGI("");
		});
	});
	core.renderer.startDrawLoopDemo();
	core.renderer.exDestroyImgui();
	core.renderer.finalize();

	return 0;
}