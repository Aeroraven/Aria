export interface AriaLoadMeshDataStruct{
    v:number[]
    vt:number[]
    vn:number[]
    vc:number[]
    f:number[]
    tan:number[]
    bitan:number[]
}

export interface AriaTextureTangentStruct{
    t:number[]
    b:number[]
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
    getTangent(a:number,b:number,c:number){
        let va = this.fvList[a]
        let vb = this.fvList[b]
        let vc = this.fvList[c]
        let pa = [this.vList[3*va],this.vList[3*va+1],this.vList[3*va+2]]
        let pb = [this.vList[3*vb],this.vList[3*vb+1],this.vList[3*vb+2]]
        let pc = [this.vList[3*vc],this.vList[3*vc+1],this.vList[3*vc+2]]

        let vta = this.fvtList[a]
        let vtb = this.fvtList[b]
        let vtc = this.fvtList[c]
        let uva = [this.vtList[3*vta],this.vtList[3*vta+1]]
        let uvb = [this.vtList[3*vtb],this.vtList[3*vtb+1]]
        let uvc = [this.vtList[3*vtc],this.vtList[3*vtc+1]]

        //AB = (ub-ua)*T+(vb-va)*B
        //AC = (uc-ua)*T+(vc-va)*B

        //Bi-tangent
        //(uc-ua)AB=(uc-ua)(ub-ua)*T+(vb-va)(uc-ua)B
        //(ub-ua)AC=(uc-ua)(ub-ua)*T+(vc-va)(ub-ua)B
        //B[(vb-va)(uc-ua)-(vc-va)(ub-ua)]=AB(uc-ua)-AC(ub-ua)
        //B = [ AB(uc-ua)-AC(ub-ua) ] / [(vb-va)(uc-ua)-(vc-va)(ub-ua)]

        //Tangent
        //(vc-va)AB = (vc-va)(ub-ua)*T+(vb-va)(vc-va)*B
        //(vb-va)AC = (vb-va)(uc-ua)*T+(vc-va)(vb-va)*A
        // [(vc-va)(ub-ua)-(vb-va)(uc-ua)]*T = (vc-va)*AB-(vb-va)*AC
        // T = [(vc-va)*AB-(vb-va)*AC] / [(vc-va)(ub-ua)-(vb-va)(uc-ua)]

        let tangent = [0,0,0]
        let bitangent = [0,0,0]
        let tan_deno = (uvc[1]-uva[1])*(uvb[0]-uva[0]) - (uvb[1]-uva[1])*(uvc[0]-uva[0])
        let bitan_deno = (uvb[1]-uva[1])*(uvc[0]-uva[0]) - (uvc[1]-uva[1])*(uvb[0]-uva[0]) 
        for(let i=0;i<3;i++){
            tangent[i] = ((uvc[1]-uva[1])*(pb[i]-pa[i]) - (uvb[1]-uva[1]) * (pc[i]-pa[i])) / tan_deno
            bitangent[i] = ((uvc[0]-uva[0])*(pb[i]-pa[i]) - (uvb[0]-uva[0])*(pc[i]-pa[i])) / bitan_deno
        }

        let ret = <AriaTextureTangentStruct>{
            t:tangent,
            b:bitangent
        }
        return ret
    }
    exportData(){
        const vlst:number[] = []
        const vnlst:number[] = []
        const vclst:number[] = []
        const vtlst:number[] = []
        const flst:number[] = []

        //Tangent
        const vtanlst:number[] = []
        const vbitlst:number[] = [] 

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

        for(let i=0;i<this.fvList.length;i+=3){
            const w = this.getTangent(i,i+1,i+2)
            for(let j=0;j<3;j++){
                vtanlst.push(w.t[0])
                vtanlst.push(w.t[1])
                vtanlst.push(w.t[2])

                vbitlst.push(w.b[0])
                vbitlst.push(w.b[1])
                vbitlst.push(w.b[2])
            }
        }

        return <AriaLoadMeshDataStruct>{
            v:vlst,
            vn:vnlst,
            vt:vtlst,
            vc:vclst,
            f:flst,
            tan:vtanlst,
            bitan:vbitlst
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