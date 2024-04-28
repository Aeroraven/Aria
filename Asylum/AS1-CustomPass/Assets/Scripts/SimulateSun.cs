using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SimulateSun : MonoBehaviour
{
    public Light sun;

    void ExposeDirection()
    {
        if (sun == null) return;
        var d = sun.transform.rotation;
        var p = new Vector3(0, 0, 1);
        var e = d * p;
        Shader.SetGlobalVector("_SunDirection", e);
    }
    void Start()
    {
        ExposeDirection();
    }

    void Update()
    {
        ExposeDirection();
    }
}
