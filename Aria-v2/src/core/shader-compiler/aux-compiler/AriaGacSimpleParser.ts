import { AriaGacLL1ParserBase, IAriaGacLL1ParserInputToken, AriaGacLL1ParserNonterimalNode, AriaGacLL1ParserTerminalNodeCriterion } from "./AriaGacLL1Parser";
import { AriaGacLexer, AriaGacLexerTokenTypes} from "./AriaGacLexer"
import { AriaGacFuncInjectorIndexer } from "./AriaGacAuxFuncInjector";

//Implementaion Starts

//[CodeLine] -> ([NoteLine]|[ShaderCodeLine]|[Spacings]|[EmptyLine]|[AuxCodeLine]) | [CodeLine]

//[NoteLine] -> AGL_GLSL_NOTE [NoteContent] AGL_CHANGE_LINE
//[NoteContent] -> (AGL_ANY-(AGL_RESERVED_METHOD|AGL_CHANGE_LINE)) [NoteContent_Body]
//[NoteContent_Body] -> (AGL_ANY-AGL_CHANGE_LINE) [NoteContent_Body] | EMPTY

//[ShaderCodeLine] -> (AGL_ANY-(AGL_GLSL_NOTE|AGL_SPACING|AGL_CHANGE_LINE|AGL_ENDS)) [ShaderCodeLine_Body] AGL_CHANGE_LINE
//[ShaderCodeLine_Body] -> (AGL_ANY-AGL_CHANGE_LINE) [ShaderCodeLine_Body] | EMPTY

//[Spacings] -> AGL_SPACING [Spacings]  | AGL_SPACING
//[EmptyLine] -> AGL_CHANGE_LINE

//[AuxCodeLine] -> AGL_GLSL_NOTE [AuxCodeExpl] AGL_CHANGE_LINE
//[AuxCodeExpl] -> [AuxFunc]

//[AuxFunc] -> AGL_RESERVED_METHODS AGL_LEFT_PARE [AuxFuncArgs] AGL_RIGHT_PARE 
//[AuxFuncArgs] -> ((AGL_ANY-(AGL_CHANGE_LINE|AGL_COMMA)) [AuxFuncArgsCont] ) | EMPTY
//[AuxFuncArgCont] -> AGL_COMMA (AGL_ANY-(AGL_CHANGE_LINE)) [AuxFuncArgsCont] | EMPTY 

//[EndLine] -> AGL_END_SIGN

class AriaGacSimpleParserNode extends AriaGacLL1ParserNonterimalNode{
    public generateCode(): string {
        let w = ""
        for(let i=0;i<this.children.length;i++){
            const child = this.children[i]
            if(child instanceof AriaGacLL1ParserNonterimalNode){
                w+=child.generateCode()
            }else{
                w+=child.content
            }
        }
        return w
    }
}

class AriaGacParserNode_AuxFuncArgCont extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/AuxFuncArgCont")
        this.addRule([
            (x)=>{return x.tag==AriaGacLexerTokenTypes.AGLT_COMMA},
            (x)=>{return x.tag!=AriaGacLexerTokenTypes.AGLT_CHANGE_LINE},
            AriaGacParserNode_AuxFuncArgCont
        ])
        this.addRule([])
    }
    
}

class AriaGacParserNode_AuxFuncArgs extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/AuxFuncArgs")
        this.addRule([
            (x)=>{return (x.tag!=AriaGacLexerTokenTypes.AGLT_CHANGE_LINE&&x.tag!=AriaGacLexerTokenTypes.AGLT_COMMA)},
            AriaGacParserNode_AuxFuncArgCont
        ])
        this.addRule([])
    }
}

class AriaGacParserNode_AuxFunc extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/AuxFunc")
        this.addRule([
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_RESERVED_METHODS)},
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_LEFT_PARE)},
            AriaGacParserNode_AuxFuncArgs,
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_RIGHT_PARE)},
        ])
    }
    public generateCode(): string {
        let method = (<IAriaGacLL1ParserInputToken>this.children[0]).content.substring(5)
        const args:string[] = []
        //First argument
        const fargs = (<AriaGacLL1ParserNonterimalNode>this.children[2])
        let cp = 0
        if(fargs.children.length==2){
            args.push( (<IAriaGacLL1ParserInputToken>(fargs.children[0])).content )
            //Iterative
            let cur:AriaGacLL1ParserNonterimalNode = <AriaGacLL1ParserNonterimalNode>fargs.children[1]
            while(true){
                cp+=1
                if(cp>=17){
                    this._logError("agac.simple_parser.aux_func: generateCode: exceed recursion limit")
                }
                if(cur.children.length==0){
                    break
                }
                args.push( (<IAriaGacLL1ParserInputToken>(cur.children[1])).content )
                cur = <AriaGacLL1ParserNonterimalNode>cur.children[2]
            }
        }
        let processor = new AriaGacFuncInjectorIndexer()
        return processor.generate(method,args)
    }
}

class AriaGacParserNode_AuxCodeExpr extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/AuxCodeExpr")
        this.addRule([AriaGacParserNode_AuxFunc])
    }
}

class AriaGacParserNode_AuxCodeLine extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/AuxCodeLine")
        this.addRule([
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_GLSL_NOTE_SL)},
            AriaGacParserNode_AuxCodeExpr,
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_CHANGE_LINE)}
        ])
    }
    public generateCode(): string {
        let w = ""
        for(let i=1;i<this.children.length;i++){
            const child = this.children[i]
            if(child instanceof AriaGacLL1ParserNonterimalNode){
                w+=child.generateCode()
            }else{
                w+=child.content
            }
        }
        return w
    }
}

class AriaGacParserNode_EmptyLine extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/EmptyLine")
        this.addRule([
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_CHANGE_LINE)},
        ])
    }
}

class AriaGacParserNode_Spacing extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/Spacing")
        this.addRule([
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_SPACING)},
            AriaGacParserNode_Spacing
        ])
        this.addRule([
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_SPACING)},
        ])
    }
}

class AriaGacParserNode_ShaderCodeLineBody extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/ShaderCodeLineBody")
        this.addRule([
            (x)=>{return (x.tag!=AriaGacLexerTokenTypes.AGLT_CHANGE_LINE)},
            AriaGacParserNode_ShaderCodeLineBody
        ])
        this.addRule([])
    }
}

class AriaGacParserNode_ShaderCodeLine extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/ShaderCodeLine")
        this.addRule([
            (x)=>{
                return (x.tag!=AriaGacLexerTokenTypes.AGLT_CHANGE_LINE)&&
                    (x.tag!=AriaGacLexerTokenTypes.AGLT_SPACING)&&
                    (x.tag!=AriaGacLexerTokenTypes.AGLT_GLSL_NOTE_SL)&&
                    (x.tag!=AriaGacLexerTokenTypes.AGLT_END_SIGN)
            },
            AriaGacParserNode_ShaderCodeLineBody,
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_CHANGE_LINE)}
        ])
    }
}

class AriaGacParserNode_NoteContentBody extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/NoteContentBody")
        this.addRule([
            (x)=>{return (x.tag!=AriaGacLexerTokenTypes.AGLT_CHANGE_LINE)},
            AriaGacParserNode_NoteContentBody,
        ])
        this.addRule([])
    }
}

class AriaGacParserNode_NoteContent extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/NoteContent")
        this.addRule([
            (x)=>{return (x.tag!=AriaGacLexerTokenTypes.AGLT_CHANGE_LINE)&&(x.tag!=AriaGacLexerTokenTypes.AGLT_RESERVED_METHODS)},
            AriaGacParserNode_NoteContentBody,
        ])
    }
}


class AriaGacParserNode_NoteLine extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/NoteLine")
        this.addRule([
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_GLSL_NOTE_SL)},
            AriaGacParserNode_NoteContent,
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_CHANGE_LINE)},
        ])
    }
}

class AriaGacParserNode_EndLine extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/EndLine")
        this.addRule([
            (x)=>{return (x.tag==AriaGacLexerTokenTypes.AGLT_END_SIGN)},
        ])
    }
}




class AriaGacParserNode_CodeLine extends AriaGacSimpleParserNode{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser/CodeLine")
        this.addRule([AriaGacParserNode_NoteLine,AriaGacParserNode_CodeLine])
        this.addRule([AriaGacParserNode_ShaderCodeLine,AriaGacParserNode_CodeLine])
        this.addRule([AriaGacParserNode_Spacing,AriaGacParserNode_CodeLine])
        this.addRule([AriaGacParserNode_EmptyLine,AriaGacParserNode_CodeLine])
        this.addRule([AriaGacParserNode_AuxCodeLine,AriaGacParserNode_CodeLine])
        this.addRule([AriaGacParserNode_EndLine,AriaGacParserNode_CodeLine])
        this.addRule([
            (x)=>{return x.end_token==true}
        ])
        this.guaranteeValid=true
    }
}

export class AriaGacSimpleParser extends AriaGacLL1ParserBase{
    constructor(){
        super()
        this._rename("AriaGac/SimpleParser")
        this.setStartingNode(AriaGacParserNode_CodeLine)
    }
    public parse(x: IAriaGacLL1ParserInputToken[]): AriaGacLL1ParserNonterimalNode {
        x = x.concat([{
            tag:"",
            end_token:true,
            content:""
        }])
        return super.parse(x)
    }
}