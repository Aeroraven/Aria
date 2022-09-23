import { AriaBufferMap } from "../core/aria-buffer-map";

export class AriaComHelloCube{
    static initBuffer(gl:WebGL2RenderingContext):AriaBufferMap{
        const posBuffer = <WebGLBuffer>gl.createBuffer();
        let vertices = [
            // Front face
            -1.0, -1.0,  1.0,
             1.0, -1.0,  1.0,
             1.0,  1.0,  1.0,
            -1.0,  1.0,  1.0,
          
            // Back face
            -1.0, -1.0, -1.0,
            -1.0,  1.0, -1.0,
             1.0,  1.0, -1.0,
             1.0, -1.0, -1.0,
          
            // Top face
            -1.0,  1.0, -1.0,
            -1.0,  1.0,  1.0,
             1.0,  1.0,  1.0,
             1.0,  1.0, -1.0,
          
            // Bottom face
            -1.0, -1.0, -1.0,
             1.0, -1.0, -1.0,
             1.0, -1.0,  1.0,
            -1.0, -1.0,  1.0,
          
            // Right face
             1.0, -1.0, -1.0,
             1.0,  1.0, -1.0,
             1.0,  1.0,  1.0,
             1.0, -1.0,  1.0,
          
            // Left face
            -1.0, -1.0, -1.0,
            -1.0, -1.0,  1.0,
            -1.0,  1.0,  1.0,
            -1.0,  1.0, -1.0
        ];
        gl.bindBuffer(gl.ARRAY_BUFFER,posBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(vertices),gl.STATIC_DRAW)
    
        //Color
        const colBuffer = <WebGLBuffer>gl.createBuffer()
        let preColors = [
            [1.0,  1.0,  1.0,  1.0],    // Front face: white
            [1.0,  0.0,  0.0,  1.0],    // Back face: red
            [0.0,  1.0,  0.0,  1.0],    // Top face: green
            [0.0,  0.0,  1.0,  1.0],    // Bottom face: blue
            [1.0,  1.0,  0.0,  1.0],    // Right face: yellow
            [1.0,  0.0,  1.0,  1.0]     // Left face: purple
        ];
        let colList:number[] = []
        for(let i=0;i<6;i++){
            for(let j=0;j<4;j++){
                colList = colList.concat(preColors[i])
            }
        }
        gl.bindBuffer(gl.ARRAY_BUFFER,colBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(colList),gl.STATIC_DRAW)
    
        //Normal
        const normBuffer = <WebGLBuffer>gl.createBuffer()
        let preNorms = [
            [0,0,1],
            [0,0,-1],
            [0,1,0],
            [0,-1,0],
            [1,0,0],
            [-1,0,0]
        ]
        let normList:number[] = []
        for(let i=0;i<6;i++){
            for(let j=0;j<4;j++){
                normList = normList.concat(preNorms[i])
            }
        }
        gl.bindBuffer(gl.ARRAY_BUFFER,normBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(normList),gl.STATIC_DRAW)
    
        //Texture
        const texBuffer = <WebGLBuffer>gl.createBuffer()
        let preTex = [0.0,0.0,1.0,0.0,1.0,1.0,0.0,1.0]
        let texList:number[] = []
        for(let i=0;i<6;i++){
            texList = texList.concat(preTex)
        }
        gl.bindBuffer(gl.ARRAY_BUFFER,texBuffer)
        gl.bufferData(gl.ARRAY_BUFFER,new Float32Array(texList),gl.STATIC_DRAW)
    
    
        const eleBuffer = <WebGLBuffer>gl.createBuffer()
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER,eleBuffer)
        const eleIndices = [
            0,  1,  2,      0,  2,  3,    // front
            4,  5,  6,      4,  6,  7,    // back
            8,  9,  10,     8,  10, 11,   // top
            12, 13, 14,     12, 14, 15,   // bottom
            16, 17, 18,     16, 18, 19,   // right
            20, 21, 22,     20, 22, 23,   // left
        ];
        
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,new Uint16Array(eleIndices),gl.STATIC_DRAW)
    
        //Return
        const r = new AriaBufferMap()
        r.set("pos",posBuffer)
        r.set("col",colBuffer)
        r.set("tex",texBuffer)
        r.set("norm",normBuffer)
        r.set("ele",eleBuffer)
        return r
    }
}