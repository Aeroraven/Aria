export interface IAriaDynamicGeometry{
    localRotateX(r:number):void
    localRotateY(r:number):void
    localRotateZ(r:number):void
    localTranslate(x:number,y:number,z:number):void
    localTranslateAbsolute(x:number,y:number,z:number):void
    localPosition(x:number,y:number,z:number):void
}