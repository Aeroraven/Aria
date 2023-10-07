export class AriaPageIndicator{
    private static instance:AriaPageIndicator|null
    private loader:HTMLDivElement
    private prog:HTMLSpanElement
    private con:HTMLSpanElement
    private sceneName:string
    private subTitle:HTMLDivElement
    private subTitleContent:string
    private constructor(){
        this.sceneName = ""
        this.subTitleContent = ""
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

        this.subTitle = document.createElement("div")
        const subTitle = this.subTitle
        subTitle.id = "aria-subtitle"
        subTitle.style.position = "fixed"
        subTitle.style.left = "0px"
        subTitle.style.bottom = "0px"
        subTitle.style.backgroundColor = "#000000"
        subTitle.style.color = "#ffffff"
        subTitle.innerHTML = ""
        subTitle.style.fontSize = "12px"
        document.getElementsByTagName("body")[0].appendChild(subTitle)

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
        this.con.innerHTML = "<b>"+this.sceneName+"</b><br/>FPS: "+ Math.ceil(x*100)/100
        this.prog.innerHTML = ""
    }
    setSceneName(x:string){
        this.sceneName = x
    }
    updateLoadingProgress(x:number){
        this.prog.innerHTML = " ("+Math.ceil(x)+"%)"
    }
    setSubTitle(x:string){
        this.subTitleContent = x
        this.subTitle.innerHTML = x
    }
    done(){
        //this.loader.style.display = "none"
    }
}