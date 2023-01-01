import { AriaObject } from "../base/AriaObject";

export class AriaTime extends AriaObject{
    constructor(){
        super("AriaTime")
    }
    public static getTimeStamp(){
        return Date.now()
    }
}