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

        this.camPos[0] = 0.0
        this.camPos[1] = 0.0
        this.camPos[2] = -6.0
        
        this.camFront[0] = 0.0
        this.camFront[1] = 0.0
        this.camFront[2] = 1.0

        this.camUp[0] = 0.0
        this.camUp[1] = 1.0
        this.camUp[2] = 0.0

        vec3.add(this.camLookAtCenter,this.camFront,this.camPos)
        mat4.lookAt(this.camLookAt,this.camPos,this.camLookAtCenter,this.camUp)
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
            this.camYaw = (mx/this.winWidth)*3.1415926
            this.camFront[0] = Math.cos(this.camPitch) * Math.cos(this.camYaw)
            this.camFront[1] = Math.sin(this.camPitch)
            this.camFront[2] = Math.cos(this.camPitch) * Math.sin(this.camYaw)
            //vec3.normalize(this.camFront,this.camFront)
            this.movePos(0,0,0)
            console.log(this.camFront)
        })
        window.addEventListener("keydown",(e:KeyboardEvent)=>{
            if(e.key.toLowerCase() == "a"){
                this.movePos(-this.roleStep,0,0)
            }
            if(e.key.toLowerCase() == "d"){
                this.movePos(this.roleStep,0,0)
            }
            if(e.key.toLowerCase() == "w"){
                this.movePos(0,0,this.roleStep)
            }
            if(e.key.toLowerCase() == "s"){
                this.movePos(0,0,-this.roleStep)
            }
            if(e.key.toLowerCase() == "q"){
                this.movePos(0,this.roleStep,0)
            }
            if(e.key.toLowerCase() == "e"){
                this.movePos(0,-this.roleStep,0)
            }
        })
    }
}