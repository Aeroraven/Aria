import { AriaShaderCompilerLexerBase } from "../base/AriaCompilerLexer"

export enum AriaWctLexerTokenTypes{
    AWLT_RESERVED_METHODS = "agl_reserved_methods",
    AWLT_FLOAT = "agl_float",
    AWLT_INT = "agl_int",
    AWLT_LEFT_BRACKET = "agl_left_bracket",
    AWLT_RIGHT_BRACKET = "agl_right_bracket",
    AWLT_WORD = "agl_word",

    AWLT_GLSL_NOTE_SL = "agl_glsl_note_sl",
    AWLT_LEFT_BRACE = "agl_left_brace",
    AWLT_RIGHT_BRACE = "agl_left_brace",
    AWLT_LEFT_PARE = "agl_left_pare",
    AWLT_RIGHT_PARE = "agl_left_pare",

    AWLT_ADD = "agl_add",
    AWLT_SUB = "agl_sub",
    AWLT_MUL = "agl_mul",
    AWLT_DIV = "agl_div",
    AWLT_LAND = "agl_land",
    AWLT_LOR = "agl_lor",
    AWLT_LNOT = "agl_lnot",
    AWLT_LEQ = "agl_leq",
    AWLT_LNEQ = "agl_lneq",
    AWLT_LESS = "agl_less",
    AWLT_GREATER = "agl_greater",
    AWLT_LESS_EQ = "agl_less_eq",
    AWLT_GREATER_EQ = "agl_greater_eq",

    AWLT_ASSIGN = "agl_assign",
    AWLT_ADD_ASSIGN = "agl_add_assign",
    AWLT_SUB_ASSIGN = "agl_sub_assign",
    AWLT_MUL_ASSIGN = "agl_mul_assign",
    AWLT_DIV_ASSIGN = "agl_div_assign",
    AWLT_CHANGE_LINE = "agl_cl",

    AWLT_COMMA = "agl_comma",
    AWLT_SEP = "agl_sep",
    AWLT_DOT = "agl_dot",

    AWLT_SPACING = "agl_spacing",

    AWLT_SHARP = "agl_sharp",
    AWLT_COLON = "agl_colon",

    AWLT_INC = "agl_inc",
    AWLT_DEC = "agl_dec",

    AWLT_END_SIGN= "agl_ends",

    AWLT_IF = "agl_if",
    AWLT_ELSE = "agl_else",
    AWLT_FOR = "agl_for",
    AWLT_WHILE = "agl_while",
    AWLT_BREAK = "agl_break",
    AWLT_CONTINUE = "agl_continue",
    AWLT_DISCARD = "agl_discard",
    
    AWLT_OUT = "agl_out",
    AWLT_IN = "agl_in",
    AWLT_INOUT = "agl_inout",

    AWLT_ATTRIBUTE = "agl_attribute",
    AWLT_UNIFORM = "agl_uniform",
    AWLT_VARYING = "agl_varying",

    AWLT_TP_VOID = "agl_void",
    AWLT_TP_INT = "agl_int",
    AWLT_TP_FLOAT = "agl_float",
    AWLT_TP_DOUBBLE = "agl_double",
    AWLT_TP_BOOL = "agl_bool",

    AWLT_TP_VEC2 = "agl_vec2",
    AWLT_TP_VEC3 = "agl_vec3",
    AWLT_TP_VEC4 = "agl_vec4",
    
    AWLT_TP_UVEC2 = "agl_uvec2",
    AWLT_TP_UVEC3 = "agl_uvec3",
    AWLT_TP_UVEC4 = "agl_uvec4",
    
    AWLT_TP_MAT2 = "agl_mat2",
    AWLT_TP_MAT3 = "agl_mat3",
    AWLT_TP_MAT4 = "agl_mat4",

    AWLT_TRUE = "agl_true",
    AWLT_FALSE = "agl_false"
}



export class AriaWctLexer extends AriaShaderCompilerLexerBase{
    constructor(){
        super()
        this._rename("AriaWct/Lexer")
        //decorators
        const recursive=(x:string)=>{return "("+x+")"}
        const tp = AriaWctLexer
        const en = AriaWctLexerTokenTypes

        //Reserved methods
        const rmPrefix = ["!","@","a","s","u"]
        this.addRule(rmPrefix.concat(recursive(tp.AGL_ALPHABETIC)).concat(tp.AGL_NON_ALPHABETIC),en.AWLT_RESERVED_METHODS,1)

        //numbers
        this.addRule([recursive(tp.AGL_NUMERIC),".",recursive(tp.AGL_NUMERIC),tp.AGL_NON_NUMERIC],en.AWLT_FLOAT,1)
        this.addRule([recursive(tp.AGL_NUMERIC),"\x00-\\x2d|\x2f|\x3a-\xff"],en.AWLT_INT,1)
        
        //brackets
        this.addRule(["[",tp.AGL_ANY],en.AWLT_LEFT_BRACKET,1)
        this.addRule(["]",tp.AGL_ANY],en.AWLT_RIGHT_BRACKET,1)
        this.addRule(["{",tp.AGL_ANY],en.AWLT_LEFT_BRACE,1)
        this.addRule(["}",tp.AGL_ANY],en.AWLT_RIGHT_BRACE,1)
        this.addRule(["(",tp.AGL_ANY],en.AWLT_LEFT_PARE,1)
        this.addRule([")",tp.AGL_ANY],en.AWLT_RIGHT_PARE,1)

        //words
        this.addRule(["_|a-z|A-z","\x00-\x2f|\x3a-\x40|\x5b-\x5e|\x60|\x7b-\xff"],en.AWLT_WORD,1)
        this.addRule(["_|a-z|A-z",recursive("_|a-z|A-Z|0-9"),"\x00-\x2f|\x3a-\x40|\x5b-\x5e|\x60|\x7b-\xff"],en.AWLT_WORD,1)

        //note starts
        this.addRule(["/","/",tp.AGL_ANY],en.AWLT_GLSL_NOTE_SL,1)

        //other ops
        this.addRule(["+","\x00-\x2a|\x2c-\x3c|\x3e-\xff"],en.AWLT_ADD,1)
        this.addRule(["\\--\\-","\x00-\x2c|\x2e-\x3c|\x3e-\xff"],en.AWLT_SUB,1)
        this.addRule(["*","\x00-\x3c|\x3e-\xff"],en.AWLT_MUL,1)
        this.addRule(["/","\x00-\x3c|\x3e-\xff"],en.AWLT_DIV,1)
        this.addRule(["&","&",tp.AGL_ANY],en.AWLT_LAND,1)
        this.addRule(["\\|-\\|","\\|-\\|",tp.AGL_ANY],en.AWLT_LOR,1)
        this.addRule(["=","=",tp.AGL_ANY],en.AWLT_LEQ,1)
        this.addRule(["!","=",tp.AGL_ANY],en.AWLT_LNEQ,1)
        this.addRule([".",tp.AGL_ANY],en.AWLT_DOT,1)
        this.addRule([";",tp.AGL_ANY],en.AWLT_SEP,1)
        this.addRule([",",tp.AGL_ANY],en.AWLT_COMMA,1)
        this.addRule(["<","\x00-\x3c|\x3e-\xff"],en.AWLT_LESS,1)
        this.addRule([">","\x00-\x3c|\x3e-\xff"],en.AWLT_GREATER,1)

        this.addRule(["<","="],en.AWLT_LESS_EQ,1)
        this.addRule([">","="],en.AWLT_GREATER_EQ,1)

        this.addRule(["+","+",tp.AGL_ANY],en.AWLT_INC,1)
        this.addRule(["\\--\\-","\\--\\-",tp.AGL_ANY],en.AWLT_DEC,1)

        //assigns
        this.addRule(["=","\x00-\x3c|\x3e-\xff"],en.AWLT_ASSIGN,1)
        this.addRule(["+","="],en.AWLT_ADD_ASSIGN,0)
        this.addRule(["\\--\\-","="],en.AWLT_SUB_ASSIGN,0)
        this.addRule(["*","="],en.AWLT_MUL_ASSIGN,0)
        this.addRule(["/","="],en.AWLT_DIV_ASSIGN,0)

        //spacings & change line
        this.addRule([recursive(" |\t"),"\x00-\x08|\x0a-\x1f|\x21-\xff"],en.AWLT_SPACING,1)
        this.addRule(["\n",tp.AGL_ANY],en.AWLT_CHANGE_LINE,1)
        this.addRule(["\r","\n",tp.AGL_ANY],en.AWLT_CHANGE_LINE,1)

        //pragma
        this.addRule(["#",tp.AGL_ANY],en.AWLT_SHARP,1)

        //misc
        this.addRule([":",tp.AGL_ANY],en.AWLT_COLON,1)
        this.addRule(["\x00"],en.AWLT_END_SIGN,0)
    }
}