import { vec3, vec4 } from "gl-matrix-ts";
import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComRectangle } from "../components/geometry/primary/AriaComRectangle";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaSingleColorMaterial } from "../presets/materials/AriaSingleColorMaterial";
import { AriaStage } from "./AriaStage";

export class AriaStageRectangleReproj extends AriaStage{
    constructor(){
        super("AriaStage/RectangleReproj")
    }
    public async entry(): Promise<void> {
        //Renderer  
        const renderer = new AriaWGL2Renderer("webgl_displayer")
        
        const geometry = new AriaComRectangle()
        const camera = new AriaComCamera()
        
        camera.setPos(0,0,5)
        const material = new AriaSingleColorMaterial()
        material.setColor(1,0,0,1)

        const mesh = new AriaComMesh(material,geometry)
        const scene = new AriaComScene()

        scene.addChild(mesh)

        const panel =  new AriaComParamPanel(200,270)
        //panel.addTitle("通过矩形投影后的顶点恢复法向量")
        panel.addFPSMeter("FPS")
        panel.addSlidebar("视场角Y（控制焦距）",10.0,170.0,(x)=>{
            camera.setFov(x)
        })
        panel.initInteraction()

        camera.initInteraction();

        (<any>(window)).localMat = geometry._localMat;
        panel.addDynamicText("模型矩阵",()=>"控制台修改")
        panel.addDynamicText("相机旋转",()=>"Mouse")
        panel.addDynamicText("相机位置",()=>"AD-X，WS-Z，QE-Y")
        panel.addDynamicText("冻结相机",()=>"ESC")

        // Tracking Vertex Points (Projection Procedure)
        const ptsRaw = [[-1, -1, -1, 1], [1, -1, -1, 1], [1,  1, -1, 1], [-1,  1, -1, 1]]
        let originalCoords:Float32Array[] = []
        let cameraSpaceCoords:Float32Array[] = []
        let screenSpaceCoords:Float32Array[] = []
        let screenSpaceCoords2:Float32Array[] = []
        let retinaSpaceCoords:Float32Array[] = []
        let recoveredCoords:Float32Array[] = []
        let vsNormal = vec4.create()
        let rsNormal = vec4.create()

        for(let i=0;i<4;i++){
            originalCoords.push(vec4.create())
            cameraSpaceCoords.push(vec4.create())
            screenSpaceCoords.push(vec4.create())
            retinaSpaceCoords.push(vec4.create())
            recoveredCoords.push(vec4.create())
            screenSpaceCoords2.push(vec4.create())
            for(let j=0;j<4;j++){
                originalCoords[i][j] = ptsRaw[i][j]
            }
        }

        let f32VecToString = (x:Float32Array)=>{
            let ret="["
            for(let i=0;i<x.length;i++){
                ret+=x[i].toFixed(4)+", "
            }
            ret+="]"
            return ret;
        }
        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("相机空间坐标",()=>"",true)
        panel.addDynamicText("左下顶点",()=>{return  f32VecToString(cameraSpaceCoords[0])})
        panel.addDynamicText("右下顶点",()=>{return  f32VecToString(cameraSpaceCoords[1])})
        panel.addDynamicText("右上顶点",()=>{return  f32VecToString(cameraSpaceCoords[2])})
        panel.addDynamicText("左上顶点",()=>{return  f32VecToString(cameraSpaceCoords[3])})

        //panel.addDynamicText("Dot Prod",()=>{return  vsDotV+""})

        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("像素坐标",()=>"",true)
        panel.addDynamicText("左下顶点",()=>{return  f32VecToString(screenSpaceCoords2[0])})
        panel.addDynamicText("右下顶点",()=>{return  f32VecToString(screenSpaceCoords2[1])})
        panel.addDynamicText("右上顶点",()=>{return  f32VecToString(screenSpaceCoords2[2])})
        panel.addDynamicText("左上顶点",()=>{return  f32VecToString(screenSpaceCoords2[3])})

        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("恢复后的归一化平面坐标",()=>"",true)
        panel.addDynamicText("左下顶点",()=>{return  f32VecToString(retinaSpaceCoords[0])})
        panel.addDynamicText("右下顶点",()=>{return  f32VecToString(retinaSpaceCoords[1])})
        panel.addDynamicText("右上顶点",()=>{return  f32VecToString(retinaSpaceCoords[2])})
        panel.addDynamicText("左上顶点",()=>{return  f32VecToString(retinaSpaceCoords[3])})

        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("恢复后的可行矩形顶点坐标",()=>"",true)
        panel.addDynamicText("左下顶点",()=>{return  f32VecToString(recoveredCoords[0])})
        panel.addDynamicText("右下顶点",()=>{return  f32VecToString(recoveredCoords[1])})
        panel.addDynamicText("左上顶点",()=>{return  f32VecToString(recoveredCoords[3])})
        
        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("法向量",()=>"",true)
        panel.addDynamicText("原始的法向量",()=>{return  f32VecToString(vsNormal)})
        panel.addDynamicText("算法恢复的法向量",()=>{return  f32VecToString(rsNormal)})


        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("参数",()=>"",true)
        panel.addDynamicText("宽高比",()=>{return  camera.aspectRatio+""})
        panel.addDynamicText("焦距 Y",()=>{
            let fovY = camera.fovAngle / 180 * Math.PI;
            return 1.0/ Math.tan(fovY/2)+""
        })

        let updateCameraSpaceCoords = ()=>{
            for(let i=0;i<4;i++){
                vec4.transformMat4(cameraSpaceCoords[i],originalCoords[i],camera.getLookAt())
                vec4.transformMat4(screenSpaceCoords[i],cameraSpaceCoords[i],camera.getPerspective())
                let t = screenSpaceCoords[i][3]
                vec4.scale(screenSpaceCoords[i],screenSpaceCoords[i],1.0/t)

                cameraSpaceCoords[i][2] = -cameraSpaceCoords[i][2]
                screenSpaceCoords[i][2] = -screenSpaceCoords[i][2]

                screenSpaceCoords2[i][0] = (screenSpaceCoords[i][0]+1.0)*0.5*window.innerWidth
                screenSpaceCoords2[i][1] = (screenSpaceCoords[i][1]+1.0)*0.5*window.innerHeight
                screenSpaceCoords2[i][2] = 1.0
                screenSpaceCoords2[i][3] = 1.0
            }

            let vsNa = vec3.create()
            vsNa[0] = cameraSpaceCoords[1][0] -  cameraSpaceCoords[0][0]
            vsNa[1] = cameraSpaceCoords[1][1] -  cameraSpaceCoords[0][1]
            vsNa[2] = cameraSpaceCoords[1][2] -  cameraSpaceCoords[0][2]

            let vsNb = vec3.create()
            vsNb[0] = cameraSpaceCoords[3][0] -  cameraSpaceCoords[0][0]
            vsNb[1] = cameraSpaceCoords[3][1] -  cameraSpaceCoords[0][1]
            vsNb[2] = cameraSpaceCoords[3][2] -  cameraSpaceCoords[0][2]

            vec3.cross(vsNormal,vsNa,vsNb);
            vec3.normalize(vsNormal,vsNormal)

        }

        let s0 = 0,t0 = 0;
        // Reverse tracking from NDC Coordinates
        let recoverRetina = ()=>{
            let fovY = camera.fovAngle / 180 * Math.PI;
            let focalBase = 1.0/ Math.tan(fovY/2);
            let aspect = camera.aspectRatio
            let focalX = focalBase/aspect
            let focalY = focalBase

            for(let i=0;i<4;i++){
                retinaSpaceCoords[i][0] = screenSpaceCoords[i][0] / focalX;
                retinaSpaceCoords[i][1] = screenSpaceCoords[i][1] / focalY;
                retinaSpaceCoords[i][2] = 1.0;
                retinaSpaceCoords[i][3] = 1.0;
            }

            let a = retinaSpaceCoords[0][0], b=retinaSpaceCoords[1][0], c=retinaSpaceCoords[2][0],d=retinaSpaceCoords[3][0]
            let m = retinaSpaceCoords[0][1], n=retinaSpaceCoords[1][1], p=retinaSpaceCoords[2][1],q=retinaSpaceCoords[3][1]

            let deno = -b*p+b*q+c*n-c*q-d*n+d*p;
            let sNum = -a*n+a*p+b*m-b*p-c*m+c*n;
            let tNum = -a*p+a*q+c*m-c*q-d*m+d*p;

            s0 = sNum/deno
            t0 = tNum/deno

            //Recover E
            recoveredCoords[0][0] = retinaSpaceCoords[0][0]
            recoveredCoords[0][1] = retinaSpaceCoords[0][1]
            recoveredCoords[0][2] = retinaSpaceCoords[0][2]
            recoveredCoords[0][3] = 1

            //Recover H
            recoveredCoords[3][0] = retinaSpaceCoords[3][0]*s0
            recoveredCoords[3][1] = retinaSpaceCoords[3][1]*s0
            recoveredCoords[3][2] = retinaSpaceCoords[3][2]*s0
            recoveredCoords[3][3] = 1

            //Recover F
            recoveredCoords[1][0] = retinaSpaceCoords[1][0]*t0
            recoveredCoords[1][1] = retinaSpaceCoords[1][1]*t0
            recoveredCoords[1][2] = retinaSpaceCoords[1][2]*t0
            recoveredCoords[1][3] = 1


            let vsNa = vec3.create()
            vsNa[0] = recoveredCoords[1][0] -  recoveredCoords[0][0]
            vsNa[1] = recoveredCoords[1][1] -  recoveredCoords[0][1]
            vsNa[2] = recoveredCoords[1][2] -  recoveredCoords[0][2]

            let vsNb = vec3.create()
            vsNb[0] = recoveredCoords[3][0] -  recoveredCoords[0][0]
            vsNb[1] = recoveredCoords[3][1] -  recoveredCoords[0][1]
            vsNb[2] = recoveredCoords[3][2] -  recoveredCoords[0][2]

            vec3.cross(rsNormal,vsNa,vsNb);
            vec3.normalize(rsNormal,rsNormal)


        }
        panel.addDynamicText("解 S0",()=>{return  s0+""})
        panel.addDynamicText("解 T0",()=>{return  t0+""})

        const renderCall = ()=>{
            updateCameraSpaceCoords()
            recoverRetina()
            renderer.renderScene(camera,scene);
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
}