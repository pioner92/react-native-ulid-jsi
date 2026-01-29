# react-native-ulid-jsi

[![npm version](https://img.shields.io/npm/v/react-native-ulid-jsi.svg)](https://www.npmjs.com/package/react-native-ulid-jsi)
[![npm downloads](https://img.shields.io/npm/dm/react-native-ulid-jsi.svg)](https://www.npmjs.com/package/react-native-ulid-jsi)
[![license](https://img.shields.io/npm/l/react-native-ulid-jsi.svg)](https://github.com/pioner92/react-native-ulid-jsi/blob/main/LICENSE)

⚡️ **Ultra-fast ULID (Universally Unique Lexicographically Sortable Identifier) generator for React Native**

Built with JSI (JavaScript Interface) and C++ for maximum performance. Features **monotonic generation** with thread-local state for guaranteed ID ordering. Supports both **New Architecture (Fabric + TurboModules)** and **Old Architecture** on iOS and Android.

🚀 **500x faster** than JavaScript | 🔄 **Monotonic** | 📦 **Zero dependencies**

## 🚀 Performance

**500x faster** than pure JavaScript implementations!

| Implementation | Time (1000 iterations) | Performance |
|---------------|----------------------|-------------|
| **react-native-ulid-jsi (JSI/C++)** | **0.17ms** | ⚡️ **500x faster** |
| Pure JavaScript (ulid package) | 83.62ms | 🐌 Baseline |

```
react-native-ulid-jsi  ▓ 0.17ms
JavaScript ULID        ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ 83.62ms
                       └────────────────────────────────────────────────┘
                                    500x performance gain
```

*Benchmark performed on iPhone 16 Pro with production build*

### Why so fast?

1. **Zero Bridge Overhead**: Direct JSI binding to C++ (no serialization)
2. **Native Random Generation**: Platform-specific secure random APIs
3. **Optimized Encoding**: Hand-tuned Base32 encoding in C++
4. **Thread-local State**: No synchronization overhead for monotonic generation
5. **Minimal Allocations**: Stack-allocated buffers, no heap fragmentation

### Run your own benchmark

```typescript
import { ulid } from 'react-native-ulid-jsi';

const iterations = 1000;
const start = performance.now();

for (let i = 0; i < iterations; i++) {
  ulid();
}

const end = performance.now();
console.log(`Generated ${iterations} ULIDs in ${(end - start).toFixed(2)}ms`);
// Output: Generated 1000 ULIDs in 0.17ms
```

## ✨ Features

- ⚡️ **Lightning Fast** - JSI + C++ implementation for native performance
- 🏗️ **New Architecture Ready** - Full support for React Native's new architecture (Fabric + TurboModules)
- 🔄 **Backward Compatible** - Works seamlessly with old architecture
- 📱 **Cross Platform** - iOS and Android support
- 🎯 **Type Safe** - Full TypeScript support
- 🪶 **Lightweight** - Zero dependencies, pure C++ implementation
- 📈 **Monotonic Generation** - Guarantees increasing IDs even within the same millisecond
- ⏱️ **Timestamp Encoded** - Contains creation timestamp (first 48 bits)
- 🎲 **Cryptographically Secure** - Platform-native secure random generation
  - iOS: `SecRandomCopyBytes` (Security Framework)
  - Android: getrandom when available, falls back to /dev/urandom
- 🔤 **Crockford's Base32** - Excludes ambiguous characters (I, L, O, U)

## 📦 Installation

```sh
npm install react-native-ulid-jsi
```

or with yarn:

```sh
yarn add react-native-ulid-jsi
```

### iOS

```sh
cd ios && pod install
```

### Android

No additional steps required. Gradle will handle everything automatically.

## 📖 Usage

### Basic Usage

```typescript
import { ulid } from 'react-native-ulid-jsi';

// Generate a new ULID
const id = ulid();
console.log(id); // 01ARZ3NDEKTSV4RRFFQ69G5FAV

// Generate multiple ULIDs - they will be monotonically increasing
const id1 = ulid(); // 01HGW4Z6C8ABCDEFGHIJKLMNOP
const id2 = ulid(); // 01HGW4Z6C8ABCDEFGHIJKLMNPQ (guaranteed > id1)
const id3 = ulid(); // 01HGW4Z6C8ABCDEFGHIJKLMNPR (guaranteed > id2)
```

### With Seed Time

```typescript
import { ulid } from 'react-native-ulid-jsi';

// Generate ULID with custom timestamp (milliseconds since epoch)
const timestamp = Date.now();
const id = ulid(timestamp);
console.log(id); // 01ARZ3NDEKTSV4RRFFQ69G5FAV
```

### Validate ULID

```typescript
import { isValid } from 'react-native-ulid-jsi';

const id = '01ARZ3NDEKTSV4RRFFQ69G5FAV';
const valid = isValid(id);
console.log(valid); // true
```

### Decode Timestamp

```typescript
import { decodeTime } from 'react-native-ulid-jsi';

const id = '01ARZ3NDEKTSV4RRFFQ69G5FAV';
const timestamp = decodeTime(id);
console.log(timestamp); // 1469918176385
console.log(new Date(timestamp)); // 2016-07-31T04:42:56.385Z
```

## 🎯 API Reference

### `ulid(seedTime?: number): string`

Generates a new ULID.

**Parameters:**
- `seedTime` (optional): Custom timestamp in milliseconds since Unix epoch. If not provided, uses current time.

**Returns:** A 26-character ULID string.

**Example:**
```typescript
const id1 = ulid(); // Uses current time
const id2 = ulid(1469918176385); // Uses custom time
```

### `isValid(id: string): boolean`

Validates whether a string is a valid ULID.

**Parameters:**
- `id`: String to validate

**Returns:** `true` if valid ULID, `false` otherwise.

**Example:**
```typescript
isValid('01ARZ3NDEKTSV4RRFFQ69G5FAV'); // true
isValid('invalid-id'); // false
```

### `decodeTime(id: string): number`

Extracts the timestamp from a ULID.

**Parameters:**
- `id`: Valid ULID string

**Returns:** Timestamp in milliseconds since Unix epoch.

**Example:**
```typescript
const timestamp = decodeTime('01ARZ3NDEKTSV4RRFFQ69G5FAV');
console.log(new Date(timestamp)); // Original creation time
```

## 🏗️ Architecture Support

This library is built to support both React Native architectures:

### New Architecture (Fabric + TurboModules)
✅ Fully supported with JSI direct bindings
✅ Zero overhead JavaScript ↔️ Native communication
✅ Synchronous API access

### Old Architecture (Bridge)
✅ Fully supported with JSI module installation
✅ Same performance benefits
✅ No breaking changes required

The library automatically detects and works with the architecture your app is using.

## 🔍 What is ULID?

ULID (Universally Unique Lexicographically Sortable Identifier) is a better alternative to UUID for many use cases:

- **Sortable**: Unlike UUIDs, ULIDs are lexicographically sortable
- **Timestamp**: Contains a timestamp component (first 48 bits)
- **Compact**: 26 characters vs UUID's 36 characters
- **URL-safe**: Uses Crockford's base32 encoding
- **Case-insensitive**: Easy to use in various contexts
- **No special characters**: Just alphanumeric characters

### ULID Format

```
 01AN4Z07BY      79KA1307SR9X4MV3
|----------|    |----------------|
 Timestamp       Randomness
  (48 bits)       (80 bits)
```

## 🔄 Monotonic Generation

This library implements **monotonic ULID generation**, ensuring that IDs are always increasing even when generated within the same millisecond.

### How it works:

1. **Same timestamp**: If multiple ULIDs are generated in the same millisecond, the random component is incremented instead of generating new random bytes
2. **Overflow protection**: If the random component overflows, a new random value is generated
3. **Time progression**: When time advances, a fresh random value is used

### Why monotonic?

```typescript
// Without monotonic (pure random):
const id1 = ulid(); // 01HGW4Z6C8ABCDEFGHIJKLMNOP
const id2 = ulid(); // 01HGW4Z6C8ZYXWVUTSRQPONMLK  ❌ Not guaranteed to be > id1

// With monotonic (this library):
const id1 = ulid(); // 01HGW4Z6C8ABCDEFGHIJKLMNOP
const id2 = ulid(); // 01HGW4Z6C8ABCDEFGHIJKLMNPQ  ✅ Always > id1
```

**Benefits:**
- ✅ Database indexes work optimally (B-tree friendly)
- ✅ No collisions within same millisecond
- ✅ Guaranteed sortability
- ✅ Better for distributed systems

## 💡 Use Cases

- **Database IDs**: Sortable by creation time without separate timestamp field
- **Distributed Systems**: Generate unique IDs without coordination
- **API Resources**: URL-safe identifiers for REST APIs
- **Event Sourcing**: Time-ordered events with unique identifiers
- **Logging**: Sortable log entry identifiers
- **File Names**: Unique, sortable file names

## 🔧 Technical Details

- **Implementation**: Pure C++ with JSI (JavaScript Interface) bindings
- **Monotonic Generation**: Thread-local state ensures IDs increment even within same millisecond
- **Thread Safety**: Thread-local storage per thread, no locks or mutexes needed
- **Random Generation** (with automatic fallback):
  - iOS: `SecRandomCopyBytes` (Security Framework)
  - Android: getrandom when available, falls back to /dev/urandom
  - Last resort: `std::random_device` (other platforms)
- **Encoding**: Crockford's Base32 (0-9, A-Z excluding I, L, O, U)
- **Memory**: Minimal allocation, optimized for mobile devices
- **Dependencies**: Zero - pure C++ standard library
- **Bundle Size**: Native module only, zero JavaScript bundle impact
- **Platforms**: iOS 12+, Android API 21+

## 📊 Comparison with JavaScript ULID

| Feature | react-native-ulid-jsi | JavaScript ULID |
|---------|------------------|-----------------|
| Performance | ⚡️ 0.17ms (1000 ops) | 🐌 83.62ms (1000 ops) |
| Speed Improvement | **500x faster** | Baseline |
| Implementation | C++ / JSI | JavaScript |
| Monotonic | ✅ Thread-local | ⚠️ Varies by package |
| Architecture Support | New + Old | N/A |
| Secure Random | ✅ Platform native | ⚠️ JS Math.random or crypto |
| Bundle Impact | Native only (0 KB JS) | +5-10 KB bundle |
| Dependencies | Zero | Varies |

## 🤝 Contributing

Contributions are welcome! Please read our [contributing guidelines](CONTRIBUTING.md) and [code of conduct](CODE_OF_CONDUCT.md).

### Development Workflow

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'feat: add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed development setup.

## 📄 License

MIT © [Alex Shumihin](https://github.com/pioner92)

## 🔗 Links

- [GitHub Repository](https://github.com/pioner92/react-native-ulid-jsi)
- [npm Package](https://www.npmjs.com/package/react-native-ulid-jsi)
- [Issues](https://github.com/pioner92/react-native-ulid-jsi/issues)
- [ULID Specification](https://github.com/ulid/spec)

## ⭐️ Show Your Support

If this project helped you, please give it a ⭐️!

---

Made with ❤️ using [create-react-native-library](https://github.com/callstack/react-native-builder-bob)
