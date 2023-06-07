import { AriaObject } from "../base/AriaObject";
import { AriaArithmetic } from "./AriaArithmetic";

export class AriaCompGeo extends AriaObject{
    constructor(){
        super("AriaCompGeo")
    }
    static lineSegmentIntersects(xa1:number,ya1:number,xa2:number,ya2:number,xb1:number,yb1:number,xb2:number,yb2:number){
        let Ax = xa2-xa1
        let Ay = ya2-ya1
        let Bx = xb2-xb1
        let By = yb2-yb1
        return (AriaArithmetic.cross2(Ax,Ay,xb1-xa1,yb1-ya1) * AriaArithmetic.cross2(Ax,Ay,xb2-xa1,yb2-ya1))<=0 && 
            (AriaArithmetic.cross2(Bx,By,xa1-xb1,ya1-yb1) * AriaArithmetic.cross2(Bx,By,xa2-xb1,ya2-yb1))<=0
    }
    static lineSegmentIntersectsCube(segment:number[][],xmin:number,xmax:number,ymin:number,ymax:number,zmin:number,zmax:number){
        let inRange = (x:number,a:number,b:number)=>{return x>=a&&x<=b}
        //YoX Projection
        let yoxCond = ()=>{
            let ax = segment[0][0]
            let ay = segment[0][1]
            let bx = segment[1][0]
            let by = segment[1][1]
            if(inRange(ax,xmin,xmax)&&inRange(ay,ymin,ymax)){
                return true
            }else if(inRange(bx,xmin,xmax)&&inRange(by,ymin,ymax)){
                return true
            }else if(this.lineSegmentIntersects(ax,ay,bx,by,xmin,ymin,xmax,ymax)){
                return true
            }else if(this.lineSegmentIntersects(ax,ay,bx,by,xmin,ymax,xmax,ymin)){
                return true
            }
            return false
        }
        //YoZ Projection
        let yozCond = ()=>{
            let az = segment[0][2]
            let ay = segment[0][1]
            let bz = segment[1][2]
            let by = segment[1][1]
            if(inRange(az,zmin,zmax)&&inRange(ay,ymin,ymax)){
                return true
            }else if(inRange(bz,zmin,zmax)&&inRange(by,ymin,ymax)){
                return true
            }else if(this.lineSegmentIntersects(az,ay,bz,by,zmin,ymin,zmax,ymax)){
                return true
            }else if(this.lineSegmentIntersects(az,ay,bz,by,zmin,ymax,zmax,ymin)){
                return true
            }
            return false
        }
        //XoZ Projection
        let xozCond = ()=>{
            let az = segment[0][2]
            let ax = segment[0][0]
            let bz = segment[1][2]
            let bx = segment[1][0]
            if(inRange(az,zmin,zmax)&&inRange(ax,xmin,xmax)){
                return true
            }else if(inRange(bz,zmin,zmax)&&inRange(bx,xmin,xmax)){
                return true
            }else if(this.lineSegmentIntersects(az,ax,bz,bx,zmin,xmin,zmax,xmax)){
                return true
            }else if(this.lineSegmentIntersects(az,ax,bz,bx,zmin,xmax,zmax,xmin)){
                return true
            }
            return false
        }
        return xozCond()&&yozCond()&&yoxCond()
    }
    static triangleIntersectsCube(triangles:number[][],xmin:number,xmax:number,ymin:number,ymax:number,zmin:number,zmax:number){
        return this.lineSegmentIntersectsCube([triangles[0],triangles[1]],xmin,xmax,ymin,ymax,zmin,zmax)||
        this.lineSegmentIntersectsCube([triangles[1],triangles[2]],xmin,xmax,ymin,ymax,zmin,zmax) || 
        this.lineSegmentIntersectsCube([triangles[2],triangles[0]],xmin,xmax,ymin,ymax,zmin,zmax)
    }
}