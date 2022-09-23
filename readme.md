## Aria

Visualize some stuffs using native WebGL. (Or learning TypeScript ?)





### Main Dependencies

Nodejs, Webpack, Axios, TypeScript(ts-loader)，gl-matrix-ts, webgl-gltf





### How to Run

Install a browser which supports WebGL2

Execute the command `npm run deploy` to establish the debug server.

Execute the command `npm run build` to build the production bundle.





### Available Stages

Open the browser with url param `stage` to choose stages

Available stages are `gltf`, `fog`, `value_noise`, `perlin_noise`, `volume_render`, `fxaa`, `gerstner_wave`, `vol_light`, `ssao`, `ssr`

1. `fog`: Foggy Scene (C1S1 - C5S5)
2. `value_noise`: Shader noise generated using pseudo random function and fractal mixing (C6S1)
3. `perlin_noise`: Perlin noise (C6S1)
4. `gerstner_wave`: A gerstner wave (C6S2)
5. `volume_render`: Volume rendered bonsai (C6S3)
6. `fxaa`: A FXAA implementation (C6S4)
7. `gltf`: Displaying a GLTF model (C6S5)
8. `vol_light`: A simple volumetric lighted scene (C6S6)
9. `ssao`: Screen space ambient occlusion (C6S7)
10. `ssr`: Screen space reflection (C6S8)
11. `ts-outline`: Outline shader (C6S9)
11. `waterwave`: Refraction under the circumstance of spherical wave, without depth difference (C6S10)



For example: `http://localhost:1551/?stage=volume_render`

All scenes have been implemented in non-optimized version. Do not run on devices without proper GPU supports.





### License

The license coincides with the repository to which this project belongs





### Material Referenced / Acknowledgements

**Wavefront African Model \(Incl. Specular, Diffuse, Normal Texture\)**  : By Dmitry V. Sokolov. In Project "TinyRenderer" https://github.com/ssloy/tinyrenderer

**Bonzai Volume Texture**: By Leandro Barbagallo. In Project "WebGLVolumeRendering" https://github.com/lebarba/WebGLVolumeRendering/

**Skybox Texture**: By Joey de Vries. In Project "LearnOpenGL" https://learnopengl.com/

**Wood PBR Texture:** https://freepbr.com/wp-content/uploads/bl/bamboo-wood-semigloss-bl.zip

**Klee GLTF Model**: By INSTICT. In Creation "Klee FBX", Converted by Sktechfab, Re-exported using Blender. https://sketchfab.com/3d-models/klee-fbx-eb4ba3c0d92349cfaf7361e4e55604f0





### Tutorials Referenced / Acknowledgements

https://learnopengl.com/

https://github.com/lebarba/WebGLVolumeRendering/

https://zhuanlan.zhihu.com/p/431384101

https://zhuanlan.zhihu.com/p/404778222

http://www.alexandre-pestana.com/volumetric-lights/

https://zhuanlan.zhihu.com/p/415500177
