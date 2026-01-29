import {NativeModules} from 'react-native'
declare global {
  var __getUlid: (seedTime?:number) => string;
  var __checkIsValid: (ulid: string) => boolean;
  var __decodeTime: (ulid: string) => number;
}


let RNUlidInitialized = globalThis.__getUlid;

if(!RNUlidInitialized){
  if(NativeModules.RNUlid){
    NativeModules.RNUlid.install();
    RNUlidInitialized = globalThis.__getUlid;
    console.log('✅ react-native-ulid initialized successfully')
  }
}

/** 
 * Generate ULID
 * @param seedTime number (optional)
 * @returns string
 */
export const ulid = (seedTime?: number): string => {
  return globalThis.__getUlid(seedTime);
}

/** 
  * Validate ULID
 * @param ulid string
 * @returns boolean
 */
export const isValid = (ulid: string): boolean => {
  if(typeof ulid !== 'string' || ulid.length !== 26){
    return false;
  }

  return  globalThis.__checkIsValid(ulid);;
}

/**
 * Decode time from ULID
 * @param ulid string
 * @returns number | undefined
 */
export const decodeTime = (ulid: string): number | undefined => {
  if(typeof ulid !== 'string' || ulid.length !== 26){
    return undefined;
  }
  return globalThis.__decodeTime(ulid);
}

