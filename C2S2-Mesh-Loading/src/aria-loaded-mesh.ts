export interface AriaLoadMeshDataStruct{
    v:number[]
    vt:number[]
    vn:number[]
    vc:number[]
    f:number[]
}

export class AriaLoadedMesh{
    vList:number[]
    vtList:number[]
    vnList:number[]
    fvList:number[]
    fvnList:number[]
    fvtList:number[]

    constructor(){
        this.vList = []
        this.vtList = []
        this.vnList = []
        this.fvnList = []
        this.fvList = []
        this.fvtList = []
    }
    exportData(){
        const vlst:number[] = []
        const vnlst:number[] = []
        const vclst:number[] = []
        const vtlst:number[] = []
        const flst:number[] = []
        for(let i=0;i<this.fvList.length;i++){
            let vidx = this.fvList[i]
            vlst.push(this.vList[3*vidx])
            vlst.push(this.vList[3*vidx+1])
            vlst.push(this.vList[3*vidx+2])

            let vtidx = this.fvtList[i]
            vtlst.push(this.vtList[3*vtidx])
            vtlst.push(this.vtList[3*vtidx+1])
            //vtlst.push(this.vtList[3*vtidx+2])

            let vnidx = this.fvnList[i]
            vnlst.push(this.vnList[3*vidx])
            vnlst.push(this.vnList[3*vidx+1])
            vnlst.push(this.vnList[3*vidx+2])

            vclst.push(1)
            vclst.push(0)
            vclst.push(0)
            vclst.push(1)

            flst.push(i)
        }
        return <AriaLoadMeshDataStruct>{
            v:vlst,
            vn:vnlst,
            vt:vtlst,
            vc:vclst,
            f:flst
        }
    }
    loadFromWavefront(src:string){
        const fsrc = src.replace(/\r/g,'').replace(/[ ]+/g,' ')
        const lines = fsrc.split('\n')
        for(let i=0;i<lines.length;i++){
            let token = lines[i].split(' ')
            if(token[0]=='v'){
                this.vList.push(parseFloat(token[1]))
                this.vList.push(parseFloat(token[2]))
                this.vList.push(parseFloat(token[3]))
            }
            if(token[0]=="vt"){
                this.vtList.push(parseFloat(token[1]))
                this.vtList.push(parseFloat(token[2]))
                this.vtList.push(parseFloat(token[3]))
            }
            if(token[0]=="vn"){
                this.vnList.push(parseFloat(token[1]))
                this.vnList.push(parseFloat(token[2]))
                this.vnList.push(parseFloat(token[3]))
            }
            if(token[0]=="f"){
                let slc:number[] = []
                for(let j=1;j<=3;j++){
                    let w = token[j].split('/')
                    slc.push(parseInt(w[0]))
                    slc.push(parseInt(w[1]))
                    slc.push(parseInt(w[2]))
                }
                for(let j=0;j<9;j+=3){
                    this.fvList.push(slc[j]-1)
                    this.fvtList.push(slc[j+1]-1)
                    this.fvnList.push(slc[j+2]-1)
                }
            }
        }
        console.log(lines)
    }
}