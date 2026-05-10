/**
 * @file displayDriver.h
 * @brief Polymorphic display abstraction for every supported board.
 *
 * Each board ships a single @ref DisplayDriver instance and the global
 * @ref currentDisplayDriver pointer is set at boot based on build-time
 * defines. The mining and monitor layers stay display-agnostic — they
 * call into the driver via function pointers.
 *
 * To add a new board, see docs/boards.md. To add a new screen or
 * animation on an existing board, see docs/screens.md.
 */
#ifndef DISPLAYDRIVER_H_
#define DISPLAYDRIVER_H_

#include "../devices/device.h"

/// Toggles backlight / display power. Called when the user presses the
/// display-toggle button on boards that have one.
typedef void (*AlternateFunction)(void);
/// One-shot driver initialization. Sets up the panel, sprite buffer, and fonts.
typedef void (*DriverInitFunction)(void);
/// Single-shot screen render (loading splash, captive-portal hint, etc.).
typedef void (*ScreenFunction)(void);
/// Cyclic screen render. @p mElapsed is milliseconds since the previous
/// invocation of *this* screen — useful for time-derivative metrics like
/// instantaneous hashrate.
typedef void (*CyclicScreenFunction)(unsigned long mElapsed);
/// Per-frame animation hook. @p frame is a monotonic counter; modulo it
/// for cycle length. Must be cheap — runs on the display SPI bus.
typedef void (*AnimateCurrentScreenFunction)(unsigned long frame);
/// Optional per-frame LED-strip / RGB-LED update.
typedef void (*DoLedStuff)(unsigned long frame);

/**
 * @brief A complete description of how a board renders the UI.
 *
 * Function pointers may be left null on boards that don't support that
 * capability (e.g. headless devices nullify @c loadingScreen and the
 * cyclic-screen array). The dispatch in display.cpp checks before calling.
 */
typedef struct
{
  DriverInitFunction initDisplay;                    ///< Initialize the display.
  AlternateFunction alternateScreenState;            ///< Toggle backlight / power.
  AlternateFunction alternateScreenRotation;         ///< Flip rotation 180°.
  ScreenFunction loadingScreen;                      ///< Boot splash.
  ScreenFunction setupScreen;                        ///< Captive-portal hint.
  CyclicScreenFunction *cyclic_screens;              ///< Array of cyclic screens, length @ref num_cyclic_screens.
  AnimateCurrentScreenFunction animateCurrentScreen; ///< Per-frame animator for the current screen.
  DoLedStuff doLedStuff;                             ///< Per-frame LED update.
  int num_cyclic_screens;                            ///< Number of entries in @ref cyclic_screens.
  int current_cyclic_screen;                         ///< Index of the screen currently shown.
  int screenWidth;                                   ///< Native panel width in pixels.
  int screenHeight;                                  ///< Native panel height in pixels.
} DisplayDriver;

/**
 * @brief Active display driver, chosen at boot in display.cpp.
 *
 * Pointer (not embedded struct) so that headless boards can leave it null.
 * Code that draws to the screen must null-check before dereferencing.
 */
extern DisplayDriver *currentDisplayDriver;

extern DisplayDriver m5stackDisplayDriver;
extern DisplayDriver wt32DisplayDriver;
extern DisplayDriver noDisplayDriver;
extern DisplayDriver ledDisplayDriver;
extern DisplayDriver oled042DisplayDriver;
extern DisplayDriver tDisplayDriver;
extern DisplayDriver amoledDisplayDriver;
extern DisplayDriver dongleDisplayDriver;
extern DisplayDriver esp32_2432S028RDriver;
extern DisplayDriver t_qtDisplayDriver;
extern DisplayDriver tDisplayV1Driver;
extern DisplayDriver m5stickCDriver;
extern DisplayDriver m5stickCPlusDriver;
extern DisplayDriver t_hmiDisplayDriver;
extern DisplayDriver sp_kcDisplayDriver;

/// Number of elements in a fixed-size array. Used to populate
/// @ref DisplayDriver::num_cyclic_screens without hand-counting.
#define SCREENS_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#endif // DISPLAYDRIVER_H_
