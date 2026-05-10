/**
 * @file mining.h
 * @brief Public API of the mining loop and Stratum worker.
 *
 * The actual SHA‑256 inner loop runs on a FreeRTOS task pinned to a CPU
 * core (`runMiner` / `minerWorkerHw|Sw`) so that display SPI traffic on
 * the other core never starves hashing. Stratum job notifications arrive
 * on a separate task (`runStratumWorker`) and invalidate in-flight work
 * to avoid stale shares.
 *
 * The display layer never calls into here directly — it consumes
 * aggregated telemetry through `getMiningData()` in monitor.h.
 */

#ifndef MINING_API_H
#define MINING_API_H

/// Maximum nonce delta searched per inner-loop iteration before the
/// outer loop checks for a new Stratum job. Smaller = lower stale rate
/// at the cost of more polling overhead.
#define MAX_NONCE_STEP  5000000U
/// Hard cap on nonces tried per job before reporting back, regardless
/// of difficulty. Prevents getting stuck on a job if the pool is silent.
#define MAX_NONCE       25000000U
/// Reference target nonce used by the test harness in ShaTests.
#define TARGET_NONCE    471136297U
/// Difficulty floor used until the pool sends one. Public-pool typically
/// overrides this on the first `mining.set_difficulty`.
#define DEFAULT_DIFFICULTY  0.00015
/// How long to wait between Stratum keepalive packets (ms).
#define KEEPALIVE_TIME_ms       30000
/// If no pool message arrives within this window, the worker reconnects.
#define POOLINACTIVITY_TIME_ms  60000

//#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(CONFIG_IDF_TARGET_ESP32S3) || defined(CONFIG_IDF_TARGET_ESP32C3)
/// Use the hardware SHA-256 peripheral when present (ESP32-S2/S3/C3).
/// On classic ESP32 the software fallback is used.
#define HARDWARE_SHA265
//#endif

/// Size in bytes of the Stratum target buffer (256-bit hex string).
#define TARGET_BUFFER_SIZE 64

/// Monitor task entry point. @p name is the FreeRTOS task name.
void runMonitor(void *name);

/// Stratum worker task entry point — owns the TCP connection.
void runStratumWorker(void *name);
/// High-level miner task entry point — pulls jobs and dispatches workers.
void runMiner(void *name);

/// Software SHA-256 worker. Used as a fallback or when HARDWARE_SHA265 is undefined.
void minerWorkerSw(void * task_id);
/// Hardware-accelerated SHA-256 worker. Pinned to a core; do not call directly.
void minerWorkerHw(void * task_id);

/// Format the current NTP-synced time as a human-readable string.
String printLocalTime(void);

/// Reset the rolling hashrate / shares-found counters. Called when a
/// new pool is configured or after a manual reset from the UI.
void resetStat();

/**
 * @brief Per-worker mining state passed into the inner loop.
 *
 * Lives on the worker task's stack. The blockheader buffer is sized for
 * the largest possible header (128B) — most chains use 80B; the extra
 * gives room for the merkle padding without reallocation.
 */
typedef struct{
  uint8_t bytearray_target[32];      ///< Network target (big-endian).
  uint8_t bytearray_pooltarget[32];  ///< Per-worker pool target.
  uint8_t merkle_result[32];         ///< Final merkle root for this job.
  uint8_t bytearray_blockheader[128];///< Working block header buffer.
} miner_data;


#endif // UTILS_API_H