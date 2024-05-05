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
        req_suffix = [".vert",".frag",".geom",".comp",".tesc",".tese",".mesh",".task",".rgen",".rmiss",".rchit",".rcall"]

        for y in subfiles:
            for pv in req_suffix:
                if y.endswith(pv) or y.endswith(pv+".hlsl"):
                    hlsl_profile = {
                        "vert.hlsl": "vs_6_1",
                        "frag.hlsl": "ps_6_4",
                        "geom.hlsl": "gs_6_1",
                        "comp.hlsl": "cs_6_1",
                        "tesc.hlsl": "hs_6_1",
                        "tese.hlsl": "ds_6_1",
                        "mesh.hlsl": "ms_6_6",
                        "task.hlsl": "as_6_6",
                        "rgen.hlsl": "lib_6_3",
                        "miss.hlsl": "lib_6_3",
                        "chit.hlsl": "lib_6_3",
                        "call.hlsl": "lib_6_3"
                    }
                    extra_args = {
                        "vert.hlsl": "",
                        "frag.hlsl": "",
                        "geom.hlsl": "",
                        "comp.hlsl": "",
                        "tesc.hlsl": "",
                        "tese.hlsl": "",
                        "mesh.hlsl": "-fspv-target-env=vulkan1.3",
                        "task.hlsl": "-fspv-target-env=vulkan1.3",
                        "rgen.hlsl": "-fspv-target-env=vulkan1.3",
                        "miss.hlsl": "-fspv-target-env=vulkan1.3",
                        "chit.hlsl": "-fspv-target-env=vulkan1.3",
                        "call.hlsl": "-fspv-target-env=vulkan1.3"
                    }
                    w = 1
                    if args.lang == "glsl":
                        w = subprocess.run(["glslc", "./shader/glsl/"+x+"/"+y, "-o", "./shader/glsl/"+x+"/"+y+".spv"]) 
                    elif args.lang == "hlsl":
                        w = subprocess.run(["dxc",
                                            "-spirv",
                                            "-T", hlsl_profile[y[-9:]], 
                                            "./shader/hlsl/"+x+"/"+y, 
                                            extra_args[y[-9:]] , 
                                            '-HV 2021',
                                            '-fspv-extension=SPV_KHR_ray_tracing',
                                            '-fspv-extension=SPV_KHR_multiview',
                                            '-fspv-extension=SPV_KHR_shader_draw_parameters',
                                            '-fspv-extension=SPV_EXT_descriptor_indexing',
                                            '-fspv-extension=SPV_KHR_ray_query',
                                            '-fspv-extension=SPV_KHR_fragment_shading_rate',
                                            "-Fo", "./shader/hlsl/"+x+"/"+y+".spv"])
                    if w.returncode != 0:
                        print("Error compiling shader:",y)
                        raise
                    print("- Compiled :",y)