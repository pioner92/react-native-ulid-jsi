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


export const getUlid = (seedTime?: number): string => {
  return globalThis.__getUlid(seedTime);
}
