import { vec3,mat4 } from "gl-matrix-ts"
import { AriaComponent } from "../../core/AriaComponent"
import { AriaShaderUniformTp } from "../../core/graphics/AriaShaderOps"
import { AriaObjArray } from "../../core/base/AriaBaseDefs"
import { IAriaShaderEmitter } from "../../core/interface/IAriaShaderEmitter"
import { IAriaInteractive } from "../base/interface/IAriaInteractive"
import { IAriaRenderable } from "../base/interface/IAriaRenderable"
import { IAriaRenderInitiator, IAriaRenderInitiatorOptions } from "../base/interface/IAriaRenderInitiator"
import { IAriaRendererCore } from "../../core/interface/IAriaRendererCore"

export class AriaComCamera extends AriaComponent implements IAriaShaderEmitter, IAriaInteractive, IAriaRenderInitiator{
    camPos:Float32Array
    camFront:Float32Array
    camUp:Float32Array
    camLookAt:Float32Array
    camLookAtCenter:Float32Array
    camPitch:number
    camYaw:number
    winCenterY:number
    winCenterX:number
    winWidth:number
    winHeight:number
    roleStep:number
    winListenerEnable:boolean
    fovAngle:number
    aspectRatio:number

    constructor(){
        super("AriaCom/Camera")
        this.winListenerEnable = true
        this.roleStep=0.15
        this.camPitch = 0
        this.camYaw = 0
        this.winWidth = window.innerWidth
        this.winHeight = window.innerHeight
        this.winCenterX = window.innerWidth/2
        this.winCenterY = window.innerHeight/2

        this.camPos = vec3.create()
        this.camFront = vec3.create()
        this.camUp = vec3.create()
        this.camLookAt = mat4.create()
        this.camLookAtCenter = vec3.create()

        this.camPos[0] = 0.0
        this.camPos[1] = 0.0
        this.camPos[2] = 10.0
        
        this.camFront[0] = 0.0
        this.camFront[1] = 0.0
        this.camFront[2] = -1.0

        this.camUp[0] = 0.0
        this.camUp[1] = 1.0
        this.camUp[2] = 0.0

        this.fovAngle = 45.0
        this.aspectRatio = window.innerWidth / window.innerHeight;

        vec3.add(this.camLookAtCenter,this.camFront,this.camPos)
        mat4.lookAt(this.camLookAt,this.camPos,this.camLookAtCenter,this.camUp)
    }
    initiateRender(renderables: AriaObjArray<IAriaRenderable<void>>, options: IAriaRenderInitiatorOptions={}): void {
        this._logError("aria_com_camera: function deprecated")
        const injectRenderTriggers = [
            ()=>{
                //this.exportToShader()
            }
        ]
    }

    
    exportToShader(renderer:IAriaRendererCore): void {
        //Computation
        const modelview = this.getLookAt()
        const projectionMatrix = this.getPerspective()
        const viewportOrtho = this.getViewportOrtho()
        const shadowOrtho = this.getShadowOrtho()
        const modelIT2 = mat4.create()
        const modelIT = mat4.create()
        const model3 = this.getLookAt3()
        mat4.invert(modelIT2,modelview)
        mat4.transpose(modelIT,modelIT2)

        //Emitter
        renderer.defineUniform("uModelView", AriaShaderUniformTp.ASU_MAT4, modelview)
        renderer.defineUniform("uModel3", AriaShaderUniformTp.ASU_MAT4, model3)
        renderer.defineUniform("uProj", AriaShaderUniformTp.ASU_MAT4, projectionMatrix)
        renderer.defineUniform("uViewOrtho", AriaShaderUniformTp.ASU_MAT4, viewportOrtho)
        renderer.defineUniform("uShadowOrtho", AriaShaderUniformTp.ASU_MAT4, shadowOrtho)
        renderer.defineUniform("uModelIT", AriaShaderUniformTp.ASU_MAT4, modelIT)
        renderer.defineUniform("uCamPos", AriaShaderUniformTp.ASU_VEC3, this.getCamPosArray())
        renderer.defineUniform("uCamFront", AriaShaderUniformTp.ASU_VEC3, this.getCamFrontArray())
        renderer.defineUniform("uCamAspect", AriaShaderUniformTp.ASU_VEC1, this.getAspect())
        renderer.defineUniform("uScrWidth", AriaShaderUniformTp.ASU_VEC1, window.innerWidth)
        renderer.defineUniform("uScrHeight", AriaShaderUniformTp.ASU_VEC1, window.innerHeight)

        //Framebuffer Hooks
        renderer.setCameraPos(this.camPos[0],this.camPos[1],this.camPos[2])
    }
    disableInteraction(){
        this.winListenerEnable = false
    }
    getViewportOrtho(){
        const proj = mat4.create()
        mat4.ortho(proj,-1,1,-1,1,0.001,400)
        return proj
    }
    getShadowOrtho(){
        const proj = mat4.create()
        mat4.ortho(proj,-25,25,-25,25,0.001,400)
        return proj
    }
    getPerspective(){
        const fov = this.fovAngle / 180 * Math.PI;
        const aspect = 1.0 * this.aspectRatio
        const zNear = 0.01;
        const zFar = 450;
        const projectionMatrix = mat4.create()
        mat4.perspective(projectionMatrix,fov,aspect,zNear,zFar)
        return projectionMatrix
    }
    getAspect(){
        return this.aspectRatio
    }
    getCamPosArray(){
        return [this.camPos[0],this.camPos[1],this.camPos[2]]
    }
    getCamFrontArray(){
        return [this.camFront[0],this.camFront[1],this.camFront[2]]
    }
    getLookAt():Float32Array{
        return this.camLookAt
    }
    getLookAt3():Float32Array{
        const r = mat4.create()
        mat4.copy(r,this.camLookAt)
        r[3] = 0
        r[7] = 0
        r[11] = 0
        r[12] = 0
        r[13] = 0
        r[14] = 0
        r[15] = 1
        return r
    }
    setRoleStep(step:number){
        this.roleStep = step
    }
    setFov(fov:number){
        this.fovAngle = fov
    }
    setAspect(aspect:number){
        this.aspectRatio = aspect
    }
    setPos(dx:number,dy:number,dz:number){
        this.camPos[0] = dx
        this.camPos[1] = dy
        this.camPos[2] = dz
        this.movePos(0,0,0)
    }
    movePos(dx:number,dy:number,dz:number){
        this.camPos[0] += dx;
        this.camPos[1] += dy;
        this.camPos[2] += dz;
        vec3.add(this.camLookAtCenter,this.camFront,this.camPos)
        mat4.lookAt(this.camLookAt,this.camPos,this.camLookAtCenter,this.camUp)
    }
    setLookatCenter(dx:number,dy:number,dz:number){
        this.camLookAtCenter[0] = dx;
        this.camLookAtCenter[1] = dy;
        this.camLookAtCenter[2] = dz;
        vec3.sub(this.camFront,this.camLookAtCenter,this.camPos)
        mat4.lookAt(this.camLookAt,this.camPos,this.camLookAtCenter,this.camUp)
    }
    initInteraction(){
        window.addEventListener("mousemove",(e:MouseEvent)=>{
            if(this.winListenerEnable){
                let mx = e.x
                let my = e.y
                this.camPitch = (-my/this.winHeight+0.5)*3.1415926
                this.camYaw = (mx/this.winWidth+1)*3.1415926
                this.camFront[0] = Math.cos(this.camPitch) * Math.cos(this.camYaw)
                this.camFront[1] = Math.sin(this.camPitch)
                this.camFront[2] = Math.cos(this.camPitch) * Math.sin(this.camYaw)
                vec3.normalize(this.camFront,this.camFront)
                this.movePos(0,0,0)
            }
        })
        window.addEventListener("keydown",(e:KeyboardEvent)=>{
            if(this.winListenerEnable){
                if(e.key.toLowerCase() == "d"){
                    let lf = vec3.create()
                    vec3.cross(lf,this.camFront,this.camUp)
                    this.movePos(lf[0]*this.roleStep,lf[1]*this.roleStep,lf[2]*this.roleStep)
                }
                if(e.key.toLowerCase() == "a"){
                    let lf = vec3.create()
                    vec3.cross(lf,this.camFront,this.camUp)
                    this.movePos(-lf[0]*this.roleStep,-lf[1]*this.roleStep,-lf[2]*this.roleStep)
                }
                if(e.key.toLowerCase() == "w"){
                    this.movePos(this.camFront[0]*this.roleStep,this.camFront[1]*this.roleStep,this.camFront[2]*this.roleStep)
                }
                if(e.key.toLowerCase() == "s"){
                    this.movePos(-this.camFront[0]*this.roleStep,-this.camFront[1]*this.roleStep,-this.camFront[2]*this.roleStep)
                }
                if(e.key.toLowerCase() == "q"){
                    this.movePos(this.camUp[0]*this.roleStep,this.camUp[1]*this.roleStep,this.camUp[2]*this.roleStep)
                }
                if(e.key.toLowerCase() == "e"){
                    this.movePos(-this.camUp[0]*this.roleStep,-this.camUp[1]*this.roleStep,-this.camUp[2]*this.roleStep)
                }
            }
            if(e.key.toLowerCase() == "escape"){
                this.winListenerEnable = !this.winListenerEnable
            }
        })
    }
}