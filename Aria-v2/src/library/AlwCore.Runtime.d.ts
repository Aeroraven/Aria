export declare class AlwVectorTp{
    constructor(x:number)
    public at(i:number):number
    public setVal(i:number,v:number):void
    public add(x:AlwVector):AlwVector
    public add_(x:AlwVector):AlwVector
    public addScaled_(x:AlwVector,s:number):AlwVector
    public sub(x:AlwVector):AlwVector
    public sub_(x:AlwVector):AlwVector
    public mul(x:AlwVector):AlwVector
    public mul_(x:AlwVector):AlwVector
    public mulScalar(x:number):AlwVector
    public mulScalar_(x:number):AlwVector
    public div(x:AlwVector):AlwVector
    public div_(x:AlwVector):AlwVector
    public divScalar(x:number):AlwVector
    public divScalar_(x:number):AlwVector
    public dot(x:AlwVector):number
    public len():number
    public normalize():AlwVector
    public normalize_():AlwVector
    public fromArray(x:number[]):AlwVector
    public toArray():number[]
    public cross(x:AlwVector):AlwVector
    public cross_(x:AlwVector):AlwVector
    public static create(x:number):AlwVector
    public static createA(x:number[]):AlwVector
}
export interface IAriaAlwRuntimeBackend{
    AlwVector: typeof AlwVectorTp
}
export async function alwGetRuntime():Promise<IAriaAlwRuntimeBackend>