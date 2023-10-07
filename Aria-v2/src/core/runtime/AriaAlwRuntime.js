import { alwGetRuntime } from "../../library/AlwCore.Runtime"

export class AriaAlwRuntime{
    static _instance = undefined
    _runtime = undefined
    constructor(){
        
    }
    async initRuntime(){
        this._runtime = await alwGetRuntime()
        console.log("Runtime Initialized")
    }
    get runtime(){
        if(this._runtime === undefined){
            throw new Error("AriaAlwRuntime is not initialized")
        }
        return this._runtime
    }
    static get instance(){
        if(this._instance === undefined){
            this._instance = new AriaAlwRuntime()
        }
        return this._instance
    }
    
}