import { AriaComponent } from "../../core/AriaComponent";
import { AriaTime } from "../../core/timing/AriaTime";

export class AriaComStopwatch extends AriaComponent{
    private _last = 0
    private _scaler = 1
    constructor(){
        super("AriaCom/Stopwatch")
        this._last = AriaTime.getTimeStamp()
    }
    public setScaler(x:number){
        this._scaler = x
    }
    public recordAndGetDuration(){
        let cur = AriaTime.getTimeStamp()
        let ret = cur - this._last
        this._last = cur
        return ret * this._scaler
    }
}