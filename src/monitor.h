/**
 * @file monitor.h
 * @brief Telemetry aggregator that feeds every screen.
 *
 * The display drivers call the @c getXxxData() functions below to pull
 * a snapshot of what they need to render. This header is the only thing
 * a driver needs to include from the mining side — keep the screen
 * implementations decoupled from `mining.cpp` internals.
 */
#ifndef MONITOR_API_H
#define MONITOR_API_H

#include <Arduino.h>

/// Index into `cyclic_screens[]` — the live mining stats screen.
#define SCREEN_MINING   0
/// Index into `cyclic_screens[]` — the clock-with-stats screen.
#define SCREEN_CLOCK    1
/// Index into `cyclic_screens[]` — the global / network stats screen.
#define SCREEN_GLOBAL   2
/// Sentinel for headless boards that have no panel at all.
#define NO_SCREEN       3

//Time update period
#define UPDATE_PERIOD_h   5

//API BTC price (Update to USDT cus it's more liquidity and flow price updade)   

//#define getBTCAPI "https://api.coindesk.com/v1/bpi/currentprice.json" -- doesn't work anymore
//#define getBTCAPI "https://api.blockchain.com/v3/exchange/tickers/BTC-USDT" -- updates infrequently
#define getBTCAPI "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd"

#define UPDATE_BTC_min   1

//API Block height
#define getHeightAPI "https://mempool.space/api/blocks/tip/height"
#define UPDATE_Height_min 2

//APIs Global Stats
#define getGlobalHash "https://mempool.space/api/v1/mining/hashrate/3d"
#define getDifficulty "https://mempool.space/api/v1/difficulty-adjustment"
#define getFees "https://mempool.space/api/v1/fees/recommended"
#define UPDATE_Global_min 2

//API public-pool.io
// https://public-pool.io:40557/api/client/btcString
#define getPublicPool "https://public-pool.io:40557/api/client/" // +btcString
#define UPDATE_POOL_min   1

#define NEXT_HALVING_EVENT 1050000 //840000
#define HALVING_BLOCKS 210000

/**
 * @brief Top-level state of the miner — used by the loading / setup
 *        screens to choose what to render.
 */
enum NMState {
  NM_waitingConfig, ///< Captive portal is up, no Wi-Fi credentials yet.
  NM_Connecting,    ///< Joining Wi-Fi or connecting to the Stratum pool.
  NM_hashing        ///< Steady-state mining.
};

typedef struct{
  uint8_t screen;
  bool rotation;
  NMState NerdStatus;
}monitor_data;

typedef struct{
  String globalHash; //hexahashes
  String currentBlock;
  String difficulty;
  String blocksHalving;
  float progressPercent;
  int remainingBlocks;
  int halfHourFee;
#ifdef NERDMINER_T_HMI
  int fastestFee;
  int hourFee;
  int economyFee;
  int minimumFee;
#endif
}global_data;

typedef struct {
  String completedShares;
  String totalMHashes;
  String totalKHashes;
  String currentHashRate;
  String templates;
  String bestDiff;
  String timeMining;
  String valids;
  String temp;
  String currentTime;
}mining_data;

typedef struct {
  String completedShares;
  String totalKHashes;
  String currentHashRate;
  String btcPrice;
  String blockHeight;
  String currentTime;  
  String currentDate;
}clock_data;

typedef struct {
  String currentHashRate;
  String valids;
  unsigned long currentHours;
  unsigned long currentMinutes;
  unsigned long currentSeconds;
}clock_data_t;

typedef struct {
  String completedShares;
  String totalKHashes;
  String currentHashRate;
  String btcPrice;
  String currentTime;
  String halfHourFee;
#ifdef NERDMINER_T_HMI
  String hourFee;
  String fastestFee;
  String economyFee;
  String minimumFee;
#endif
  String netwrokDifficulty;
  String globalHashRate;
  String blockHeight;
  float progressPercent;
  String remainingBlocks;
}coin_data;

typedef struct{
  int workersCount;       // Workers count, how many nerdminers using your address
  String workersHash;     // Workers Total Hash Rate
  String bestDifficulty;  // Your miners best difficulty
}pool_data;

/// One-shot monitor initialization. Spawns the FreeRTOS task that
/// polls external APIs (BTC price, block height, fees, pool stats).
void setup_monitor(void);

/**
 * @brief Snapshot for the mining-stats screen.
 * @param mElapsed Milliseconds since the previous call from this screen.
 *                 Used to compute instantaneous hashrate.
 */
mining_data getMiningData(unsigned long mElapsed);

/// Snapshot for the clock screen (BTC price + light mining stats).
clock_data getClockData(unsigned long mElapsed);

/// Snapshot for the global-stats screen (network hashrate, halving, fees).
coin_data getCoinData(unsigned long mElapsed);

/// Snapshot of public-pool worker stats for *this* BTC address.
pool_data getPoolData(void);

/// Compact clock-only data used by displays that show a digital clock.
clock_data_t getClockData_t(unsigned long mElapsed);

/// Returns the configured public-pool API URL, suffixed with the
/// user's BTC address.
String getPoolAPIUrl(void);

#endif //MONITOR_API_H
