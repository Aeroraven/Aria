export class AriaAsyncURLOp{
    public static async fetchText(url:string){
        let response = await window.fetch(url);
        return await response.text();
    };

    public static async fetchBlob(url:string){
        let response = await window.fetch(url);
        return await response.blob();
    };
}