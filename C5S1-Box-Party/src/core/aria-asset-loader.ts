import { createTexture, loadFile, loadImage } from "./aria-base";
import { AriaShader } from "./aria-shader";
import { AriaTextureMap } from "./aria-texture-map";

export class AriaAssetLoader{
    private static instance:AriaAssetLoader;
    private gl:WebGL2RenderingContext
    private shaders: Map<string,AriaShader>
    private textures: AriaTextureMap

    private constructor(gl:WebGL2RenderingContext){
        this.gl = gl
        this.shaders = new Map<string,AriaShader>();
        this.textures = new AriaTextureMap()
    }
    static async getInstance(gl:WebGL2RenderingContext){
        if(this.instance == null){
            this.instance = new this(gl)
            await this.instance.load()
        }
        return this.instance

    }
    getTexStruct(){
        return this.textures
    }
    addTexture(name:string,x:WebGLTexture){
        this.textures.set(name,x)
    }
    getTexture(x:string){
        const el = this.textures.get(x)
        if(el == null){
            window.alert("No element:"+x)
        }
        return <AriaTextureMap>this.textures.get(x)
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
        
        //Shaders
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
            {name:"floorlit",v:"./shaders/floor-lighted/vertex.glsl",f:"./shaders/floor-lighted/fragment.glsl"},
            {name:"light",v:"./shaders/light/vertex.glsl",f:"./shaders/light/fragment.glsl"},
            {name:"floor",v:"./shaders/floor/floor-vertex.glsl",f:"./shaders/floor/floor-fragment.glsl"},
            {name:"plaincube",v:"./shaders/plaincube/vertex.glsl",f:"./shaders/plaincube/fragment.glsl"},
            
        ]
        for(let el of shaderLists){
            loadAssert(await loadShader(el.name,el.v,el.f)); 
        }

        //Textures
        const loadTexture = async(k:string,p:string)=>{
            try{
                const im = await loadImage(p)
                const tx = createTexture(gl,im)
                this.textures.set(k,tx)
                return true
            }catch(e){
                return false
            }
        }
        const textureLists = [
            {name:"tex1",path:"./african_diffuse.jpg"},
            {name:"tex2",path:"./african_specular.jpg"},
            {name:"tex3",path:"./star.jpg"},
            {name:"texFloor",path:"./floor.jpg"},
            {name:"texNorm",path:"./normal.jpg"},
            {name:'wall/diffuse',path:"./textures/wall/diffuse.jpg"}
        ]
        for(let el of textureLists){
            loadAssert(await loadTexture(el.name,el.path)); 
        }
    }
}