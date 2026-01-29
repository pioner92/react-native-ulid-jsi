#pragma once
#ifndef RNUlid_H
#define RNUlid_H
#include <jsi/jsi.h>
#include <iomanip>
#include <sstream>
#include "ulid.h"

#include <cstdint>

using namespace facebook;

namespace rn_ulid {
 void install(jsi::Runtime* rt);
}

#endif /* RNUlid_H */
