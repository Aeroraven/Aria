# Aria

This repository is here for **PERSONAL PURPOSE**. It serves as the record for the time I used to learn useless algorithms.



## 0. Clarification

The rectified WebGL ray-tracing is here: https://github.com/Aeroraven/Aria/blob/main/Aria-v2/src/examples/AriaStagePathTracing.ts. This implementation corrects https://github.com/Aeroraven/Ray-tracing.



## I. Running Instruction

### 1. Native Examples

Native examples use Vulkan as backend. Use following instruction to build the project. Note that your compiler should be `g++` or `msvc` and at least supports partial `C++23` standard. `$SELECTED_TARGETS` can be chosen from `demo` subdirectory.

Before build the project, install `glfw3` 、`freetype`、`OpenCV`and `Vulkan SDK`. Then use `cmake` to build the project.

NOTE: Change paths in CMake configuration file before the compilation! (I am too lazy to make them compatible with different envs). Ensure that your device supports `Vulkan 1.3` and `Ray Tracing Extension`

```bash
cd Anthem
cmake -S . -B ./build
cmake --build ./build --config Debug --target $SELECTED_TARGETS
```

Only supports `g++` (14+, `mingw` included), and `msvc` compilers. (Other compilers like `clang` will not be taken into consideration)

Edit `CMakeLists.txt` before changing compiler.

**Note:** Running the code in `WSL` is not recommended!

### 2. Browser-based Examples

Browser-based examples uses WebGL as backend. Install node dependencies before running this project. After installation, run the project.
```bash
cd Aria-v2
npm install --force
npm run deploy
```

(WebGPU might be considered later)

## II. Acknowledgement & License

### Rectification

This repository partially reinplements the WebGL Ray Tracer (https://github.com/Aeroraven/Ray-tracing), which contains fatal mistakes.

### License & Disclaimer

This repository is licensed under AGPL-3.0 License. Referenced models and textures are excluded.

Licenses for referenced models and textures coincide with the license of original repository they belong to, or the license their author determined.


### Acknowledgements & References

Please refer to [acknowledgement.md](./acknowledgement.md) for more details.