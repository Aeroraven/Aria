import { AriaShaderCompilerLexerBase } from "../base/AriaCompilerLexer"

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
    AGLT_MOD = "agl_mod",
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

    AGLT_END_SIGN= "agl_ends",
    AGLT_MISC = "agl_misc"
}



export class AriaGacLexer extends AriaShaderCompilerLexerBase{
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
        this.addRule([recursive(tp.AGL_NUMERIC),"\x00-\\x2d|\x2f|\x3a-\uffff"],en.AGLT_INT,1)
        
        //brackets
        this.addRule(["[",tp.AGL_ANY],en.AGLT_LEFT_BRACKET,1)
        this.addRule(["]",tp.AGL_ANY],en.AGLT_RIGHT_BRACKET,1)
        this.addRule(["{",tp.AGL_ANY],en.AGLT_LEFT_BRACE,1)
        this.addRule(["}",tp.AGL_ANY],en.AGLT_RIGHT_BRACE,1)
        this.addRule(["(",tp.AGL_ANY],en.AGLT_LEFT_PARE,1)
        this.addRule([")",tp.AGL_ANY],en.AGLT_RIGHT_PARE,1)

        //words
        this.addRule(["_|a-z|A-z","\x00-\x2f|\x3a-\x40|\x5b-\x5e|\x60|\x7b-\uffff"],en.AGLT_WORD,1)
        this.addRule(["_|a-z|A-z",recursive("_|a-z|A-Z|0-9"),"\x00-\x2f|\x3a-\x40|\x5b-\x5e|\x60|\x7b-\uffff"],en.AGLT_WORD,1)

        //note starts
        this.addRule(["/","/",tp.AGL_ANY],en.AGLT_GLSL_NOTE_SL,1)

        //other ops
        this.addRule(["+","\x00-\x2a|\x2c-\x3c|\x3e-\uffff"],en.AGLT_ADD,1)
        this.addRule(["%",tp.AGL_ANY],en.AGLT_MOD,1)
        this.addRule(["\\--\\-","\x00-\x2c|\x2e-\x3c|\x3e-\uffff"],en.AGLT_SUB,1)
        this.addRule(["*","\x00-\x3c|\x3e-\uffff"],en.AGLT_MUL,1)
        this.addRule(["/","\x00-\x3c|\x3e-\uffff"],en.AGLT_DIV,1)
        this.addRule(["&","&",tp.AGL_ANY],en.AGLT_LAND,1)
        this.addRule(["\\|-\\|","\\|-\\|",tp.AGL_ANY],en.AGLT_LOR,1)
        this.addRule(["=","=",tp.AGL_ANY],en.AGLT_LEQ,1)
        this.addRule(["!","=",tp.AGL_ANY],en.AGLT_LNEQ,1)
        this.addRule([".",tp.AGL_ANY],en.AGLT_DOT,1)
        this.addRule([";",tp.AGL_ANY],en.AGLT_SEP,1)
        this.addRule([",",tp.AGL_ANY],en.AGLT_COMMA,1)
        this.addRule(["<","\x00-\x3c|\x3e-\uffff"],en.AGLT_LESS,1)
        this.addRule([">","\x00-\x3c|\x3e-\uffff"],en.AGLT_GREATER,1)

        this.addRule(["<","="],en.AGLT_LESS_EQ,1)
        this.addRule([">","="],en.AGLT_GREATER_EQ,1)

        this.addRule(["+","+",tp.AGL_ANY],en.AGLT_INC,1)
        this.addRule(["\\--\\-","\\--\\-",tp.AGL_ANY],en.AGLT_DEC,1)

        //assigns
        this.addRule(["=","\x00-\x3c|\x3e-\uffff"],en.AGLT_ASSIGN,1)
        this.addRule(["+","="],en.AGLT_ADD_ASSIGN,0)
        this.addRule(["\\--\\-","="],en.AGLT_SUB_ASSIGN,0)
        this.addRule(["*","="],en.AGLT_MUL_ASSIGN,0)
        this.addRule(["/","="],en.AGLT_DIV_ASSIGN,0)

        //spacings & change line
        this.addRule([recursive(" |\t"),"\x00-\x08|\x0a-\x1f|\x21-\uffff"],en.AGLT_SPACING,1)
        this.addRule(["\n",tp.AGL_ANY],en.AGLT_CHANGE_LINE,1)
        this.addRule(["\r","\n",tp.AGL_ANY],en.AGLT_CHANGE_LINE,1)

        //pragma
        this.addRule(["#",tp.AGL_ANY],en.AGLT_SHARP,1)

        //misc
        this.addRule([":",tp.AGL_ANY],en.AGLT_COLON,1)
        this.addRule(["\x00"],en.AGLT_END_SIGN,0)

        this.addRule(["\u0000-\uffff",tp.AGL_ANY],en.AGLT_MISC,1)
    }
}