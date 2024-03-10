import os 
import subprocess
import argparse

if __name__ == "__main__":
    argparser = argparse.ArgumentParser(description="Compile all shaders in the glsl folder")
    argparser.add_argument("--target", help="Compile a specific shader", default=None)
    argparser.add_argument("--lang", help="Lang", default="glsl")
    args = argparser.parse_args()

    dirs = os.listdir("./shader/"+args.lang+"/")
    for x in dirs:
        if args.target and x != args.target:
            continue
        print("Compiling :",x)
        subfiles = os.listdir("./shader/"+args.lang+"/"+x)
        for y in subfiles:
            if y.endswith(".vert") or y.endswith(".frag") or y.endswith(".geom") or y.endswith(".comp") or \
                y.endswith(".tesc") or y.endswith(".tese") or \
                y.endswith(".vert.hlsl") or y.endswith(".frag.hlsl") or y.endswith(".geom.hlsl") or y.endswith(".comp.hlsl") or \
                y.endswith(".tesc.hlsl") or y.endswith(".tese.hlsl"):
                hlsl_profile = {
                    "vert.hlsl": "vs_6_0",
                    "frag.hlsl": "ps_6_0",
                    "geom.hlsl": "gs_6_0",
                    "comp.hlsl": "cs_6_0",
                    "tesc.hlsl": "hs_6_0",
                    "tese.hlsl": "ds_6_0"
                }
                w = 1
                if args.lang == "glsl":
                    w = subprocess.run(["glslc", "./shader/glsl/"+x+"/"+y, "-o", "./shader/glsl/"+x+"/"+y+".spv"]) 
                elif args.lang == "hlsl":
                    w = subprocess.run(["dxc","-spirv","-T", hlsl_profile[y[-9:]], "./shader/hlsl/"+x+"/"+y, "-Fo", "./shader/hlsl/"+x+"/"+y+".spv"])
                if w.returncode != 0:
                    print("Error compiling shader:",y)
                    raise
                print("- Compiled :",y)