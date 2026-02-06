#import "RNUlid.h"
#import <React/RCTBridge+Private.h>
#import <ReactCommon/RCTTurboModule.h>
#import <React/RCTBridge.h>
#import <React/RCTUtils.h>
#import <jsi/jsi.h>

using namespace facebook;

@implementation RNUlid
RCT_EXPORT_MODULE(RNUlid)

@synthesize bridge = _bridge;
@synthesize methodQueue = _methodQueue;

+ (BOOL)requiresMainQueueSetup {
  return NO;
}

RCT_EXPORT_BLOCKING_SYNCHRONOUS_METHOD(install){
	NSLog(@"Installing JSI bindings for react-native-ulid-jsi ...");
	RCTBridge* bridge = [RCTBridge currentBridge];
	RCTCxxBridge* cxxBridge = (RCTCxxBridge*)bridge;

	if (cxxBridge == nil) {
		return @false;
	}

	auto jsiRuntime = (jsi::Runtime*) cxxBridge.runtime;
	if (jsiRuntime == nil) {
		return @false;
	}

	rn_ulid::install(jsiRuntime);

	return @true;
}


@end
