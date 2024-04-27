using System;
using UnityEngine;
using UnityEngine.Rendering;

public class RenderReplacementShaderToTexture : MonoBehaviour
{
    [SerializeField]
    FloatParameter _TestParameter = new FloatParameter(1.0f);

    private RenderTexture rt;
    private void Start()
    {
        
    }

    private void OnGUI()
    {

    }
}