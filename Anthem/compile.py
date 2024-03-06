import os 
import subprocess
import argparse

if __name__ == "__main__":
    argparser = argparse.ArgumentParser(description="Compile all shaders in the glsl folder")
    argparser.add_argument("--target", help="Compile a specific shader", default=None)
    args = argparser.parse_args()

    dirs = os.listdir("./shader/glsl/")
    for x in dirs:
        if args.target and x != args.target:
            continue
        print("Compiling :",x)
        subfiles = os.listdir("./shader/glsl/"+x)
        for y in subfiles:
            if y.endswith(".vert") or y.endswith(".frag") or y.endswith(".geom") or y.endswith(".comp"):
                w = subprocess.run(["glslc", "./shader/glsl/"+x+"/"+y, "-o", "./shader/glsl/"+x+"/"+y+".spv"]) 
                if w.returncode != 0:
                    print("Error compiling shader:",y)
                    raise
                print("- Compiled :",y)