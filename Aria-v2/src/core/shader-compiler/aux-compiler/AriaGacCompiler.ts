import { AriaObject } from "../../base/AriaObject";
import { AriaGacLexer } from "./AriaGacLexer";
import { AriaGacLL1ParserNonterimalNode, IAriaGacLL1ParserInputToken } from "./AriaGacLL1Parser";
import { AriaGacSimpleParser } from "./AriaGacSimpleParser";

export class AriaGacCompiler extends AriaObject{
    private _lexer = new AriaGacLexer()
    private _parser = new AriaGacSimpleParser()
    private _result:AriaGacLL1ParserNonterimalNode|null = null

    constructor(){
        super("AriaGac/Compiler")
    }
    public compile(x:string){
        this._logInfo("agac.compiler: compile: compilation starts")
        let tokens = this._lexer.parse(x+"\n")
        let convertedTokens:IAriaGacLL1ParserInputToken[] = []
        if(tokens.valid==false){
            this._logError("agac.compiler: compile: failed to compile source at lexer procedure:"+tokens.info)
            throw new Error()
        }
        tokens.tokens.forEach((el)=>{
            convertedTokens.push({
                tag:el.getTag(),
                end_token:false,
                content:el.getValue()
            })
        })
        this._result = this._parser.parse(convertedTokens)
        this._logInfo("agac.compiler: compile: compilation done")
    }
    public generateCode(){
        if(this._result===null){
            this._logError("agac.compiler: generateCode: no code has been compiled")
            throw new Error()
        }   
        this._logInfo("agac.compiler: generateCode: start code generation")
        return this._result.generateCode()
    }
}