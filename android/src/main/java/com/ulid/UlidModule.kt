package com.ulid

import com.facebook.react.bridge.ReactApplicationContext

class UlidModule(reactContext: ReactApplicationContext) :
  NativeUlidSpec(reactContext) {

  override fun multiply(a: Double, b: Double): Double {
    return a * b
  }

  companion object {
    const val NAME = NativeUlidSpec.NAME
  }
}
