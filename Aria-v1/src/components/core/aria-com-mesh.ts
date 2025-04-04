import { mat4 } from "gl-matrix-ts";
import { AriaCamera } from "../../core/aria-camera";
import { AriaShader } from "../../core/aria-shader";
import { AriaComponent } from "../base/aria-component";
import { AriaComLightSet } from "../light/aria-com-light-set";
import { AriaComBuffers } from "./aria-com-buffers";
import { AriaComTexture } from "./aria-com-texture";
import { IAriaComFrameUpdateable } from "./interfaces/aria-com-frame-updateable";
import { IAriaComLight } from "./interfaces/aria-com-light";
import { IAriaComMaterial } from "./interfaces/aria-com-material";
import { IAriaComShaderEmitter } from "./interfaces/aria-com-shader-emitter";
import { IAriaComShadowBaker } from "./interfaces/aria-com-shadow-baker";



export interface IAriaRenderable{
    render():void
}

export enum AriaComMeshTextureType{
    acmtDiffuse = "texDiffuse",
    acmtSpecular = "texSpecular",
    acmtNormal = "texNormal",
    acmtAlbedo = "texAlbedo"
}


interface IAriaComMeshTextureTypeMapping{
    k:AriaComMeshTextureType
    u:string
}

export class AriaComMesh extends AriaComponent implements IAriaRenderable,IAriaComShadowBaker{
    camera:AriaCamera
    shader:AriaShader
    depthShader:AriaShader
    texUniformMaps:IAriaComMeshTextureTypeMapping[]
    texUniformOrds:number[]
    reservedKeys:string[]
    instances:number
    lightShadowPos:number[]

    constructor(gl:WebGL2RenderingContext){
        super(gl)
        this.instances = 1
        this.lightShadowPos = [0,9,0]
        this.camera = new AriaCamera()
        this.shader = new AriaShader(gl,"","")
        this.depthShader = new AriaShader(gl,"","")
        this.texUniformMaps = [
            {k:AriaComMeshTextureType.acmtDiffuse,u:"uDiffuse"},
            {k:AriaComMeshTextureType.acmtSpecular,u:"uSpecular"},
            {k:AriaComMeshTextureType.acmtNormal,u:"uNormal"},
            {k:AriaComMeshTextureType.acmtAlbedo,u:"uAlbedo"}
        ]
        this.texUniformOrds = [
            gl.TEXTURE0,
            gl.TEXTURE1,
            gl.TEXTURE2,
            gl.TEXTURE3
        ]
        this.reservedKeys = ["light","buffer","material"]
    }
    renderLightDepthMapS(id: number): void {
        this.renderLightDepthMap((<AriaComLightSet><unknown>this.getChild("light")),id)
    }
    renderLightDepthMap(x: AriaComponent & IAriaComLight, id: number): void {
        if(this.childExist("light")==false){
            throw new Error("Light does not exist")
        }
        const pushedCamera = this.camera
        const pushedShader = this.shader
        const lightCamera = new AriaCamera()
        this.camera = lightCamera
        this.shader = this.depthShader
        lightCamera.camFront = new Float32Array((<AriaComLightSet><unknown>x).lightPos[id])
        lightCamera.camPos = new Float32Array(this.lightShadowPos)
        if(lightCamera.camFront[0]==0&&lightCamera.camFront[2]==0){
            lightCamera.camUp = new Float32Array([0,0,1])
        }
        lightCamera.movePos(0,0,0)
        this.render()
        this.camera = pushedCamera
        this.shader = pushedShader

    }
    setLightProjPos(x:number[]){
        this.lightShadowPos = x
        return this
    }
    setCamera(o:AriaCamera){
        this.camera = o
        return this
    }

    setShader(s:AriaShader){
        this.shader = s
        return this
    }

    setDepthShader(s:AriaShader){
        this.depthShader = s
        return this
    }

    setBuffer(b:AriaComponent){
        this.addComponent("buffer",b)
        return this
    }

    setTexture(tp:AriaComMeshTextureType,o:AriaComponent){
        this.addComponent(tp,o)
        return this
    }

    setMaterial(o: AriaComponent & IAriaComMaterial){
        this.addComponent("material",o)
        return this
    }

    setLight(o: AriaComponent & IAriaComLight){
        this.addComponent("light",o)
        return this
    }
    setNumInstances(x:number){
        this.instances = x
        return this
    }
    addAttachments(n:string, o:IAriaComShaderEmitter & AriaComponent){
        this.addComponent(n,o)
        return this
    }
    render(){
        const gl = this.gl
        const modelview = this.camera.getLookAt()
        const projectionMatrix = this.camera.getPerspective()
        const viewportOrtho = this.camera.getViewportOrtho()
        const shadowOrtho = this.camera.getShadowOrtho()
        const buf : AriaComBuffers = <AriaComBuffers>this.getChild("buffer")
        const modelIT2 = mat4.create()
        const modelIT = mat4.create()
        const model3 = this.camera.getLookAt3()
        mat4.invert(modelIT2,modelview)
        mat4.transpose(modelIT,modelIT2)

        //Shader
        this.shader.use()

        //Attachments
        const inExclList = (key:string) =>{
            this.reservedKeys.forEach((ls)=>{
                if(ls==key){
                    return true
                }
            })
            return false
        }
        this.components.forEach((value,keys) => {
            if("emitUniforms" in value && !inExclList(keys) ){
                (<IAriaComShaderEmitter><unknown>value).emitUniforms(this.shader)
            }
            if("frameUpdate" in value){
                (<IAriaComFrameUpdateable><unknown>value).frameUpdate()
            }
        });

        //Attrib
        gl.bindBuffer(gl.ARRAY_BUFFER, buf.getBuffer().get("pos"));
        gl.vertexAttribPointer(this.shader.getAttr("aVert"),buf.getBuffer().getSize("pos") || 3, buf.getBuffer().getType("pos") ||gl.FLOAT,false,0,0)
        gl.enableVertexAttribArray(this.shader.getAttr("aVert"))

        if(buf.getBuffer().exists("tex")){
            gl.bindBuffer(gl.ARRAY_BUFFER, buf.getBuffer().get("tex"));
            let id = this.shader.getAttr("aTex");
            if(id>=0){
                gl.vertexAttribPointer(this.shader.getAttr("aTex"),buf.getBuffer().getSize("tex") || 2,buf.getBuffer().getType("tex") || gl.FLOAT,false,0,0)
                gl.enableVertexAttribArray(this.shader.getAttr("aTex"))
            }
        }

        if(buf.getBuffer().exists("norm")){
            
            gl.bindBuffer(gl.ARRAY_BUFFER, buf.getBuffer().get("norm"));
            let id = this.shader.getAttr("aNorm");
            if(id>=0){
                gl.vertexAttribPointer(this.shader.getAttr("aNorm"),buf.getBuffer().getSize("norm") || 3,buf.getBuffer().getType("norm") || gl.FLOAT,false,0,0)
                gl.enableVertexAttribArray(this.shader.getAttr("aNorm"))
            }
            
        }

        if(buf.getBuffer().exists("tan")){
            gl.bindBuffer(gl.ARRAY_BUFFER, buf.getBuffer().get("tan"));
            let id = this.shader.getAttr("aTangent");
            if(id>=0){
                gl.vertexAttribPointer(this.shader.getAttr("aTangent"),3,gl.FLOAT,false,0,0)
                gl.enableVertexAttribArray(this.shader.getAttr("aTangent"))
            }
        }

        if(buf.getBuffer().exists("bitan")){
            gl.bindBuffer(gl.ARRAY_BUFFER, buf.getBuffer().get("bitan"));
            let id = this.shader.getAttr("aBitangent");
            if(id>=0){
                gl.vertexAttribPointer(this.shader.getAttr("aBitangent"),3,gl.FLOAT,false,0,0)
                gl.enableVertexAttribArray(this.shader.getAttr("aBitangent"))
            }
        }

        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,buf.getBuffer().get("ele"))

        gl.uniformMatrix4fv(this.shader.getUniform("uModel"),false,modelview)
        gl.uniformMatrix4fv(this.shader.getUniform("uModel3"),false,model3)
        gl.uniformMatrix4fv(this.shader.getUniform("uProj"),false,projectionMatrix)
        gl.uniformMatrix4fv(this.shader.getUniform("uViewOrtho"),false,viewportOrtho)
        gl.uniformMatrix4fv(this.shader.getUniform("uShadowOrtho"),false,shadowOrtho)
        gl.uniformMatrix4fv(this.shader.getUniform("uModelIT"),false,modelIT)
        gl.uniform3fv(this.shader.getUniform("uCamPos"),this.camera.getCamPosArray())
        gl.uniform3fv(this.shader.getUniform("uCamFront"),this.camera.getCamFrontArray())
        gl.uniform1f(this.shader.getUniform("uCamAspect"),this.camera.getAspect())
        gl.uniform1f(this.shader.getUniform("uScrWidth"),window.innerWidth)
        gl.uniform1f(this.shader.getUniform("uScrHeight"),window.innerHeight)

        

        //Texture
        for(let i=0;i<this.texUniformMaps.length;i++){
            if(this.childExist(this.texUniformMaps[i].k)){
                let tid = this.shader.getTexId()
                gl.activeTexture(gl.TEXTURE0 + tid)
                gl.bindTexture(gl.TEXTURE_2D, (<AriaComTexture>this.getChild(this.texUniformMaps[i].k)).getTex())
                gl.uniform1i(this.shader.getUniform(this.texUniformMaps[i].u),tid);
            }
        }
        
        //Material
        if(this.childExist("material")){
            (<IAriaComMaterial><unknown>this.getChild("material")).emitUniforms(this.shader)
        }

        //Light
        if(this.childExist("light")){
            (<IAriaComLight><unknown>this.getChild("light")).emitUniforms(this.shader)
        }

        //gl.drawElements(gl.TRIANGLES,buf.getBuffer().numVertices,gl.UNSIGNED_SHORT,0)
        gl.drawElementsInstanced(gl.TRIANGLES,buf.getBuffer().numVertices,gl.UNSIGNED_SHORT,0,this.instances)
    
    }
}
