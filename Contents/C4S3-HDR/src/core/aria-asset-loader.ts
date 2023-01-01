import { loadFile } from "./aria-base";
import { AriaShader } from "./aria-shader";

export class AriaAssetLoader{
    private static instance:AriaAssetLoader;
    private gl:WebGL2RenderingContext
    shaders: Map<string,AriaShader>

    private constructor(gl:WebGL2RenderingContext){
        this.gl = gl
        this.shaders = new Map<string,AriaShader>();
    }
    static async getInstance(gl:WebGL2RenderingContext){
        if(this.instance == null){
            this.instance = new this(gl)
            await this.instance.load()
        }
        return this.instance

    }

    getShader(x:string){
        const el = this.shaders.get(x)
        if(el == null){
            window.alert("No element:"+x)
        }
        return <AriaShader>this.shaders.get(x)
    }

    private async load(){
        const gl = this.gl
        const loadAssert = (x:boolean)=>{
            if(!x){
                window.alert("Error occurred during loading procedure")
            }
        }
        
        const loadShader = async(k:string,v:string,f:string) =>{
            try{
                const insertShader = (key:string,obj:AriaShader) =>{
                    this.shaders.set(key,obj)
                }
                const loadShaderImpl = async (v:string,f:string) =>{
                    const vlsrc = await loadFile(v);
                    const flsrc = await loadFile(f); 
                    const shaderProg = new AriaShader(gl,vlsrc,flsrc);
                    return shaderProg
                }
                insertShader(k,await loadShaderImpl(v,f))
                return true
            }catch(e){
                return false
            }
            
        }
        const shaderLists = [
            {name:"hdrShader",v:"./shaders/hdr/vertex.glsl",f:"./shaders/hdr/fragment.glsl"},
            {name:"skyboxShader",v:"./shaders/skybox/vertex.glsl",f:"./shaders/skybox/fragment.glsl"},
            {name:"africanTangent",v:"./shaders/african-tangent/vertex.glsl",f:"./shaders/african-tangent/fragment.glsl"},
            {name:"postproc",v:"./shaders/postprocess/vertex.glsl",f:"./shaders/postprocess/fragment.glsl"},
            {name:"floorlit",v:"./shaders/floor-lighted/vertex.glsl",f:"./shaders/floor-lighted/fragment.glsl"}
        ]
        for(let el of shaderLists){
            loadAssert(await loadShader(el.name,el.v,el.f)); 
        }
    }
}