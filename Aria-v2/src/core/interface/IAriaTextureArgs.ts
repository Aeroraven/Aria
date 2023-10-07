export enum AriaTextureFormat{
    ATF_MONOCHANNEL = 1
}
export enum AriaTextureDataType{
    ATDT_UNSIGNED_BYTE = 1
}
export interface IAriaTextureArgs{
    w?:number,
    h?:number,
    format:AriaTextureFormat,
    datatype:AriaTextureDataType
}