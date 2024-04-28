using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ToonShadingController : MonoBehaviour
{
    public Color baseColor = Color.white;
    public Color shadowColor = new Color(238.0f/255.0f, 193.0f / 255.0f, 138.0f / 255.0f);
    public Color transitionColor = new Color(252.0f / 255.0f, 248.0f / 255.0f, 134.0f / 255.0f);
    public float colorRamp1Start = 0.09f;
    public float colorRamp1End = 0.11f;
    public float colorRamp2Start = 0.15f;
    public float colorRamp2End = 0.17f;

    void ToonSync()
    {
        Shader.SetGlobalColor("_ToonBaseColor", baseColor);
        Shader.SetGlobalColor("_ToonShadowColor", shadowColor);
        Shader.SetGlobalColor("_ToonTransitionColor", transitionColor);
        Shader.SetGlobalFloat("_ToonColorRamp1Start", colorRamp1Start);
        Shader.SetGlobalFloat("_ToonColorRamp1End", colorRamp1End);
        Shader.SetGlobalFloat("_ToonColorRamp2Start", colorRamp2Start);
        Shader.SetGlobalFloat("_ToonColorRamp2End", colorRamp2End);

    }
    void Start()
    {
        ToonSync();
    }

    void Update()
    {
        ToonSync();
    }

    private void OnGUI()
    {
        ToonSync();
    }
}
