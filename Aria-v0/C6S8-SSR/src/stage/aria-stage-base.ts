import { AriaPageIndicator } from "../core/aria-page-indicator";

export class AriaStage{
    public async prepare(gl:WebGL2RenderingContext):Promise<any>{
        AriaPageIndicator.getInstance().updateLoadingTip("ERROR: The stage given is invalid")

    }
    public async render(gl:WebGL2RenderingContext):Promise<any>{}
}