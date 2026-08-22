#include "variant.h"

#ifdef USE_XL9555
#include "ExtensionIOXL9555.hpp"
extern ExtensionIOXL9555 io;
#endif

#if defined(USE_XL9555) && defined(HAS_TOUCHSCREEN)
#include "TouchDrvCSTXXX.hpp"
static TouchDrvCSTXXX ultraTouch;
static bool ultraTouchOk = false;
#endif

// Runs right after power->setup() (PMU rails on), before the I2C scan, radio
// and display init - see main.cpp.
void tWatchUltraExpanderInit()
{
    // Keep unselected SPI devices off the bus: the NFC reader and SD card share
    // the radio's SPI lines, and a floating CS lets them drive MISO and corrupt
    // radio transactions (RadioLib then reports chip-not-found)
    pinMode(NFC_CS, OUTPUT);
    digitalWrite(NFC_CS, HIGH);
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);

#ifdef USE_XL9555
    if (!io.begin(Wire, XL9555_SLAVE_ADDR, I2C_SDA, I2C_SCL)) {
        return;
    }

    // The AMOLED latches if a previous boot leaves it mid-command, and the
    // battery-backed PMU means USB replugs never drop its rail. Recover with
    // LilyGoLib's power-gate + long reset pulse before the display driver runs.
    io.pinMode(EXPANDS_DISP_EN, OUTPUT);
    io.digitalWrite(EXPANDS_DISP_EN, LOW);
    pinMode(CO5300_RST, OUTPUT);
    digitalWrite(CO5300_RST, LOW);
    delay(50);
    io.digitalWrite(EXPANDS_DISP_EN, HIGH);
    delay(20);
    digitalWrite(CO5300_RST, HIGH);
    delay(200);
    digitalWrite(CO5300_RST, LOW);
    delay(300);
    digitalWrite(CO5300_RST, HIGH);
    delay(200);

    // Haptic driver enable
    io.pinMode(EXPANDS_DRV_EN, OUTPUT);
    io.digitalWrite(EXPANDS_DRV_EN, HIGH);

#ifdef HAS_TOUCHSCREEN
    // Reset CST9217 (reset line lives on the expander), then bring it up ahead
    // of the input thread init that runs during setupModules
    io.pinMode(EXPANDS_TOUCH_RST, OUTPUT);
    io.digitalWrite(EXPANDS_TOUCH_RST, LOW);
    delay(20);
    io.digitalWrite(EXPANDS_TOUCH_RST, HIGH);
    delay(60);

    ultraTouch.setPins(-1, SCREEN_TOUCH_INT);
    ultraTouch.setTouchDrvModel(TouchDrv_CST92XX);
    ultraTouchOk = ultraTouch.begin(Wire, TOUCH_SLAVE_ADDRESS, I2C_SDA, I2C_SCL) ||
                   ultraTouch.begin(Wire, 0x5A, I2C_SDA, I2C_SCL);
#endif

    // Route the radio to the built-in antenna (LOW = external USB LoRa port)
    io.pinMode(EXPANDS_LORA_RF_SW, OUTPUT);
    io.digitalWrite(EXPANDS_LORA_RF_SW, HIGH);
#endif
}

#if defined(USE_XL9555) && defined(HAS_TOUCHSCREEN)
bool tWatchUltraHasTouch()
{
    return ultraTouchOk;
}

bool tWatchUltraGetTouch(int16_t *x, int16_t *y)
{
    int16_t touchX, touchY;
    if (ultraTouch.getPoint(&touchX, &touchY, 1) == 0)
        return false;
    touchX -= TFT_BEVEL_INSET_X;
    touchY -= TFT_BEVEL_INSET_Y;
    if (touchX < 0 || touchX >= TFT_WIDTH - 2 * TFT_BEVEL_INSET_X || touchY < 0 ||
        touchY >= TFT_HEIGHT - 2 * TFT_BEVEL_INSET_Y)
        return false;
    *x = touchX;
    *y = touchY;
    return true;
}
#else
bool tWatchUltraHasTouch()
{
    return false;
}

bool tWatchUltraGetTouch(int16_t *x, int16_t *y)
{
    (void)x;
    (void)y;
    return false;
}
#endif
