export class AriaPageIndicator{
    private static instance:AriaPageIndicator|null
    private loader:HTMLDivElement
    private prog:HTMLSpanElement
    private con:HTMLSpanElement
    private constructor(){
        this.loader = document.createElement("div")
        const loader = this.loader
        loader.id = "aria-loader"
        loader.style.position = "fixed"
        loader.style.left = "0px"
        loader.style.top = "0px"
        loader.style.backgroundColor = "#000000"
        loader.style.color = "#ffffff"
        loader.innerHTML = ""
        document.getElementsByTagName("body")[0].appendChild(loader)

        this.con = document.createElement("span")
        this.con.id = "aria-load-con";
        (<HTMLElement>document.getElementById("aria-loader")).appendChild(this.con)

        this.prog = document.createElement("span")
        this.prog.id = "aria-load-prog";
        (<HTMLElement>document.getElementById("aria-loader")).appendChild(this.prog)
    }
    public static  getInstance(){
        if(this.instance==null){
            this.instance = new AriaPageIndicator()
        }
        return <AriaPageIndicator>this.instance
    }
    updateLoadingTip(x:string){
        this.con.innerHTML = "Loading... "+x
    }
    updateFPS(x:number){
        this.con.innerHTML = "FPS: "+ Math.ceil(x*100)/100
        this.prog.innerHTML = ""
    }
    updateLoadingProgress(x:number){
        this.prog.innerHTML = " ("+Math.ceil(x)+"%)"
    }
    done(){
        //this.loader.style.display = "none"
    }
}