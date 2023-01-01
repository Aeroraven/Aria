import { AriaRef } from "../base/AriaBaseDefs";
import { AriaObject } from "../base/AriaObject";

export interface IAriaGacLL1ParserInputToken{
    content:string
    tag:string
    end_token:boolean
}

export type AriaGacLL1ParserTerminalNodeCriterion = (x:IAriaGacLL1ParserInputToken)=>boolean
type AriaGacLL1ParserRule = ((typeof AriaGacLL1ParserNonterimalNode)|AriaGacLL1ParserTerminalNodeCriterion)[]

export class AriaGacLL1ParserNonterimalNode extends AriaObject{
    public children:(AriaGacLL1ParserNonterimalNode|IAriaGacLL1ParserInputToken)[] = []
    protected rules:AriaGacLL1ParserRule[] = []
    protected chosenRule = 0
    protected identifier:string = ""
    protected guaranteeValid = false
    constructor(){
        super("AriaGac/LL1ParserNonterminalNode")
    }
    protected setIdentifier(x:string){
        this.identifier = x
    }
    protected internalParse(lexerTokens:IAriaGacLL1ParserInputToken[],curPointer:AriaRef<number>,depth:number):boolean{
        
        if(depth>2048){
            this._logError("agac.ll1parser.nonterminal_node: recursion exceeded limit during the parsing procedure at position "+curPointer.value)
            throw Error()
        }
        if(curPointer.value>=lexerTokens.length){
            return false
        }
        let startingPos = curPointer.value
        let succFlag = false
        for(let i=0;i<this.rules.length;i++){
            curPointer.value = startingPos
            this.children = []
            if(this.rules[i].length==0){
                return true
            }
            succFlag=true
            //this._logInfo("Using rule:"+i+", depth="+depth+", cursor="+startingPos)
            for(let j=0;j<this.rules[i].length;j++){
                if('prototype' in this.rules[i][j]){
                    let wx = new (<typeof AriaGacLL1ParserNonterimalNode>this.rules[i][j])()
                    
                    if(wx.internalParse(lexerTokens,curPointer,depth+1)==false){
                        succFlag=false
                        break
                    }else{
                        this.children.push(wx)
                    }
                }else{
                    
                    if((<AriaGacLL1ParserTerminalNodeCriterion>(this.rules[i][j]))(lexerTokens[curPointer.value])){
                        this.children.push(lexerTokens[curPointer.value])
                        curPointer.value+=1
                    }else{
                        succFlag=false
                        break
                    }
                }
            }
            if(succFlag){
                return true
            }
        }
        if(this.guaranteeValid){
            this._logError("agac.ll1parser.nonterminal_node: no candidate found at position "+startingPos)
        }
        return false
    }
    public parse(lexerTokens:IAriaGacLL1ParserInputToken[]){
        let cur = new AriaRef<number>(0)
        return this.internalParse(lexerTokens,cur,0)
    }
    public addRule(rule:AriaGacLL1ParserRule){
        this.rules.push(rule)
    }
    public generateCode():string{
        this._logError("agac.ll1parser.nonterminal_node: code generation not specified")
        throw Error()
    }
}

export class AriaGacLL1ParserBase extends AriaObject{
    private startingNode:typeof AriaGacLL1ParserNonterimalNode = AriaGacLL1ParserNonterimalNode
    constructor(){
        super("AriaGac/LL1ParserBase")
    }
    protected setStartingNode(x:typeof AriaGacLL1ParserNonterimalNode){
        this.startingNode = x
    }
    public parse(x:IAriaGacLL1ParserInputToken[]):AriaGacLL1ParserNonterimalNode{
        const stNode = new this.startingNode()
        stNode.parse(x)
        return stNode
    }
}

