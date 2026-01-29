import "react-native-get-random-values"
import {  useState } from 'react';
import { Text, View, StyleSheet, Pressable } from 'react-native';
import { ulid as jsiUlid, isValid as isValidNative, decodeTime as decodeTimeNative } from 'react-native-ulid-jsi';
import { ulid, isValid, decodeTime  } from "ulid";



function delay(ms: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, ms))
}

async function waitForGc(): Promise<void> {
  //@ts-ignore
  global.gc()
  await delay(500)
}

interface BenchmarksResult {
  numberOfIterations: number
  jsTimeMs: number
  nativeTimeMs: number
}

const ITERATIONS = 1000


function benchmarkJS(): number {
  const start = performance.now()

  for (let i = 0; i < ITERATIONS; i++) {
    const id1 = ulid();
  }
  const end = performance.now()

  return end - start
}

function benchmarkNative(): number {

  // warm up
  jsiUlid();
  jsiUlid();

  const start = performance.now()

  for (let i = 0; i < ITERATIONS; i++) {
    const id1 = jsiUlid();
  }
  const end = performance.now()

  return end - start
}

const testCorrectness = () => {
  const nativeUlid = jsiUlid();
  const jsUlid = ulid();

  console.log('IS VALID', {
    nativeCheckNative: isValidNative(nativeUlid),
    nativeCheckJS: isValidNative(jsUlid),
    jsCheckNative: isValid(nativeUlid),
    jsCheckJS: isValid(jsUlid),
  });

  console.log('DECODE TIME', {
    nativeDecodeNative: decodeTimeNative(nativeUlid),
    nativeDecodeJS: decodeTimeNative(jsUlid),
    jsDecodeNative: decodeTime(nativeUlid),
    jsDecodeJS: decodeTime(jsUlid),
  });
}

async function runBenchmarks(): Promise<BenchmarksResult> {
  console.log(`Running benchmarks ${ITERATIONS}x...`)
  await waitForGc()

  const jsTime = benchmarkJS()
  const nativeTime = benchmarkNative()

  console.log(
    `Benchmarks finished! js: ${jsTime.toFixed(2)}ms | native JSI: ${nativeTime.toFixed(2)}ms`
  )

  testCorrectness()
  return {
    jsTimeMs: jsTime,
    nativeTimeMs: nativeTime,
    numberOfIterations: ITERATIONS,
  }
}

export default function App() {


  const [result, setResult] = useState<BenchmarksResult | null>(null)


  const runTest = async () => {
    const res = await runBenchmarks()
    setResult(res)
  }

  return (
    <View style={styles.container}>
      <View style={{ alignItems: 'center' }}>
        {result ? (
          <>
            <Text style={{ fontSize: 20 }}>ULID generations: {result?.numberOfIterations}x</Text>
            <View style={{ marginTop: 20 }}>
              <Text style={{ fontWeight: 'bold', fontSize:18 }}>JS: {result?.jsTimeMs.toFixed(2)}ms</Text>
              <Text style={{ fontWeight: 'bold', fontSize:18 }}>Native JSI: {result?.nativeTimeMs.toFixed(2)}ms</Text>
              <Text style={{ fontWeight: 'bold', fontSize:18 }}>
                Native is {(result!.jsTimeMs / result!.nativeTimeMs).toFixed(2)}× faster
              </Text>
            </View>
          </>
        ) : (
          null
        )}

      </View>
      <Pressable style={{ marginTop: 20 }} onPress={runTest}><Text style={{ fontSize: 18 }}>Run Benchmarks</Text></Pressable>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    height:'100%',
    width:'100%',
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: 'white'
  },
});
