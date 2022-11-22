export class AriaLogger{
    public static inst:AriaLogger|null = null
    public static getInstance(){
        if(this.inst==null){
            this.inst = new AriaLogger()
        }
        return <AriaLogger>this.inst
    }
    constructor(){

    }
    public log(content:string, level:AriaLogLevel, initiator:string){
        let style = ""
        if(level==AriaLogLevel.WARN){
            style = "color:#ff9900"
        }
        if(level==AriaLogLevel.INFO){
            style = "color:#0099ff"
        }
        if(level==AriaLogLevel.ERROR){
            style = "color:#ff0000"
        }
        if(level!=AriaLogLevel.ERROR){
            console.log("%c[Aria2] ("+level+") <"+initiator+"> "+content,style)
        }else{
            console.trace("%c[Aria2] ("+level+") <"+initiator+"> "+content,style)
        }
        
        if(level == AriaLogLevel.ERROR){
            window.alert("Aria Unexpected Event: "+initiator+"\n"+content)
            throw Error("Aria Unexpected Event: "+initiator+"\n"+content)
        }
    }
}

export enum AriaLogLevel{
    WARN = "Warn",
    INFO = "Info",
    ERROR = "Error"
}