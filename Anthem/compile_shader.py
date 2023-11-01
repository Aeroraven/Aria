import os 
import subprocess

if __name__ == "__main__":
    dirs = os.listdir("./shader/glsl/")
    for x in dirs:
        print("Compiling Shader:",x)
        subfiles = os.listdir("./shader/glsl/"+x)
        for y in subfiles:
            if y.endswith(".vert") or y.endswith(".frag") or y.endswith(".geom"):
                subprocess.run(["glslc", "./shader/glsl/"+x+"/"+y, "-o", "./shader/glsl/"+x+"/"+y+".spv"]) 
                print("- Compiled shader:",y)