#include "react-native-ulid.h"

void rn_ulid::install(jsi::Runtime* runtime) {
  jsi::Runtime& rt = *runtime;

  auto getUlid = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "__getUlid"), 1,
      [](jsi::Runtime& runtime, const jsi::Value& thisValue,
         const jsi::Value* arguments, size_t count) -> jsi::Value {
        uint64_t seedTime;

        if (count >= 1 && arguments[0].isNumber()) {
          double d = arguments[0].getNumber();
          if (d >= 0) {
            seedTime = (uint64_t)d;
          } else {
            seedTime = nowMs();
          }
        } else {
          seedTime = nowMs();
        }

        char out[26];
        generateUlidMonotonic(seedTime, out);

        return jsi::String::createFromUtf8(runtime, (const uint8_t*)out, 26);
      });

  auto checkIsValid = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "__checkIsValid"), 1,
      [](jsi::Runtime& runtime, const jsi::Value& thisValue,
        const jsi::Value* arguments, size_t count) -> jsi::Value {
        std::string s = arguments[0].getString(runtime).utf8(runtime);
        return checkULID(s.data(), s.length());
      });

  auto decodeTime = jsi::Function::createFromHostFunction(
      rt, jsi::PropNameID::forAscii(rt, "__decodeTime"), 1,
      [](jsi::Runtime& runtime, const jsi::Value& thisValue,
         const jsi::Value* arguments, size_t count) -> jsi::Value {
        if (count >= 1 && arguments[0].isString()) {
          std::string s = arguments[0].getString(runtime).utf8(runtime);

          uint64_t ts = 0;

          if (!decodeTimeFromUlid(s.data(), s.size(), ts)) {
            return jsi::Value::undefined();
          }

          return jsi::Value((double)ts);
        }

        return jsi::Value::undefined();
      });

  rt.global().setProperty(rt, jsi::PropNameID::forAscii(rt, "__getUlid"),
                          std::move(getUlid));

  rt.global().setProperty(rt, jsi::PropNameID::forAscii(rt, "__checkIsValid"),
                          std::move(checkIsValid));

  rt.global().setProperty(rt, jsi::PropNameID::forAscii(rt, "__decodeTime"),
                          std::move(decodeTime));
}
