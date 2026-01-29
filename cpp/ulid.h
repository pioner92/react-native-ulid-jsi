#if defined(__APPLE__)
  #include <Security/SecRandom.h>
 // Android/Linux
#elif defined(__linux__) || defined(__ANDROID__)
  #include <sys/random.h> 
#endif


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
#elif defined(__linux__) || defined(__ANDROID__)
  size_t off = 0;
  while (off < len) {
    ssize_t r = getrandom(out + off, len - off, 0);
    if (r <= 0) throw std::runtime_error("getrandom failed");
    off += (size_t)r;
  }
#else
  std::random_device rd;
  for (size_t i = 0; i < len; i++) out[i] = (uint8_t)rd();
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
    incrementBase32Vals(st.lastRandVals);
  }

  char time10[10];
  encodeTime48To10(t, time10);

  char rand16[16];
  valsToChars(st.lastRandVals, rand16);

  std::memcpy(out26,  time10, 10);
  std::memcpy(out26 + 10, rand16, 16);
}
