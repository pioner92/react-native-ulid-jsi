#if defined(__APPLE__)
  #include <Security/SecRandom.h>
#elif defined(__ANDROID__) || defined(__linux__)
  #include <fcntl.h>        // open
  #include <unistd.h>       // read, close
  #include <errno.h>        // errno
  // getrandom available on Android API 28+ and Linux with glibc 2.25+
  #if defined(__ANDROID__)
    #include <sys/syscall.h>  // syscall
    #include <linux/random.h> // GRND_NONBLOCK
  #else
    #include <sys/random.h>   // getrandom
  #endif
#endif

// --------- GENERATE ULID --------//

static constexpr char kEncoding[32] = {
  '0','1','2','3','4','5','6','7','8','9',
  'A','B','C','D','E','F','G','H',
  'J','K','M','N','P','Q','R','S','T','V','W','X','Y','Z'
};

static inline uint64_t nowMs() noexcept {
  using namespace std::chrono;
  return (uint64_t)duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

static inline void secureRandomBytes(uint8_t* out, size_t len) {
#if defined(__APPLE__)
  if (SecRandomCopyBytes(kSecRandomDefault, len, out) != errSecSuccess) {
    throw std::runtime_error("SecRandomCopyBytes failed");
  }

#elif defined(__ANDROID__) || defined(__linux__)
  // 1) Try getrandom() first (Android API 28+ / Linux kernel 3.17+)
  size_t off = 0;
  bool getrandom_ok = true;

  while (off < len) {
#if defined(__ANDROID__)
    // Use syscall directly for Android compatibility with older NDK
    ssize_t r = syscall(__NR_getrandom, out + off, len - off, 0);
#else
    // Use getrandom() directly on Linux
    ssize_t r = getrandom(out + off, len - off, 0);
#endif
    if (r > 0) {
      off += (size_t)r;
    } else if (errno == ENOSYS) {
      // getrandom not available, use fallback
      getrandom_ok = false;
      break;
    } else if (errno == EINTR) {
      // Interrupted, retry
      continue;
    } else {
      // Other error, use fallback
      getrandom_ok = false;
      break;
    }
  }

  if (getrandom_ok) {
    return;
  }

  // 2) Fallback: /dev/urandom (works on all Android/Linux versions)
  int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
  if (fd < 0) {
    throw std::runtime_error("open(/dev/urandom) failed");
  }

  off = 0;
  while (off < len) {
    ssize_t r = read(fd, out + off, len - off);
    if (r > 0) {
      off += (size_t)r;
    } else if (r == 0) {
      close(fd);
      throw std::runtime_error("unexpected EOF from /dev/urandom");
    } else if (errno == EINTR) {
      // Interrupted, retry
      continue;
    } else {
      close(fd);
      throw std::runtime_error("read(/dev/urandom) failed");
    }
  }
  close(fd);

#else
  // Last-resort fallback (should never happen on React Native)
  std::random_device rd;
  for (size_t i = 0; i < len; ++i) {
    out[i] = static_cast<uint8_t>(rd());
  }
#endif
}

static inline void encodeTime48To10(uint64_t ms, char out10[10]) {
    const uint8_t b0 = (uint8_t)((ms >> 40) & 0xFF);
    const uint8_t b1 = (uint8_t)((ms >> 32) & 0xFF);
    const uint8_t b2 = (uint8_t)((ms >> 24) & 0xFF);
    const uint8_t b3 = (uint8_t)((ms >> 16) & 0xFF);
    const uint8_t b4 = (uint8_t)((ms >>  8) & 0xFF);
    const uint8_t b5 = (uint8_t)((ms >>  0) & 0xFF);


    out10[0] = kEncoding[(b0 & 0xE0) >> 5];
    out10[1] = kEncoding[(b0 & 0x1F)];
    out10[2] = kEncoding[(b1 & 0xF8) >> 3];
    out10[3] = kEncoding[((b1 & 0x07) << 2) | ((b2 & 0xC0) >> 6)];
    out10[4] = kEncoding[(b2 & 0x3E) >> 1];
    out10[5] = kEncoding[((b2 & 0x01) << 4) | ((b3 & 0xF0) >> 4)];
    out10[6] = kEncoding[((b3 & 0x0F) << 1) | ((b4 & 0x80) >> 7)];
    out10[7] = kEncoding[(b4 & 0x7C) >> 2];
    out10[8] = kEncoding[((b4 & 0x03) << 3) | ((b5 & 0xE0) >> 5)];
    out10[9] = kEncoding[(b5 & 0x1F)];
}


struct UlidMonotonicState {
  uint64_t lastTime = 0;
  uint8_t lastRandVals[16]{};
  bool hasLast = false;
};

static inline void randomValsFrom80bits(
    const uint8_t rnd[10],
    uint8_t out[16]
) noexcept {
  out[0]  = (rnd[0] >> 3) & 0x1F;
  out[1]  = ((rnd[0] & 0x07) << 2) | (rnd[1] >> 6);
  out[2]  = (rnd[1] >> 1) & 0x1F;
  out[3]  = ((rnd[1] & 0x01) << 4) | (rnd[2] >> 4);
  out[4]  = ((rnd[2] & 0x0F) << 1) | (rnd[3] >> 7);

  out[5]  = (rnd[3] >> 2) & 0x1F;
  out[6]  = ((rnd[3] & 0x03) << 3) | (rnd[4] >> 5);
  out[7]  = rnd[4] & 0x1F;

  out[8]  = (rnd[5] >> 3) & 0x1F;
  out[9]  = ((rnd[5] & 0x07) << 2) | (rnd[6] >> 6);
  out[10] = (rnd[6] >> 1) & 0x1F;
  out[11] = ((rnd[6] & 0x01) << 4) | (rnd[7] >> 4);
  out[12] = ((rnd[7] & 0x0F) << 1) | (rnd[8] >> 7);

  out[13] = (rnd[8] >> 2) & 0x1F;
  out[14] = ((rnd[8] & 0x03) << 3) | (rnd[9] >> 5);
  out[15] = rnd[9] & 0x1F;
}


static inline bool incrementBase32Vals(uint8_t vals[16]) noexcept {
  for (int i = 15; i >= 0; --i) {
    uint8_t x = vals[i];
    if (x != 31u) {
      vals[i] = (uint8_t)(x + 1u);
      return true;
    }
    vals[i] = 0u;
  }
  return false;
}

static inline void valsToChars(const uint8_t v[16],
                               char o[16]) noexcept
{
  o[0] = kEncoding[v[0] & 31u];
  o[1] = kEncoding[v[1] & 31u];
  o[2] = kEncoding[v[2] & 31u];
  o[3] = kEncoding[v[3] & 31u];
  o[4] = kEncoding[v[4] & 31u];
  o[5] = kEncoding[v[5] & 31u];
  o[6] = kEncoding[v[6] & 31u];
  o[7] = kEncoding[v[7] & 31u];
  o[8] = kEncoding[v[8] & 31u];
  o[9] = kEncoding[v[9] & 31u];
  o[10] = kEncoding[v[10] & 31u];
  o[11] = kEncoding[v[11] & 31u];
  o[12] = kEncoding[v[12] & 31u];
  o[13] = kEncoding[v[13] & 31u];
  o[14] = kEncoding[v[14] & 31u];
  o[15] = kEncoding[v[15] & 31u];
}

static inline void generateUlidMonotonic(uint64_t seedTimeMs, char out26[26]) {
  thread_local UlidMonotonicState st;

  uint64_t t = seedTimeMs;

  if (!st.hasLast || t > st.lastTime) {
    st.lastTime = t;
    st.hasLast = true;

    uint8_t rnd[10];
    secureRandomBytes(rnd, sizeof(rnd));
    randomValsFrom80bits(rnd, st.lastRandVals);
  } else {
    t = st.lastTime;

    if (!incrementBase32Vals(st.lastRandVals)) {
      // overflow: advance time by 1ms and reseed randomness
      st.lastTime += 1;
      t = st.lastTime;

      uint8_t rnd[10];
      secureRandomBytes(rnd, sizeof(rnd));
      randomValsFrom80bits(rnd, st.lastRandVals);
    }
  }

  char time10[10];
  encodeTime48To10(t, time10);

  char rand16[16];
  valsToChars(st.lastRandVals, rand16);

  std::memcpy(out26,  time10, 10);
  std::memcpy(out26 + 10, rand16, 16);
}

// --------- CHECK IS VALID ULID --------//

static inline int8_t crockfordBase32Value(uint8_t c) noexcept {
  static const std::array<int8_t, 256> T = [] {
    std::array<int8_t, 256> t{};
    t.fill(-1);

    // 0-9
    for (int i = 0; i <= 9; ++i) {
      t[static_cast<uint8_t>('0' + i)] = static_cast<int8_t>(i);
    }

    // A-Z (Crockford without I, L, O, U)
    t[static_cast<uint8_t>('A')] = 10; t[static_cast<uint8_t>('B')] = 11;
    t[static_cast<uint8_t>('C')] = 12; t[static_cast<uint8_t>('D')] = 13;
    t[static_cast<uint8_t>('E')] = 14; t[static_cast<uint8_t>('F')] = 15;
    t[static_cast<uint8_t>('G')] = 16; t[static_cast<uint8_t>('H')] = 17;
    t[static_cast<uint8_t>('J')] = 18; t[static_cast<uint8_t>('K')] = 19;
    t[static_cast<uint8_t>('M')] = 20; t[static_cast<uint8_t>('N')] = 21;
    t[static_cast<uint8_t>('P')] = 22; t[static_cast<uint8_t>('Q')] = 23;
    t[static_cast<uint8_t>('R')] = 24; t[static_cast<uint8_t>('S')] = 25;
    t[static_cast<uint8_t>('T')] = 26;
    t[static_cast<uint8_t>('V')] = 27; t[static_cast<uint8_t>('W')] = 28;
    t[static_cast<uint8_t>('X')] = 29; t[static_cast<uint8_t>('Y')] = 30;
    t[static_cast<uint8_t>('Z')] = 31;

    // lowercase = same values (case-insensitive)
    for (int ch = 'a'; ch <= 'z'; ++ch) {
      const int upper = ch - 32; // ASCII: 'a'->'A'
      t[static_cast<uint8_t>(ch)] = t[static_cast<uint8_t>(upper)];
    }

    return t;
  }();

  return T[c];
}

static inline bool checkULID(const char* s, size_t len) noexcept {
  if (len != 26) return false;

  const int8_t v0 = crockfordBase32Value((uint8_t)s[0]);
  if (v0 < 0 || v0 > 7) return false;

  for (size_t i = 1; i < 26; ++i) {
    if (crockfordBase32Value((uint8_t)s[i]) < 0) return false;
  }
  return true;
}


// --------- DECODE TIME --------//
static inline bool decodeTime48From10(const char* s10, uint64_t& outMs) noexcept {
  uint64_t x = 0;

  // First char must be 0..7
  const int8_t v0 = crockfordBase32Value((uint8_t)s10[0]);
  if (v0 < 0 || v0 > 7) return false;

  x = (uint64_t)v0;

  // Remaining 9 chars
  for (int i = 1; i < 10; ++i) {
    const int8_t v = crockfordBase32Value((uint8_t)s10[i]);
    if (v < 0) return false;
    x = (x << 5) | (uint64_t)v;
  }

  // x is 50 bits, but top 2 bits are 0 => it equals the 48-bit ms timestamp
  outMs = x;
  return true;
}


static inline bool decodeTimeFromUlid(const char* s, size_t len, uint64_t& outMs) noexcept {
  if (len != 26) return false;

  return decodeTime48From10(s, outMs);
}

