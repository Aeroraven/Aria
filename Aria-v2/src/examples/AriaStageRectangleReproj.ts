import { mat4, vec3, vec4 } from "gl-matrix-ts";
import { AriaComCamera } from "../components/camera/AriaComCamera";
import { AriaComRectangle } from "../components/geometry/primary/AriaComRectangle";
import { AriaComMesh } from "../components/mesh/AriaComMesh";
import { AriaWGL2Renderer } from "../components/renderer/AriaWGL2Renderer";
import { AriaComScene } from "../components/scene/AriaComScene";
import { AriaComParamPanel } from "../components/ui/panel/AriaComParamPanel";
import { AriaSimpleMaterial } from "../presets/materials/AriaSimpleMaterial";
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
        panel.addTitle("空间法向恢复")
        panel.addFPSMeter("帧率")
        panel.addSlidebar("视场角 (Y轴)",10.0,170.0,(x)=>{
            camera.setFov(x)
        })
        panel.initInteraction()

        camera.initInteraction();

        (<any>(window)).localMat = geometry._localMat;
        panel.addDynamicText("模型矩阵",()=>"在F12控制台中修改变量 `localMat` ")
        panel.addDynamicText("相机转角",()=>"鼠标移动")
        panel.addDynamicText("相机位置",()=>"键盘:AD-X轴，WS-Z轴，QE-Y轴")
        panel.addDynamicText("冻结相机",()=>"键盘:ESC")

        // Tracking Vertex Points (Projection Procedure)
        const ptsRaw = [[-1, -1, -1, 1], [1, -1, -1, 1], [1,  1, -1, 1], [-1,  1, -1, 1]]
        let originalCoords:Float32Array[] = []
        let cameraSpaceCoords:Float32Array[] = []
        let screenSpaceCoords:Float32Array[] = []
        let retinaSpaceCoords:Float32Array[] = []
        let recoveredCoords:Float32Array[] = []
        let vsNormal = vec4.create()
        let vsDotV = 0
        let rsNormal = vec4.create()
        let rsDotV = 0

        for(let i=0;i<4;i++){
            originalCoords.push(vec4.create())
            cameraSpaceCoords.push(vec4.create())
            screenSpaceCoords.push(vec4.create())
            retinaSpaceCoords.push(vec4.create())
            recoveredCoords.push(vec4.create())
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

        panel.addDynamicText("邻边点积",()=>{return  vsDotV+""})

        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("标准化设备空间坐标NDC",()=>"",true)
        panel.addDynamicText("左下顶点",()=>{return  f32VecToString(screenSpaceCoords[0])})
        panel.addDynamicText("右下顶点",()=>{return  f32VecToString(screenSpaceCoords[1])})
        panel.addDynamicText("右上顶点",()=>{return  f32VecToString(screenSpaceCoords[2])})
        panel.addDynamicText("左上顶点",()=>{return  f32VecToString(screenSpaceCoords[3])})

        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("恢复后归一化平面坐标",()=>"",true)
        panel.addDynamicText("左下顶点",()=>{return  f32VecToString(retinaSpaceCoords[0])})
        panel.addDynamicText("右下顶点",()=>{return  f32VecToString(retinaSpaceCoords[1])})
        panel.addDynamicText("右上顶点",()=>{return  f32VecToString(retinaSpaceCoords[2])})
        panel.addDynamicText("左上顶点",()=>{return  f32VecToString(retinaSpaceCoords[3])})

        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("可行空间矩形顶点",()=>"",true)
        panel.addDynamicText("左下顶点",()=>{return  f32VecToString(recoveredCoords[0])})
        panel.addDynamicText("右下顶点",()=>{return  f32VecToString(recoveredCoords[1])})
        panel.addDynamicText("左上顶点",()=>{return  f32VecToString(recoveredCoords[3])})
        
        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("法向量",()=>"",true)
        panel.addDynamicText("原始法向量",()=>{return  f32VecToString(vsNormal)})
        panel.addDynamicText("恢复法向量",()=>{return  f32VecToString(rsNormal)})


        panel.addDynamicText("&nbsp;",()=>"",true)
        panel.addDynamicText("参数",()=>"",true)
        panel.addDynamicText("宽高比",()=>{return  camera.aspectRatio+""})
        panel.addDynamicText("Y焦距",()=>{
            let fovY = camera.fovAngle / 180 * Math.PI;
            return 1.0/ Math.tan(fovY/2)+""
        })

        let updateCameraSpaceCoords = ()=>{
            for(let i=0;i<4;i++){
                vec4.transformMat4(cameraSpaceCoords[i],originalCoords[i],camera.getLookAt())
                vec4.transformMat4(screenSpaceCoords[i],cameraSpaceCoords[i],camera.getPerspective())
                let t = screenSpaceCoords[i][3]
                vec4.scale(screenSpaceCoords[i],screenSpaceCoords[i],1.0/t)
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

            vsDotV = vec3.dot(vsNa,vsNb)
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
                retinaSpaceCoords[i][2] = -1.0;
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

            rsDotV = vec3.dot(vsNa,vsNb)

        }
        panel.addDynamicText("可行解S0",()=>{return  s0+""})
        panel.addDynamicText("可行解T0",()=>{return  t0+""})

        const renderCall = ()=>{
            updateCameraSpaceCoords()
            recoverRetina()
            renderer.renderScene(camera,scene);
            panel.reqAniFrame(renderCall)
        }
        renderCall()
    }
}