export class AriaComponent{
    protected gl:WebGL2RenderingContext
    private components:Map<string,AriaComponent>
    private attributes:Map<string,any>
    protected parent:AriaComponent|null

    protected constructor(gl:WebGL2RenderingContext){
        this.gl = gl
        this.components = new Map<string,AriaComponent>()
        this.attributes = new Map<string,any>()
        this.parent = null
    }

    static create(gl:WebGL2RenderingContext){
        let p = new this(gl)
        return p
    }

    protected register(){
        throw new Error("Component cannot be registered")    
    }

    protected respond(command:string, args:any[]):any{
        throw new Error("Component cannot respond")   
    }

    public addComponent(name:string,x:AriaComponent){
        this.components.set(name,x)
        x.parent = this
        x.register()
        return this
    }

    public childExist(name:string){
        return this.components.has(name)
    }

    public signal(child:string,command:string,...args:any[]):any{
        return this.components.get(child)?.respond(command,args)
    }

    public setAttr(name:string,x:any){
        this.attributes.set(name,x)
        return this
    }
    public getAttr(name:string){
        return this.attributes.get(name)
    }
    public getChild(name:string){
        return this.components.get(name)
    }
}