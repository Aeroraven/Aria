import { IAriaAlwRuntimeBackend,alwGetRuntime } from "../../library/AlwCore.Runtime"

export declare class AriaAlwRuntime{
    private static _instance:AriaAlwRuntime|undefined = undefined
    private _runtime:IAriaAlwRuntimeBackend|undefined = undefined
    constructor()
    async initRuntime():Promise<void>
    public get runtime():IAriaAlwRuntimeBackend
    public static get instance():AriaAlwRuntime
    
}