import { str } from "gl-matrix-ts/dist/mat3";
import { AriaObject } from "../base/AriaObject";

export interface IAriaGacLexerParsingResult<T>{
    valid:boolean,
    tokens:T[],
    info:string
}

export interface IAriaGacLexerAcceptanceAction<T>{
    accept:boolean
    final:boolean
    retrace:number
    next:AriaGacLexerStatusNode<T>|null
}

export interface IAriaGacLexerToken<T>{
    getValue():T
    parse(ins:string):T
    getTag():string
}

export class AriaGacLexerToken<T> extends AriaObject implements IAriaGacLexerToken<T>{
    private tag:string = ""
    protected content:string = ""
    constructor(tag:string){
        super("AriaGac/LexerTokenTag")
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

export class AriaGacLexerBaseToken extends AriaGacLexerToken<string>{
    
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


export class AriaGacLexerStatusNode<T> extends AriaObject{
    private final:boolean = false
    private retrace:number = 0
    private forward:Map<string,AriaGacLexerStatusNode<T>> = new Map<string,AriaGacLexerStatusNode<T>>()
    private attrs:T|null = null

    constructor(){
        super("AriaGac/LexerStatusNode")
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
    public define(condition:string,forwarding:AriaGacLexerStatusNode<T>){
        this.forward.set(condition,forwarding)
    }
    public accept(acc:string):IAriaGacLexerAcceptanceAction<T>{
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
    public acceptCondition(cond:string):AriaGacLexerStatusNode<T>|null{
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

export class AriaGacLexerRuleSet extends AriaObject{
    private tags:Map<string,typeof AriaGacLexerToken<string>>= new Map<string,typeof AriaGacLexerToken<string>>()
    private ruleSet:AriaGacLexerStatusNode<string> = new AriaGacLexerStatusNode<string>();
    
    constructor(){
        super("AriaGac/LexerRuleSet")
    }
    public test(source:string):IAriaGacLexerParsingResult<AriaGacLexerBaseToken>{
        let curNode = this.ruleSet
        const invalidReturn = (info:string)=>{
            return <IAriaGacLexerParsingResult<AriaGacLexerBaseToken>>{
                valid:false,
                tokens:[],
                info:info
            }
        }
        let tokenList:AriaGacLexerBaseToken[] = []
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
    public addRule(rule:string[],identifier:string,retrace:number,tokenParser:typeof AriaGacLexerToken<string>=AriaGacLexerBaseToken){
        let cur = this.ruleSet
        this.tags.set(identifier,tokenParser)
        for(let i=0;i<rule.length;i++){
            if(rule[i].startsWith("(")&&rule[i].endsWith(")")&&rule[i][1]!="-"){
                //Circular rule
                const tmp = cur.acceptCondition(rule[i].substring(1,rule[i].length-1))
                const drule = rule[i].substring(1,rule[i].length-1)
                if(tmp==null){
                    const newNode = new AriaGacLexerStatusNode<string>()
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
                    const newNode = new AriaGacLexerStatusNode<string>()
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

export class AriaGacLexerBase extends AriaObject{
    private ruleSet = new AriaGacLexerRuleSet()
    private endSign = "\x00"

    public static readonly AGL_END_SIGN = "\x00"
    public static readonly AGL_NUMERIC = "0-9"
    public static readonly AGL_ANY = "\x00-\xff"
    public static readonly AGL_NON_NUMERIC = "\x00-\x2f|\x3a-\xff"
    public static readonly AGL_ALPHABETIC = "A-Z|a-z"
    public static readonly AGL_NON_ALPHABETIC = "\x00-\x3f|\x5b-\x60|\x7b-\xff"
    constructor(){
        super("AriaGac/LexerBase")
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
export enum AriaGacLexerTokenTypes{
    AGLT_RESERVED_METHODS = "agl_reserved_methods",
    AGLT_FLOAT = "agl_float",
    AGLT_INT = "agl_int",
    AGLT_LEFT_BRACKET = "agl_left_bracket",
    AGLT_RIGHT_BRACKET = "agl_right_bracket",
    AGLT_WORD = "agl_word",

    AGLT_GLSL_NOTE_SL = "agl_glsl_note_sl",
    AGLT_LEFT_BRACE = "agl_left_brace",
    AGLT_RIGHT_BRACE = "agl_left_brace",
    AGLT_LEFT_PARE = "agl_left_pare",
    AGLT_RIGHT_PARE = "agl_left_pare",

    AGLT_ADD = "agl_add",
    AGLT_SUB = "agl_sub",
    AGLT_MUL = "agl_mul",
    AGLT_DIV = "agl_div",
    AGLT_LAND = "agl_land",
    AGLT_LOR = "agl_lor",
    AGLT_LNOT = "agl_lnot",
    AGLT_LEQ = "agl_leq",
    AGLT_LNEQ = "agl_lneq",
    AGLT_LESS = "agl_less",
    AGLT_GREATER = "agl_greater",
    AGLT_LESS_EQ = "agl_less_eq",
    AGLT_GREATER_EQ = "agl_greater_eq",

    AGLT_ASSIGN = "agl_assign",
    AGLT_ADD_ASSIGN = "agl_add_assign",
    AGLT_SUB_ASSIGN = "agl_sub_assign",
    AGLT_MUL_ASSIGN = "agl_mul_assign",
    AGLT_DIV_ASSIGN = "agl_div_assign",
    AGLT_CHANGE_LINE = "agl_cl",

    AGLT_COMMA = "agl_comma",
    AGLT_SEP = "agl_sep",
    AGLT_DOT = "agl_dot",

    AGLT_SPACING = "agl_spacing",

    AGLT_SHARP = "agl_sharp",
    AGLT_COLON = "agl_colon",

    AGLT_INC = "agl_inc",
    AGLT_DEC = "agl_dec",

    AGLT_END_SIGN= "agl_ends"
}

export class AriaGacLexer extends AriaGacLexerBase{
    constructor(){
        super()
        this._rename("AriaGac/Lexer")
        //decorators
        const recursive=(x:string)=>{return "("+x+")"}
        const tp = AriaGacLexer
        const en = AriaGacLexerTokenTypes

        //Reserved methods
        const rmPrefix = ["!","@","a","s","u"]
        this.addRule(rmPrefix.concat(recursive(tp.AGL_ALPHABETIC)).concat(tp.AGL_NON_ALPHABETIC),en.AGLT_RESERVED_METHODS,1)

        //numbers
        this.addRule([recursive(tp.AGL_NUMERIC),".",recursive(tp.AGL_NUMERIC),tp.AGL_NON_NUMERIC],en.AGLT_FLOAT,1)
        this.addRule([recursive(tp.AGL_NUMERIC),"\x00-\\x2d|\x2f|\x3a-\xff"],en.AGLT_INT,1)
        
        //brackets
        this.addRule(["[",tp.AGL_ANY],en.AGLT_LEFT_BRACKET,1)
        this.addRule(["]",tp.AGL_ANY],en.AGLT_RIGHT_BRACKET,1)
        this.addRule(["{",tp.AGL_ANY],en.AGLT_LEFT_BRACE,1)
        this.addRule(["}",tp.AGL_ANY],en.AGLT_RIGHT_BRACE,1)
        this.addRule(["(",tp.AGL_ANY],en.AGLT_LEFT_PARE,1)
        this.addRule([")",tp.AGL_ANY],en.AGLT_RIGHT_PARE,1)

        //words
        this.addRule(["_|a-z|A-z","\x00-\x2f|\x3a-\x40|\x5b-\x5e|\x60|\x7b-\xff"],en.AGLT_WORD,1)
        this.addRule(["_|a-z|A-z",recursive("_|a-z|A-Z|0-9"),"\x00-\x2f|\x3a-\x40|\x5b-\x5e|\x60|\x7b-\xff"],en.AGLT_WORD,1)

        //note starts
        this.addRule(["/","/",tp.AGL_ANY],en.AGLT_GLSL_NOTE_SL,1)

        //other ops
        this.addRule(["+","\x00-\x2a|\x2c-\x3c|\x3e-\xff"],en.AGLT_ADD,1)
        this.addRule(["\\--\\-","\x00-\x2c|\x2e-\x3c|\x3e-\xff"],en.AGLT_SUB,1)
        this.addRule(["*","\x00-\x3c|\x3e-\xff"],en.AGLT_MUL,1)
        this.addRule(["/","\x00-\x3c|\x3e-\xff"],en.AGLT_DIV,1)
        this.addRule(["&","&",tp.AGL_ANY],en.AGLT_LAND,1)
        this.addRule(["\\|-\\|","\\|-\\|",tp.AGL_ANY],en.AGLT_LOR,1)
        this.addRule(["=","=",tp.AGL_ANY],en.AGLT_LEQ,1)
        this.addRule(["!","=",tp.AGL_ANY],en.AGLT_LNEQ,1)
        this.addRule([".",tp.AGL_ANY],en.AGLT_DOT,1)
        this.addRule([";",tp.AGL_ANY],en.AGLT_SEP,1)
        this.addRule([",",tp.AGL_ANY],en.AGLT_COMMA,1)
        this.addRule(["<","\x00-\x3c|\x3e-\xff"],en.AGLT_LESS,1)
        this.addRule([">","\x00-\x3c|\x3e-\xff"],en.AGLT_GREATER,1)

        this.addRule(["<","="],en.AGLT_LESS_EQ,1)
        this.addRule([">","="],en.AGLT_GREATER_EQ,1)

        this.addRule(["+","+",tp.AGL_ANY],en.AGLT_INC,1)
        this.addRule(["\\--\\-","\\--\\-",tp.AGL_ANY],en.AGLT_DEC,1)

        //assigns
        this.addRule(["=","\x00-\x3c|\x3e-\xff"],en.AGLT_ASSIGN,1)
        this.addRule(["+","="],en.AGLT_ADD_ASSIGN,0)
        this.addRule(["\\--\\-","="],en.AGLT_SUB_ASSIGN,0)
        this.addRule(["*","="],en.AGLT_MUL_ASSIGN,0)
        this.addRule(["/","="],en.AGLT_DIV_ASSIGN,0)

        //spacings & change line
        this.addRule([recursive(" |\t"),"\x00-\x08|\x0a-\x1f|\x21-\xff"],en.AGLT_SPACING,1)
        this.addRule(["\n",tp.AGL_ANY],en.AGLT_CHANGE_LINE,1)
        this.addRule(["\r","\n",tp.AGL_ANY],en.AGLT_CHANGE_LINE,1)

        //pragma
        this.addRule(["#",tp.AGL_ANY],en.AGLT_SHARP,1)

        //misc
        this.addRule([":",tp.AGL_ANY],en.AGLT_COLON,1)
        this.addRule(["\x00"],en.AGLT_END_SIGN,0)
    }
}