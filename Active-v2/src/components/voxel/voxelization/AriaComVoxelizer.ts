import { floor } from "gl-matrix-ts/dist/vec3";
import { AriaComponent } from "../../../core/AriaComponent";
import { AriaVoxel } from "../AriaVoxel";
import { AriaCompGeo } from "../../../core/arithmetic/AriaCompGeo";
import { scale } from "gl-matrix-ts/dist/mat3";

export class AriaComVoxelizer extends AriaVoxel{
    constructor(triangles:number[][],scaler:number = 1,mag:number=1){
        super(mag)
        this._rename("AriaCom/Voxelizer")
        this.buildVoxels(triangles,scaler)
    }

    private buildVoxels(triangles:number[][],scaler:number = 1){
        this._logInfo("Total triangles:"+triangles.length)
        for(let i=0;i<triangles.length;i+=3){

            let xmin = triangles[i][0]
            let xmax = triangles[i][0]
            let ymin = triangles[i][1]
            let ymax = triangles[i][1]
            let zmin = triangles[i][2]
            let zmax = triangles[i][2]
            
            for(let j=0;j<3;j++){
                xmin = Math.min(triangles[i+j][0],xmin)
                xmax = Math.max(triangles[i+j][0],xmax)
                ymin = Math.min(triangles[i+j][1],ymin)
                ymax = Math.max(triangles[i+j][1],ymax)
                zmin = Math.min(triangles[i+j][2],zmin)
                zmax = Math.max(triangles[i+j][2],zmax)
            }
            xmin = Math.floor(xmin/scaler)
            xmax = Math.ceil(xmax/scaler)
            ymin = Math.floor(ymin/scaler)
            ymax = Math.ceil(ymax/scaler)
            zmin = Math.floor(zmin/scaler)
            zmax = Math.ceil(zmax/scaler)
            
            for(let x=xmin;x<=xmax;x++){
                for(let y=ymin;y<=ymax;y++){
                    for(let z=zmin;z<=zmax;z++){
                        if(AriaCompGeo.triangleIntersectsCube([triangles[i],triangles[i+1],triangles[i+2]],x*scaler,(x+1)*scaler,y*scaler,(y+1)*scaler,z*scaler,(z+1)*scaler)){
                            this.setVoxel(x*scaler,y*scaler,z*scaler,{tsdf:1,valid:true})
                        }
                    }
                }
            }
        }

    }
}