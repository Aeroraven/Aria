import { AriaObject } from "../base/AriaObject";

class AriaGacFuncInjector extends AriaObject{
    constructor(){
        super("AriaGac/FuncInjector")
    }
    public generate(args:string[]){
        this._logError("agac.func_injector: generate: not implemented")
        return ""
    }
}

export class AriaGacFuncInjectorIndexer extends AriaObject{
    private _methodList:Map<string,AriaGacFuncInjector> = new Map<string,AriaGacFuncInjector>()
    constructor(){
        super("AriaGac/FuncInjectorIndexer")
        this._methodList.set("SelectCubeSamplerInArray",new AriaGacFunc_SelectCubeSamplerInArray())
    }
    public generate(method:string,args:string[]){
        const ind = this._methodList.get(method)
        if(ind){
            return ind.generate(args)
        }else{
            this._logError("agac.func_injector_indexer: method `"+method+"` is not valid.")
            throw Error()
        }
    }

}

class AriaGacFunc_SelectCubeSamplerInArray extends AriaGacFuncInjector{
    constructor(){
        super()
        this._rename("AriaGac/Func/SelectCubeSamplerInArray")
    }
    public generate(args: string[]): string {
        if(args.length!=5){
            this._logError("agac.func.SelectCubeSamplerInArray: requires 5 arguments, but got "+args.length+","+args)
        }
        const outputVar = args[0]
        const samplerName = args[1]
        const indexVar = args[2]
        const indexUpperBound = parseInt(args[3])
        const texPos = args[4]
        let res = ""
        for(let i=0;i<indexUpperBound;i++){
            res+="if("+indexVar+"=="+i+"){\n"+outputVar+"=texture("+samplerName+"["+i+"],"+texPos+");\n}\n"
        }
        return res
    }
}