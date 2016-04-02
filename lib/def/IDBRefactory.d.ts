/**
 * Created by slanska on 2016-04-01.
 */

///<reference path="../../typings/lib.d.ts"/>

interface IDBRefactory
{
    /*
     Returns report on results of last refactoring action
     */
    getLastActionReport():string;

    /*
     Alters existing class. New properties can be added, deleted or modified in any way. Properties can be also
     renamed if property's def has $renameTo value. If newName is set, class will be renamed. Class name should
     be unique. Optional new schema mapping can be passed. If not passed, new default schema will be generated, based on
     the current basic schema
     */
    alterClass(classID:number, newClassDef?:IClassDefinition, newSchemaDef?:ISchemaDefinition, newName?:string);

    /*
     Creates new class with the given name (should be unique). Optional new schema mapping can be passed.
     If not passed, new default schema will be generated
     */
    createClass(name:string, classDef:IClassDefinition, schemaDef?:ISchemaDefinition);

    /*
     Drops class, all related schemas, objects and references from database. Operation can be undone.
     */
    dropClass(classID:number);

    /*
     Extracts existing properties from class definition and creates a new property of NESTED_OBJECT type.
     New class will be created/or existing one will be updated. Object data will not be affected
     */
    plainPropertiesToNestedObject(classID:number, propIDs:PropertyIDs, newRefProp:IClassProperty, filter:IObjectFilter, targetClassID:number);

    /*
     Extracts existing properties to external linked object. Existing object data might be updated or stay untouched.
     Key properties can be optionally passed to check if identical object of the target class already exists. In this case,
     new linked object will not be created, but reference will be set to existing one.
     Example: Country column as string. Then class 'Country' was created. Property 'Country' was extracted to the new class
     and replaced with link
     */
    plainPropertiesToLinkedObject(classID:number, propIDs:PropertyIDs, newRefProp:IClassProperty, filter:IObjectFilter, targetClassID:number,
                                  updateData:boolean, sourceKeyPropID:PropertyIDs, targetKeyPropID:PropertyIDs);

    /*
     Action opposite to extracting nested object: existing nested object will be disassembled into individual properties
     and these properties will be added to the master class
     */
    nestedObjectToPlainProperties(classID:number, refPropID:number, filter:IObjectFilter, propMap:IPropertyMap);

    /*
     Action opposite to extracting linked object: existing linked object will be disassembled into individual properties
     and these properties will be added to the master class. This action will involve modifying object data
     */
    linkedObjectToPlainProps(classID:number, refPropID:number, filter:IObjectFilter, propMap:IPropertyMap);

    /*
     Joins 2 non related objects into single object, using provided property map. Corresponding objects will be found using sourceKeyPropIDs
     and targetKeyPropIDs
     */
    structuralMerge(sourceClassID:number, sourceFilter:IObjectFilter, sourceKeyPropID:PropertyIDs, targetClassID:number,
                    targetKeyPropID:PropertyIDs, propMap:IPropertyMap);

    /*
     Splits objects vertically, i.e. one set of properties goes to class A, another - to class B. Resulting objects do not have any
     relation to each other
     */
    structuralSplit(sourceClassID:number, filter:IObjectFilter, targetClassID:number,
                    propMap:IPropertyMap, targetClassDef?:IClassDefinition);

    /*
     Change class ID of given objects. Updates schemas and possibly columns A..J to match new class schema
     */
    moveToAnotherClass(sourceClassID:number, filter:IObjectFilter, targetClassID:number, propMap:IPropertyMap);

    /*
     Removes duplicated objects. Updates references to point to a new object. When resolving conflict, selects object
     with larger number of references to it, or object that was updated more recently.
     */
    removeDuplicatedObjects(classID:number, filter:IObjectFilter, compareFunction:string, keyProps:PropertyIDs, replaceTargetNulls:boolean);

    /*
     Split property into multiple: use SQL expressions
     */
    splitProperty(classID:number, sourcePropID:number, propRules:ISplitPropertyRules);

    /*
     Merge many properties into one: use SQL expressions
     */
    mergeProperties(classID:number, sourcePropIDs:number[], targetProp:IClassProperty, expression:string);
}

/*
 Declares contract for defining list of objects either by objectID(s), filter and/or ClassID
 */
interface IObjectFilter
{
    objectId?:number | [number];
    filter?:any; // TODO orm filter
}

declare type ISplitPropertyRules = [{regex?:string, newPropDef:IClassProperty}];

declare type IPropertyMap = [{sourcePropID:number, targetPropID:number}];

declare type PropertyIDs = number | number[];

