import { createTexture, loadFile, loadImage } from "./aria-base";
import { AriaPageIndicator } from "./aria-page-indicator";
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
                    const vlsrc = await loadFile(v,(p:number)=>{
                        AriaPageIndicator.getInstance().updateLoadingProgress(p/2)
                    });
                    const flsrc = await loadFile(f,(p:number)=>{
                        AriaPageIndicator.getInstance().updateLoadingProgress(p/2 + 50)
                    }); 
                    const shaderProg = new AriaShader(gl,vlsrc,flsrc);
                    return shaderProg
                }
                insertShader(k,await loadShaderImpl(v,f))
                return true
            }catch(e){
                console.log(e)
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
            {name:"plaincube-pbr",v:"./shaders/plaincube-pbr/vertex.glsl",f:"./shaders/plaincube-pbr/fragment.glsl"},
            {name:"plaincube-pbr-t",v:"./shaders/plaincube-pbr-textured/vertex.glsl",f:"./shaders/plaincube-pbr-textured/fragment.glsl"},
            {name:"bloom-thresh",v:"./shaders/bloom-thresholding/vertex.glsl",f:"./shaders/bloom-thresholding/fragment.glsl"},
            {name:"gaussian-horizontal",v:"./shaders/gaussian-horizontal/vertex.glsl",f:"./shaders/gaussian-horizontal/fragment.glsl"},
            {name:"gaussian-vertical",v:"./shaders/gaussian-vertical/vertex.glsl",f:"./shaders/gaussian-vertical/fragment.glsl"},
            {name:"bloom-blend",v:"./shaders/bloom-blending/vertex.glsl",f:"./shaders/bloom-blending/fragment.glsl"},
            {name:"shadow-dir-depth",v:"./shaders/shadow-dir-depth/vertex.glsl",f:"./shaders/shadow-dir-depth/fragment.glsl"},
            {name:"post-white-noise",v:"./shaders/post-white-noise/vertex.glsl",f:"./shaders/post-white-noise/fragment.glsl"},
            {name:"post-value-noise",v:"./shaders/post-value-noise/vertex.glsl",f:"./shaders/post-value-noise/fragment.glsl"},
            {name:"post-perlin-noise",v:"./shaders/post-perlin-noise/vertex.glsl",f:"./shaders/post-perlin-noise/fragment.glsl"},
            {name:"post-perlin-noise-3d",v:"./shaders/post-perlin-noise-3d/vertex.glsl",f:"./shaders/post-perlin-noise-3d/fragment.glsl"},
            {name:"sine-wave",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"gerstner-wave",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"volume-render/first",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"volume-render/second",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"fxaa/scene",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"fxaa/post",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee/scene",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee/shadow",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee/floor",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee/debug",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee/fogf",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee/fogs",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee/depth",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee/vlight",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"fxaa-all",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee/mix",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-ssao/depth",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-ssao/normal",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-ssao/occl",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-ssao/floor",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-ssao/scene",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-ssao/postmix",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-ssr/coarsepos",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-ssr/reflpos",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-ssr/reflection",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-ssr/postmix",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
            {name:"klee-toon/scene-outline",v:"./shaders/%name%/vertex.glsl",f:"./shaders/%name%/fragment.glsl"},
        ]

        for(let el of shaderLists){
            AriaPageIndicator.getInstance().updateLoadingTip("Resources/Shader/"+el.name)
            AriaPageIndicator.getInstance().updateLoadingProgress(0)
            loadAssert(await loadShader(el.name,el.v.replace(/\%name\%/g,el.name),el.f.replace(/\%name\%/g,el.name))); 
        }

        //Textures
        const loadTexture = async(k:string,p:string)=>{
            try{
                const im = await loadImage(p,(p:number)=>{
                    AriaPageIndicator.getInstance().updateLoadingProgress(p)
                })
                const tx = createTexture(gl,im)
                this.textures.set(k,tx)
                return true
            }catch(e){
                console.log(e)
                return false
            }
        }
        const textureLists = [
            {name:"tex1",path:"./textures/african/african_diffuse.jpg"},
            {name:"tex2",path:"./textures/african/african_specular.jpg"},
            {name:"tex3",path:"./textures/star/star.jpg"},
            {name:"texFloor",path:"./textures/star/floor.jpg"},
            {name:"texNorm",path:"./textures/african/normal.jpg"},
            {name:'wall/diffuse',path:"./textures/wall/diffuse.jpg"},
            {name:'wood/albedo',path:"./textures/wood/albedo.png"},
            {name:'wood/ao',path:"./textures/wood/ao.png"},
            {name:'wood/metal',path:"./textures/wood/metal.png"},
            {name:'wood/normal',path:"./textures/wood/normal.png"},
            {name:'wood/rough',path:"./textures/wood/roughness.png"},
            {name:'bonsai/volume',path:"./textures/bonsai/bonsai.raw.png"},
        ]
        for(let el of textureLists){
            AriaPageIndicator.getInstance().updateLoadingTip("Resources/Texture/"+el.name)
            AriaPageIndicator.getInstance().updateLoadingProgress(0)
            loadAssert(await loadTexture(el.name,el.path)); 
        }
    }
}