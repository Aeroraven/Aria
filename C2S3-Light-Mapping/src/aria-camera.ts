import { vec3,mat4 } from "../node_modules/gl-matrix-ts/dist/index"

export class AriaCamera{
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

    constructor(){
        this.roleStep=0.05
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

        this.camPos[0] = 2.0
        this.camPos[1] = 2.0
        this.camPos[2] = 10.0
        
        this.camFront[0] = 0.0
        this.camFront[1] = 0.0
        this.camFront[2] = 1.0

        this.camUp[0] = 0.0
        this.camUp[1] = 1.0
        this.camUp[2] = 0.0

        vec3.add(this.camLookAtCenter,this.camFront,this.camPos)
        mat4.lookAt(this.camLookAt,this.camPos,this.camLookAtCenter,this.camUp)
    }
    getPerspective(){
        const fov = 45.0 / 180 * Math.PI;
        const aspect = 1.0 * window.innerWidth / window.innerHeight;
        const zNear = 1.0;
        const zFar = 100;
        const projectionMatrix = mat4.create()
        mat4.perspective(projectionMatrix,fov,aspect,zNear,zFar)
        return projectionMatrix
    }
    getCamPosArray(){
        return [this.camPos[0],this.camPos[1],this.camPos[2]]
    }
    getLookAt():Float32Array{
        return this.camLookAt
    }
    movePos(dx:number,dy:number,dz:number){
        this.camPos[0] += dx;
        this.camPos[1] += dy;
        this.camPos[2] += dz;
        vec3.add(this.camLookAtCenter,this.camFront,this.camPos)
        mat4.lookAt(this.camLookAt,this.camPos,this.camLookAtCenter,this.camUp)
    }
    registerInteractionEvent(){
        window.addEventListener("mousemove",(e:MouseEvent)=>{
            let mx = e.x
            let my = e.y
            this.camPitch = (my/this.winHeight-0.5)*3.1415926
            this.camYaw = (mx/this.winWidth+1)*3.1415926
            this.camFront[0] = Math.cos(this.camPitch) * Math.cos(this.camYaw)
            this.camFront[1] = Math.sin(this.camPitch)
            this.camFront[2] = Math.cos(this.camPitch) * Math.sin(this.camYaw)
            vec3.normalize(this.camFront,this.camFront)
            this.movePos(0,0,0)
        })
        window.addEventListener("keydown",(e:KeyboardEvent)=>{
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
        })
    }
}