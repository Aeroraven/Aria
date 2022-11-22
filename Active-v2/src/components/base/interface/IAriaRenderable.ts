export interface IAriaRenderable<T=void>{
    render(preTriggers?:(()=>any)[], postTriggers?:(()=>any)[]):T
}