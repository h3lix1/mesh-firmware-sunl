#pragma once

#include "Arduino.h"
#include "Observer.h"
#include "configuration.h"

/**
 * Low Battery Recovery Mode
 *
 * Purpose: Prevents solar-powered nodes from completely draining their batteries.
 * When battery drops critically low, the device enters deep sleep and periodically
 * wakes to check if solar charging has restored sufficient power.
 *
 * To enable: #define LOW_BATTERY_RECOVERY_ENABLED in variant.h
 * Also requires: BATTERY_PIN and ADC_MULTIPLIER defined for early-boot battery sensing.
 * Devices with PMU (like T-Beam S3) will do a brief normal boot to read battery.
 *
 * Behavior:
 *   - No USB: Sleep at <=10% battery, wake when >=15% (hysteresis prevents oscillation)
 *   - With USB: Sleep only at <=5%, wake when >5% (USB should prevent further drain)
 *   - Button press during sleep will force immediate wake (manual override)
 *
 * To customize thresholds, define these in variant.h before including sleep.h:
 *   LOW_BATT_SLEEP_INTERVAL_MS - wake check interval (default: 5 minutes)
 *   LOW_BATT_ENTER_THRESHOLD   - enter sleep at this % without USB (default: 10%)
 *   LOW_BATT_EXIT_THRESHOLD    - exit sleep at this % without USB (default: 15%)
 *   LOW_BATT_USB_THRESHOLD     - threshold when USB connected (default: 5%)
 */

#ifndef LOW_BATT_SLEEP_INTERVAL_MS
#define LOW_BATT_SLEEP_INTERVAL_MS (5 * 60 * 1000) // 5 minutes wake interval
#endif

#ifndef LOW_BATT_ENTER_THRESHOLD
#define LOW_BATT_ENTER_THRESHOLD 10 // Enter deep sleep at 10% battery (no USB)
#endif

#ifndef LOW_BATT_EXIT_THRESHOLD
#define LOW_BATT_EXIT_THRESHOLD 15 // Exit deep sleep at 15% battery (no USB)
#endif

#ifndef LOW_BATT_USB_THRESHOLD
#define LOW_BATT_USB_THRESHOLD 5 // With USB: sleep if ≤5%, wake if >5%
#endif

void doDeepSleep(uint32_t msecToWake, bool skipPreflight, bool skipSaveNodeDb), cpuDeepSleep(uint32_t msecToWake);

#ifdef ARCH_ESP32
#include "esp_sleep.h"
esp_sleep_wakeup_cause_t doLightSleep(uint64_t msecToWake);

extern esp_sleep_source_t wakeCause;
#endif

#ifdef HAS_PMU
#include "XPowersLibInterface.hpp"
extern XPowersLibInterface *PMU;
#endif

// Perform power on init that we do on each wake from deep sleep
void initDeepSleep();

void setCPUFast(bool on);

/** return true if sleep is allowed right now */
bool doPreflightSleep();

extern int bootCount;

#if defined(ARCH_ESP32) && defined(LOW_BATTERY_RECOVERY_ENABLED)
// Tracks if we're in low battery recovery mode (persists across deep sleep)
extern bool inLowBatteryRecoveryMode;
#endif

/// Called to ask any observers if they want to veto sleep. Return 1 to veto or 0 to allow sleep to happen
extern Observable<void *> preflightSleep;

/// Called to tell observers we are now entering (deep) sleep and you should prepare.  Must return 0
extern Observable<void *> notifyDeepSleep;

/// Called to tell observers we are rebooting ASAP.  Must return 0
extern Observable<void *> notifyReboot;

#ifdef ARCH_ESP32
/// Called to tell observers that light sleep is about to begin
extern Observable<void *> notifyLightSleep;

/// Called to tell observers that light sleep has just ended, and why it ended
extern Observable<esp_sleep_wakeup_cause_t> notifyLightSleepEnd;
#endif

void enableModemSleep();

#ifdef ARCH_ESP32
void enableLoraInterrupt();
bool shouldLoraWake(uint32_t msecToWake);

#ifdef HAS_LIGHT_SLEEP
// DFS (Dynamic Frequency Scaling) diagnostic functions
// Call dfsLogStatus() periodically to log CPU frequency changes
void dfsLogStatus();
// Call dfsLogConstraints() to get detailed diagnostics about what might prevent scaling
void dfsLogConstraints();
#endif
#endif