import { AriaComFractalNoise } from "../components/math/noise/AriaComFractalNoise";
import { AriaComImprovedNoise } from "../components/math/noise/AriaComImprovedNoise";
import { AriaStage } from "./AriaStage";

export class AriaStageNoiseTest extends AriaStage{
    constructor(){
        super("AriaStage/NoiseTest")
    }
    public async entry(): Promise<void> {
        let wx = new AriaComImprovedNoise();
        let w = new AriaComFractalNoise(wx,5);
        (<any>window).pl = w;
        (<HTMLCanvasElement>document.getElementById("webgl_displayer")).style.display="none"
        let canvas = document.createElement("canvas")
        canvas.height = 300
        canvas.width = 300
        let body = document.getElementsByTagName("body")[0]
        body.appendChild(canvas)

        let contex = canvas.getContext("2d")!
        for(let x=0;x<200;x++){
            for(let y=0;y<200;y++){
                let noise = 0.5+0.5*w.noise(x*0.02,y*0.02,0);
                let data = contex.createImageData(1,1)
                data.data[0] = Math.floor(noise*255)
                data.data[1] = data.data[0]
                data.data[2] = data.data[1]
                data.data[3] = 255
                contex.putImageData(data,x,y)
            }
        }

    }
}