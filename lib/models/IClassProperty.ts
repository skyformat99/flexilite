/**
 * Created by slanska on 04.10.2015.
 */


/*

 */
declare  interface  IClassProperty
{
    ClassID?:number;
    PropertyID?:number;
    PropertyName?:string;
    TrackChanges?:boolean;
    DefaultValue?:any;
    ctlo?:number;
    ctloMask?:number;
    DefaultDataType?:number;
    MinOccurences?:number;
    MaxOccurences?:number;
    Unique?:boolean;
    ColumnAssigned?:string;
    AutoValue?:string;
    MaxLength:number;
    TempColumnAssigned?:number;
    ReferencedClassID?:number;
    ReversePropertyID?:number;
    ctlv:number;
    Indexed?: boolean;
    ExtData?:any;
    ValidationRegex?:string;
}
