# Aria

This repository is here for **PERSONAL PURPOSE**. It serves as the record for the time I used to learn useless algorithms.



## I. Running Instruction

### 1. Native Examples

Native examples use Vulkan as backend. Use following instruction to build the project. Note that your compiler should be `g++` and at least supports partial `C++20` standard. (Tested on `g++11`). `$SELECTED_TARGETS` can be chosen from `demo` subdirectory.

```bash
cd Anthem
cmake -S . -B ./build
cmake --build ./build --config Debug --target $SELECTED_TARGETS
```

(MSVC support might be considered later, for VS is too large.)

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

### License

This repository is licensed under AGPL-3.0 License, referenced models and textures are excluded.

Licenses for referenced models and textures coincide with the license of original repository they belong to, or the license their author determined.


### Acknowledgements & References

Please refer to [acknowledgement.md](./acknowledgement.md) for more details.