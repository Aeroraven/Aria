using UnityEngine.Rendering.HighDefinition;
using UnityEngine.Rendering;
using UnityEngine;

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
        //Modified From: https://github.com/alelievr/HDRP-Custom-Passes

        if (!render || ctx.hdCamera.camera == bakingCamera || bakingCamera == null || ctx.hdCamera.camera.cameraType == CameraType.SceneView)
            return;

        if (depthTexture == null && normalTexture == null && tangentTexture == null)
            return;

        if (depthTexture != null)
            bakingCamera.aspect = Mathf.Max(bakingCamera.aspect, depthTexture.width / (float)depthTexture.height);
        if (normalTexture != null)
            bakingCamera.aspect = Mathf.Max(bakingCamera.aspect, normalTexture.width / (float)normalTexture.height);
        if (tangentTexture != null)
            bakingCamera.aspect = Mathf.Max(bakingCamera.aspect, tangentTexture.width / (float)tangentTexture.height);
        bakingCamera.TryGetCullingParameters(out var cullingParams);
        cullingParams.cullingOptions = CullingOptions.None;

        ctx.cullingResults = ctx.renderContext.Cull(ref cullingParams);
        var overrideDepthTest = new RenderStateBlock(RenderStateMask.Depth) { depthState = new DepthState(true, CompareFunction.LessEqual) };

        if (depthTexture != null)
        {
            CustomPassUtils.RenderDepthFromCamera(ctx, bakingCamera, depthTexture, ClearFlag.Depth, bakingCamera.cullingMask, overrideRenderState: overrideDepthTest);
            ctx.cmd.SetGlobalTexture("_DepthMap", depthTexture);
        }

        if (normalTexture != null)
        {
            CustomPassUtils.RenderNormalFromCamera(ctx, bakingCamera, normalTexture, ClearFlag.All, bakingCamera.cullingMask, overrideRenderState: overrideDepthTest);
            ctx.cmd.SetGlobalTexture("_NormalMap", normalTexture);
        }

        if (tangentTexture != null)
            CustomPassUtils.RenderTangentFromCamera(ctx, bakingCamera, tangentTexture, ClearFlag.All, bakingCamera.cullingMask, overrideRenderState: overrideDepthTest);
        Matrix4x4 projMatrix = GL.GetGPUProjectionMatrix(bakingCamera.projectionMatrix, false).inverse;
        ctx.cmd.SetGlobalMatrix("_InvProj", projMatrix);
    }
}

class ReplacementNormalPass : MonoBehaviour 
{
    public bool render = true;

    Camera cameraDepthBake = null;
    Camera referencedCamera = null;

    CustomPassVolume volume;
    CameraDepthBake depthBakePass;
    RenderTexture colorTexture = null;
    RenderTexture depthTexture = null;
    RenderTexture normalTexture = null;

    void SyncCamera()
    {
        cameraDepthBake.CopyFrom(referencedCamera);
        cameraDepthBake.transform.SetParent(transform);
        cameraDepthBake.targetTexture = colorTexture;
    }
    void Start()
    {
        referencedCamera = GetComponent<Camera>();

        var gameObject = new GameObject("CameraDepthBake");
        cameraDepthBake = gameObject.AddComponent<Camera>();
        SyncCamera();

        volume = gameObject.AddComponent<CustomPassVolume>();
        depthBakePass = volume.AddPassOfType<CameraDepthBake>() as CameraDepthBake;
        volume.hideFlags = HideFlags.HideInInspector | HideFlags.DontSave;

        depthTexture = new RenderTexture(referencedCamera.pixelWidth, referencedCamera.pixelHeight, 24, RenderTextureFormat.ARGBFloat);
        normalTexture = new RenderTexture(referencedCamera.pixelWidth, referencedCamera.pixelHeight, 24, RenderTextureFormat.ARGBFloat);
        colorTexture = new RenderTexture(referencedCamera.pixelWidth, referencedCamera.pixelHeight, 24, RenderTextureFormat.ARGBFloat);
        depthTexture.Create();
        normalTexture.Create();
        colorTexture.Create();
    }

    void Update()
    {
        SyncCamera();
        depthBakePass.depthTexture = depthTexture;
        depthBakePass.normalTexture = normalTexture;
        depthBakePass.bakingCamera = cameraDepthBake;


    }
}
