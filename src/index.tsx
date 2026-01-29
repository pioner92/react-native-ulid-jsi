import {NativeModules} from 'react-native'
declare global {
  var __getUlid: (seedTime?:number) => string;
}


let RNUlidInitialized = globalThis.__getUlid;

if(!RNUlidInitialized){
  if(NativeModules.RNUlid){
    NativeModules.RNUlid.install();
    RNUlidInitialized = globalThis.__getUlid;
    console.log('✅ react-native-ulid initialized successfully')
  }
}


export const ulid = (seedTime?: number): string => {
  return globalThis.__getUlid(seedTime);
}

export const isValid = (id: string): boolean => {
  // TODO: implement validation
  return true;
}

export const decodeTime = (id: string): number => {
  //TODO: implement decodeTime
  return 0;
}

