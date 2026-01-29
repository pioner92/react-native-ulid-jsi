import "react-native-get-random-values"
import { useState } from 'react';
import { Text, View, StyleSheet, Pressable } from 'react-native';
import { getUlid } from 'react-native-ulid';
import { ulid } from "ulid";



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
  getUlid();
  getUlid();

  const start = performance.now()

  for (let i = 0; i < ITERATIONS; i++) {
    const id1 = getUlid();
  }
  const end = performance.now()

  return end - start
}

async function runBenchmarks(): Promise<BenchmarksResult> {
  console.log(`Running benchmarks ${ITERATIONS}x...`)
  await waitForGc()

  const jsTime = benchmarkJS()
  const nativeTime = benchmarkNative()

  console.log(
    `Benchmarks finished! js: ${jsTime.toFixed(2)}ms | native JSI: ${nativeTime.toFixed(2)}ms`
  )
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
              <Text style={{ fontWeight: 'bold' }}>JS: {result?.jsTimeMs.toFixed(2)}ms</Text>
              <Text style={{ fontWeight: 'bold' }}>Native JSI: {result?.nativeTimeMs.toFixed(2)}ms</Text>
              <Text style={{ fontWeight: 'bold' }}>
                Native is {(result!.jsTimeMs / result!.nativeTimeMs).toFixed(2)}× faster
              </Text>
            </View>
          </>
        ) : (
          null
        )}

      </View>
      <Pressable style={{ marginTop: 20 }} onPress={runTest}><Text>Run Benchmarks</Text></Pressable>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
    backgroundColor: 'white'
  },
});
