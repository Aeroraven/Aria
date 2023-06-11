import { mat4, vec3 } from "gl-matrix-ts";
import { AriaEnv } from "./AriaEnv";
import { AriaGacCompiler } from "../aux-compiler/AriaGacCompiler";
import { AriaObject } from "../base/AriaObject";
import { IAriaFramebuffer } from "../interface/IAriaFramebuffer";
import { IAriaTexture } from "../interface/IAriaTexture";

class AriaFramebufferManager extends AriaObject{
    private static _instance:AriaFramebufferManager|undefined = undefined
    private activeFramebuffer:IAriaFramebuffer[] = []
    private constructor(){
        super("AriaFramebufferManager")
    }
    public static getInst(){
        if(this._instance===undefined){
            this._instance = new this()
        }
        return this._instance!
    }
    public setFramebuffer(buf:IAriaFramebuffer){
        this.activeFramebuffer.push(buf)
    }
    public getFramebuffer(){
        if(this.activeFramebuffer.length==0){
            return null
        }
        return this.activeFramebuffer[this.activeFramebuffer.length-1]
    }
    public removeFramebuffer(){
        return this.activeFramebuffer.pop()
    }

}

class AriaCameraPositionManager extends AriaObject{
    private camPos = [0,0,0]
    private static _instance:AriaCameraPositionManager|undefined = undefined
    private constructor(){
        super("AriaCameraPositionManager")
    }
    public static getInst(){
        if(this._instance===undefined){
            this._instance = new this()
        }
        return this._instance!
    }
    public setCameraPosition(x:number,y:number,z:number){
        this.camPos = [x,y,z]
    }
    public getCubeMapLookat(face:number){
        const lookDirs = [
            [1,0,0],
            [-1,0,0],
            [0,1,0],
            [0,-1,0],
            [0,0,1],
            [0,0,-1]
        ]
        const upDirs = [
            [0,-1,0],
            [0,-1,0],
            [0,0,1],
            [0,0,-1],
            [0,-1,0],
            [0,-1,0],
            [0,-1,0]
        ]
        const dpos = new Float32Array([this.camPos[0],this.camPos[1],this.camPos[2]])
        const dfront = new Float32Array([this.camPos[0]+lookDirs[face][0],this.camPos[1]+lookDirs[face][1],this.camPos[2]+lookDirs[face][2]])
        const dtop = new Float32Array([upDirs[face][0],upDirs[face][1],upDirs[face][2]])
        const ret = mat4.create()
        mat4.lookAt(ret,dpos,dfront,dtop)
        return ret
    }
}

export class AriaRenderOps extends AriaObject{
    constructor(){
        super("AriaRenderOps")
    }
}