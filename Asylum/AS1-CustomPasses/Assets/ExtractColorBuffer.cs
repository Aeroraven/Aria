using UnityEngine;
using UnityEngine.Rendering.HighDefinition;
using UnityEngine.Rendering;
using System.Reflection;

class CameraDepthBake : CustomPass
{
    public Camera bakingCamera = null;
    public RenderTexture depthTexture = null;
    public RenderTexture normalTexture = null;
    public RenderTexture tangentTexture = null;
    public bool render = true;

    protected override bool executeInSceneView => false;

    protected override void Execute(CustomPassContext ctx)
    {
        if (!render || ctx.hdCamera.camera == bakingCamera || bakingCamera == null || ctx.hdCamera.camera.cameraType == CameraType.SceneView)
            return;

        if (depthTexture == null && normalTexture == null && tangentTexture == null)
            return;

        // We need to be careful about the aspect ratio of render textures when doing the culling, otherwise it could result in objects poping:
        if (depthTexture != null)
            bakingCamera.aspect = Mathf.Max(bakingCamera.aspect, depthTexture.width / (float)depthTexture.height);
        if (normalTexture != null)
            bakingCamera.aspect = Mathf.Max(bakingCamera.aspect, normalTexture.width / (float)normalTexture.height);
        if (tangentTexture != null)
            bakingCamera.aspect = Mathf.Max(bakingCamera.aspect, tangentTexture.width / (float)tangentTexture.height);
        bakingCamera.TryGetCullingParameters(out var cullingParams);
        cullingParams.cullingOptions = CullingOptions.None;

        // Assign the custom culling result to the context
        // so it'll be used for the following operations
        ctx.cullingResults = ctx.renderContext.Cull(ref cullingParams);
        var overrideDepthTest = new RenderStateBlock(RenderStateMask.Depth) { depthState = new DepthState(true, CompareFunction.LessEqual) };

        // Depth
        if (depthTexture != null)
            CustomPassUtils.RenderDepthFromCamera(ctx, bakingCamera, depthTexture, ClearFlag.Depth, bakingCamera.cullingMask, overrideRenderState: overrideDepthTest);

        // Normal
        if (normalTexture != null)
            CustomPassUtils.RenderNormalFromCamera(ctx, bakingCamera, normalTexture, ClearFlag.All, bakingCamera.cullingMask, overrideRenderState: overrideDepthTest);

        // Tangent
        if (tangentTexture != null)
            CustomPassUtils.RenderTangentFromCamera(ctx, bakingCamera, tangentTexture, ClearFlag.All, bakingCamera.cullingMask, overrideRenderState: overrideDepthTest);
    }
}

class ExtractColorBuffer : MonoBehaviour { 

    public RenderTexture colorTexture = null;
    public RenderTexture depthTexture = null;
    public RenderTexture normalTexture = null;
    public bool render = true;

    Camera cameraDepthBake = null;

    CustomPassVolume volume;
    CameraDepthBake depthBakePass;


    void Start()
    {
        var curCamera = GetComponent<Camera>();

        var gameObject = new GameObject("CameraDepthBake");
        cameraDepthBake = gameObject.AddComponent<Camera>();
        cameraDepthBake.CopyFrom(curCamera);
        cameraDepthBake.transform.SetParent(transform);
        cameraDepthBake.targetTexture = colorTexture;

        volume  = gameObject.AddComponent<CustomPassVolume>();
        depthBakePass = volume.AddPassOfType<CameraDepthBake>() as CameraDepthBake;
        volume.hideFlags = HideFlags.HideInInspector | HideFlags.DontSave;
    }

    void Update()
    {
        depthBakePass.depthTexture = depthTexture;
        depthBakePass.normalTexture = normalTexture;
        depthBakePass.bakingCamera = cameraDepthBake;

        Shader.SetGlobalTexture("_NormalMap", normalTexture);
        Shader.SetGlobalTexture("_DepthMap", depthTexture);
    }
}
