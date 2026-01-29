#include <jni.h>
#include "react-native-ulid.h"
#include "jsi/jsi.h"
#include <android/log.h>
#include <fbjni/fbjni.h>
#include <fbjni/detail/Registration.h>
#include <typeinfo>

//XXHashBridge


using namespace facebook;

struct RNUlidBridge : jni::JavaClass<RNUlidBridge> {
public:
    static constexpr auto kJavaDescriptor = "Lcom/ulid/UlidModule;";

    static void registerNatives() {
        javaClassStatic()->registerNatives({
                                                   makeNativeMethod("nativeInstall", RNUlidBridge::nativeInstall)
                                           });
    }
private:
    static void nativeInstall(
            jni::alias_ref<jni::JObject> thiz,
            jlong jsiRuntimePointer
    ) {

        auto jsiRuntime = reinterpret_cast<jsi::Runtime*>(jsiRuntimePointer);
        rn_ulid::install(jsiRuntime);
    }
};

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *) {
    return jni::initialize(vm, [] { RNUlidBridge::registerNatives(); });
}
