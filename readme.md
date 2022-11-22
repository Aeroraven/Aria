# Aria

This repository is here for **personal purpose**. It serves as the record for the time I used to learn graphical algorithms, or you can say more materialistically, web techniques.

None of third party libraries which directly encapsulate WebGL interfaces are adopted.

None of reactive front-end frameworks, like react or angular, are used.



<!--## Foreword

After years chasing so-called wealth and fame, I think I have gone astray in past few years when I ignored my original oath for myself. We were taught to be subject to requirements, and were called for competition. 

The art of the computer science lies in its freedom, instead of "requirements" and "endless documents". When I retracing my earlier life, I remember that I tried to re-implementing a Minecraft, tried to find simple cross-site leaks of some educational websites. I still remember that a guest teacher attended the OI lecture and said "Love for computer originates from electronic games". This moment impressed me, for it conveyed that "freedom and curiosity are the things adolescence might chase"

It is a shame that some education settings are too outmoded. Take my undergraduate university as an example, it's terrible that teachers asked students to make game in ugly commandline (console), or focus more on documents instead of practicing. It's weird to write mobile apps, backend apps again and again. Only few of lectures deeply attracted me.

This code repository might serve as an observer. It might prove that, as time passes, occupation might be subject to the reality, research tendency might be subject to the worldly pressure, traditional laws might be outdated,  but our heart will stand.
-->


## All Demos

Here are all demos implemented in this repository



### Implemented in Unstructured WGL (Ch1-Ch5)

These sections contains the basic part and are all implemented in non-object-oriented WebGL

- **Chapter 1: View Port, Projection and Another Space**
  - Section 1: Camera
- **Chapter 2: Basic Lighting**
  - Section 1: Basic Lighting
  - Section 2: Loading Wavefront Object
  - Section 3: Light Mapping
  - Section 4: Spotlight
- **Chapter 3: Advanced Buffers and Environmental Lighting **
  - Section 1: Depth Shader
  - Section 2: Outline Drawing Using Stencil Test
  - Section 3: Convolution & Blur
  - Section 4: Sky box & Environmental Lighting
  - Section 5: Instancing
- **Chapter 4: Simulation of Better Lighting I**:
  - Section 1: Blinn-Phong Shading
  - Section 2: Normal Mapping
  - Section 3: HDR
- **Chapter 5: Simulation of Better Lighting  II**
  - Section 1: Reviewing Techniques
  - Section 2: PBR
  - Section 3: Bloom Postprocessing
  - Section 4: Shadow Map for Directional Lights & PCF
  - Section 5: PBR Using Textures   



### Implemented in V1 (Ch6)

This part is implemented in partially-encapsulated WebGL

- **Chapter 6: Draw with Freedom**

  - **Part 1: Randomness and Noise**

    - Section 1: Perlin Noise

  - **Part 2: Ray-tracing & Volumetric Rendering**

    - Section 3: Volumetric Rendering
    - Section 6: Volumetric Lighting
    - Section 8: Screen Space Reflection (SSR)

  - **Part 3: Deferred Rendering**

    - Section 7: Screen Space Ambient Occlusion (SSAO)

  - **Part 4: Anti Aliasing**

    - Section 4: Fast Approximation Anti Aliasing (FXAA)

  - **Part 5: Fluid Rendering**

    - Section 2: Gerstner Wave
    - Section 10: Refraction of a Spherical Wave
    - Section 12: Numerical Simulation of a Wave
    - Section 13: Liquid Refraction
    - Section 14: Water Caustics

  - **Part 6: Towards Diverse World**

    - Section 5: Rendering GLTF Model
    - Section 9: Toon Rendering / Drawing Outline
    - Section 11:  Toon Rendering / Light Ramping
    - Section 15: Drawing a Sphere

    

### Implemented in V2 (Ch7)

The architecture is refactored.

- **Chapter 7: Draw for Liberty**
  - **Part 1: Anti Aliasing II**
    - Section 1: Morphological Anti-aliasing (MLAA)



## Instructions (v2) 

### How to Run

Install a browser which supports WebGL2

Execute the command `npm run deploy` to establish the debug server.

Execute the command `npm run build` to build the production bundle.



### Available Stages 

It will launch `MLAA` Demo





## Instructions (v1) 

### Main Dependencies

Nodejs, Webpack, Axios, TypeScript(ts-loader)，gl-matrix-ts, webgl-gltf



### How to Run

Install a browser which supports WebGL2

Execute the command `npm run deploy` to establish the debug server.

Execute the command `npm run build` to build the production bundle.





### Available Stages 

Open the browser with url param `stage` to choose stages

Available stages are：

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
11. `ts-outline`: Toon shader (C6S9, C6S11)
    - C6S9: Outline depiction using normal vector
    - C6S11: Binary light ramping
12. `waterwave`: Refraction under the circumstance of spherical wave, without depth difference (C6S10)
13. `fluid`: Fluid simulation (C6S12 - C6S14)  (In progress & with flaws)
    - C6S12: Numerical simulation of disturbed fluid surface
    - C6S13: Ray-marching based refraction simulation
    - C6S14: Water caustics



For example: `http://localhost:1551/?stage=volume_render`

All scenes have been implemented in non-optimized version. Do not run on devices without proper GPU supports.



## Acknowledgement & License

### Future Plans

I might change the engine this repository depends on to some I listed in my personal introduction.



### License

The license coincides with the repository to which this project belongs.



### Materials Referenced / Acknowledgements

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

https://blog.csdn.net/zju_fish1996/article/details/52293782

https://madebyevan.com/webgl-water/
