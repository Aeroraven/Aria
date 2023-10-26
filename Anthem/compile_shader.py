import os 
import subprocess

if __name__ == "__main__":
    dirs = os.listdir("./shader")
    for x in dirs:
        print("Compiling Shader:",x)
        subprocess.run(["glslc", "./shader/"+x+"/shader.frag", "-o", "./shader/"+x+"/shader.frag.spv"]) 
        subprocess.run(["glslc", "./shader/"+x+"/shader.vert", "-o", "./shader/"+x+"/shader.vert.spv"]) 