export class AriaLoadIndicator{
    private static instance:AriaLoadIndicator|null
    private loader:HTMLDivElement
    private constructor(){
        this.loader = document.createElement("div")
        const loader = this.loader
        loader.id = "aria-loader"
        loader.style.position = "fixed"
        loader.style.left = "0px"
        loader.style.top = "0px"
        loader.style.backgroundColor = "#000000"
        loader.style.color = "#ffffff"
        loader.innerHTML = "Loading..."
        document.getElementsByTagName("body")[0].appendChild(loader)
    }
    public static  getInstance(){
        if(this.instance==null){
            this.instance = new AriaLoadIndicator()
        }
        return <AriaLoadIndicator>this.instance
    }
    updateLoadingTip(x:string){
        this.loader.innerHTML = "Loading... "+x
    }
    done(){
        this.loader.style.display = "none"
    }
}