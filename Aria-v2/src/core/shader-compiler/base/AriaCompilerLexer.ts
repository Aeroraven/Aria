import { AriaObject } from "../../base/AriaObject";

export interface IAriaShaderCompilerLexerParsingResult<T>{
    valid:boolean,
    tokens:T[],
    info:string
}

export interface IAriaShaderCompilerLexerAcceptanceAction<T>{
    accept:boolean
    final:boolean
    retrace:number
    next:AriaShaderCompilerLexerStatusNode<T>|null
}

export interface IAriaShaderCompilerLexerToken<T>{
    getValue():T
    parse(ins:string):T
    getTag():string
}

export class AriaShaderCompilerLexerToken<T> extends AriaObject implements IAriaShaderCompilerLexerToken<T>{
    private tag:string = ""
    protected content:string = ""
    constructor(tag:string){
        super("AriaShaderCompiler/LexerTokenTag")
        this.tag = tag
    }
    public getValue():T{
        this._logError("agac.lexer_token.get_value: not implemented")
        throw Error()
    }
    public parse(ins:string):T{
        this._logError("agac.lexer_token.parse: not implemented")
        throw Error()
    }
    public getTag(): string {
        return this.tag
    }
}

export class AriaShaderCompilerLexerBaseToken extends AriaShaderCompilerLexerToken<string>{
    
    constructor(tag:string){
        super(tag)
    }
    public parse(ins: string): string {
        this.content = ins
        return ins
    }
    public getValue(): string {
        return this.content
    }
}


export class AriaShaderCompilerLexerStatusNode<T> extends AriaObject{
    private final:boolean = false
    private retrace:number = 0
    private forward:Map<string,AriaShaderCompilerLexerStatusNode<T>> = new Map<string,AriaShaderCompilerLexerStatusNode<T>>()
    private attrs:T|null = null

    constructor(){
        super("AriaShaderCompiler/LexerStatusNode")
    }
    public getFinal(){
        return this.final
    }
    public getRetrace(){
        return this.retrace
    }
    public setFinal(final:boolean,retrace:number,value:T){
        this.final = final
        this.retrace = retrace
        this.attrs=value
    }
    private testCondition(cond:string,dest:string){
        //cond => W|cond
        //cond => A-B|A 
        //A,B, in ascii characters
        const testConds:string[] = []
        let escape = false
        let lastpos  = 0
        cond = cond + "|"
        const removeEscapeSign = (x:string)=>{
            if(x.startsWith("\\")){
                return x.substring(1,x.length)
            }
            return x
        }
        for(let i=0;i<cond.length;i++){
            if(escape==false){
                if(cond[i]=='|'){
                    testConds.push(cond.substring(lastpos,i))
                    const curCond = cond.substring(lastpos,i)
                    lastpos=i+1
                    let escapeIn = false
                    let divPos = -1
                    for(let j=0;j<curCond.length;j++){
                        if(escapeIn==false){
                            if(curCond[j]=='-'){
                                divPos=j
                            }else if(curCond[j]=='\\'){
                                escapeIn = true
                            }
                        }else{
                            escapeIn = false
                        }
                    }
                    if(divPos==-1){
                        if(curCond.charCodeAt(0)==dest.charCodeAt(0)){
                            return true
                        }
                    }else{
                        const condLeft = removeEscapeSign(curCond.substring(0,divPos))
                        const condRight = removeEscapeSign(curCond.substring(divPos+1,curCond.length))
                        if(dest.charCodeAt(0)>=condLeft.charCodeAt(0)&&dest.charCodeAt(0)<=condRight.charCodeAt(0)){
                            return true
                        }
                    }
                }else if(cond[i]=="\\"){
                    escape=true
                }
            }else{
                escape = false
            }
        }
        return false
    }
    public define(condition:string,forwarding:AriaShaderCompilerLexerStatusNode<T>){
        this.forward.set(condition,forwarding)
    }
    public accept(acc:string):IAriaShaderCompilerLexerAcceptanceAction<T>{
        let mustbe = ""
        for(let i of this.forward.keys()){
            if(this.testCondition(i,acc)){
                const nextNode = this.forward.get(i)!
                return {
                    final: nextNode.final,
                    retrace: nextNode.retrace,
                    accept:true,
                    next:nextNode
                }
            }
            mustbe += i+" or "
        }
        mustbe += " equivalent forms"
        this._logWarn("agac.lexer.statnode: failed to accept token, must be:"+mustbe)
        return {
            final:false,
            retrace:0,
            accept:false,
            next:null
        }
    }
    public acceptCondition(cond:string):AriaShaderCompilerLexerStatusNode<T>|null{
        for(let i of this.forward.keys()){
            if(i==cond){
                return this.forward.get(i)!
            }
        }
        return null
    }
    public setAttr(x:T){
        this.attrs=x
    }
    public getAttr(){
        return this.attrs
    }
}

export class AriaShaderCompilerLexerRuleSet extends AriaObject{
    private tags:Map<string,typeof AriaShaderCompilerLexerToken<string>>= new Map<string,typeof AriaShaderCompilerLexerToken<string>>()
    private ruleSet:AriaShaderCompilerLexerStatusNode<string> = new AriaShaderCompilerLexerStatusNode<string>();
    
    constructor(){
        super("AriaShaderCompiler/LexerRuleSet")
    }
    public test(source:string):IAriaShaderCompilerLexerParsingResult<AriaShaderCompilerLexerBaseToken>{
        let curNode = this.ruleSet
        const invalidReturn = (info:string)=>{
            return <IAriaShaderCompilerLexerParsingResult<AriaShaderCompilerLexerBaseToken>>{
                valid:false,
                tokens:[],
                info:info
            }
        }
        let tokenList:AriaShaderCompilerLexerBaseToken[] = []
        let lastPos = 0

        let prevI = -1
        let prevTimes = 0
        for(let i=0;i<source.length;i++){
            //window.alert("Parsing:"+i+":"+source[i])
            //this._logInfo("parsing:"+source[i])
            const status = curNode.accept(source[i])

            if(prevI == i){
                prevTimes++
                if(prevTimes>=5){
                    this._logError("agac.lexer: infinite loop at position "+i)
                }
            }else{
                prevI = i
                prevTimes=0
            }
            
            if(status.accept==false){
                this._logError("agac.lexer: failed to parse string at position "+i+": unexpected token:"+(source[i])+"(code="+source.charCodeAt(i)+")")
                return invalidReturn("agac.lexer.invalid_expression")
            }else{
                if(status.final==false){
                    if(status.next==null){
                        return invalidReturn("agac.lexer.dictionary_incomplete")
                    }
                    curNode = status.next
                }else{
                    if(status.next===null){
                        return invalidReturn("agac.lexer.dictionary_incomplete")
                    }
                    const tagId = status.next.getAttr()!
                    const tp = (this.tags.get(status.next.getAttr()!))!
                    const newToken = new tp(tagId)
                    const pw = source.substring(lastPos,i+1-status.retrace)
                    //this._logInfo("emitting:"+source.substring(lastPos,i+1-status.retrace)+","+tagId)
                    newToken.parse(pw)
                    i-=status.retrace
                    tokenList.push(newToken)
                    curNode=this.ruleSet
                    lastPos=i+1
                }
            }
        }
        return {
            valid:true,
            info:"agac.lexer.success",
            tokens:tokenList
        }
    }
    public addRule(rule:string[],identifier:string,retrace:number,tokenParser:typeof AriaShaderCompilerLexerToken<string>=AriaShaderCompilerLexerBaseToken){
        let cur = this.ruleSet
        this.tags.set(identifier,tokenParser)
        for(let i=0;i<rule.length;i++){
            if(rule[i].startsWith("(")&&rule[i].endsWith(")")&&rule[i][1]!="-"){
                //Circular rule
                const tmp = cur.acceptCondition(rule[i].substring(1,rule[i].length-1))
                const drule = rule[i].substring(1,rule[i].length-1)
                if(tmp==null){
                    const newNode = new AriaShaderCompilerLexerStatusNode<string>()
                    newNode.setFinal(i==rule.length-1,retrace,identifier)
                    newNode.define(drule,newNode)
                    cur.define(drule,newNode)
                    cur = newNode
                }else{
                    cur = tmp
                }
            }else{
                //Plain rule
                const tmp = cur.acceptCondition(rule[i])
                if(tmp==null){
                    const newNode = new AriaShaderCompilerLexerStatusNode<string>()
                    newNode.setFinal(i==rule.length-1,retrace,identifier)
                    cur.define(rule[i],newNode)
                    cur = newNode
                }else{
                    cur = tmp
                }
            }
        }
    }
}

export class AriaShaderCompilerLexerBase extends AriaObject{
    private ruleSet = new AriaShaderCompilerLexerRuleSet()
    private endSign = "\x00"

    public static readonly AGL_END_SIGN = "\x00"
    public static readonly AGL_NUMERIC = "0-9"
    public static readonly AGL_ANY = "\u0000-\uffff"
    public static readonly AGL_NON_NUMERIC = "\x00-\x2f|\x3a-\uffff"
    public static readonly AGL_ALPHABETIC = "A-Z|a-z"
    public static readonly AGL_NON_ALPHABETIC = "\x00-\x3f|\x5b-\x60|\x7b-\uffff"
    constructor(){
        super("AriaShaderCompiler/LexerBase")
    }
    public addRule(rule:string[],identifier:string,retrace:number){
        this.ruleSet.addRule(rule,identifier,retrace)
    }
    public parse(x:string){
        x = x + this.endSign
        this._logInfo("agac.lexer: start parsing")
        let res = this.ruleSet.test(x)
        this._logInfo("agac.lexer: completed, with "+res.tokens.length+" tokens returned.")
        return res
    }
}
