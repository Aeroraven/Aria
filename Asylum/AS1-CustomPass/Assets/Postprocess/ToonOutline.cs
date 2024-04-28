using UnityEngine;
using UnityEngine.Rendering;
using UnityEngine.Rendering.HighDefinition;
using System;

[Serializable, VolumeComponentMenu("Post-processing/Custom/ToonOutline")]
public sealed class ToonOutline : CustomPostProcessVolumeComponent, IPostProcessComponent
{
    public BoolParameter enabled = new BoolParameter(false,true);
    public ClampedFloatParameter scale = new ClampedFloatParameter(1f, 1f, 5f);
    public FloatParameter depthTreshold = new FloatParameter(0.5f);
    public FloatParameter normalThreshold = new FloatParameter(0.5f);
    public FloatParameter depthNormalThreshold = new FloatParameter(0.5f);
    public FloatParameter depthNormalThresholdScale = new FloatParameter(1.0f);
    public FloatParameter colorMultiplier = new FloatParameter(0.5f);

    Material m_Material;

    public bool IsActive() => m_Material != null && enabled.value;
    public override CustomPostProcessInjectionPoint injectionPoint => CustomPostProcessInjectionPoint.AfterPostProcess;

    const string kShaderName = "Hidden/Shader/ToonOutline";

    public override void Setup()
    {
        if (Shader.Find(kShaderName) != null)
            m_Material = new Material(Shader.Find(kShaderName));
        else
            Debug.LogError($"Unable to find shader '{kShaderName}'. Post Process Volume ToonOutline is unable to load.");
    }

    public override void Render(CommandBuffer cmd, HDCamera camera, RTHandle source, RTHandle destination)
    {
        if (m_Material == null)
            return;
        m_Material.SetTexture("_MainTex", source);
        m_Material.SetFloat("_Scale", scale.value);
        m_Material.SetFloat("_DepthTreshold", depthTreshold.value);
        m_Material.SetFloat("_NormalThreshold", normalThreshold.value);
        m_Material.SetFloat("_DepthNormalThreshold", depthNormalThreshold.value);
        m_Material.SetFloat("_DepthNormalThresholdScale", depthNormalThresholdScale.value);
        m_Material.SetFloat("_ColorMultiplier", colorMultiplier.value);
        HDUtils.DrawFullScreen(cmd, m_Material, destination, shaderPassId: 0);
    }

    public override void Cleanup()
    {
        CoreUtils.Destroy(m_Material);
    }
}
