// LilyGo T-Watch Ultra
// ESP32-S3, 16MB flash, 8MB PSRAM, 2.06" 410x502 AMOLED (CO5300, QSPI),
// CST9217 touch, AXP2101 PMU, XL9555 IO expander, PCF85063 RTC, BHI260AP IMU,
// DRV2605 haptic, SX1262 LoRa, u-blox MIA-M10Q GPS, MAX98357A speaker.

// CO5300 QSPI AMOLED, driven through LovyanGFX (Panel_CO5300 + Bus_SPI quad mode).
// The panel is portrait-native: 410 visible columns (offset 22 in controller
// memory, the gap for the curved-bezel glass) x 502 rows.
#define HAS_SPI_TFT 1
#define USE_TFTDISPLAY 1
#define CO5300_QSPI_AMOLED 1

#define CO5300_CS 41
#define CO5300_RST 37
#define CO5300_SCK 40
#define CO5300_D0 38
#define CO5300_D1 39
#define CO5300_D2 42
#define CO5300_D3 45
#define CO5300_SPI_HOST SPI3_HOST
#define CO5300_SPI_FREQUENCY 80000000

#define TFT_WIDTH 410
#define TFT_HEIGHT 502
#define TFT_OFFSET_X 22
#define TFT_OFFSET_Y 0
// The 2.5D glass overhangs the whole panel perimeter (~40 px on the straight
// edges, ruler-calibrated on hardware; corner curves cut ~58 px). The UI canvas
// is shrunk to the visible rectangle and centered - the occluded border is
// unusable, and black theme backgrounds make the frame invisible.
#define TFT_BEVEL_INSET_X 40
#define TFT_BEVEL_INSET_Y 32
#define SCREEN_TRANSITION_FRAMERATE 5 // fps

#define HAS_TOUCHSCREEN 1
#define SCREEN_TOUCH_INT 12  // TP_INT, CST9217 interrupt (active low)
#define TOUCH_SLAVE_ADDRESS 0x1A // CST9217; falls back to 0x5A
#define ENABLE_TOUCH_INT
#define WAKE_ON_TOUCH

#define USE_POWERSAVE
#define SLEEP_TIME 180

// Single shared I2C bus: PMU, RTC, touch, IMU, haptic, IO expander
#define I2C_SDA 3
#define I2C_SCL 2

#define HAS_AXP2101
#define PMU_IRQ 7 // Interrupt pin for the PMU
#define PMU_POWER_BUTTON_IS_CANCEL // maps a short click of the power button to a cancel action (turning off the screen)

// XL9555 GPIO expander gates display power (IO7), touch reset (IO8),
// haptic enable (IO6) and the LoRa RF switch (IO11)
#define USE_XL9555
#define XL9555_SLAVE_ADDR 0x20
#define EXPANDS_DRV_EN 6
#define EXPANDS_DISP_EN 7
#define EXPANDS_TOUCH_RST 8
#define EXPANDS_LORA_RF_SW 11

// PCF85063A RTC (button-cell backed)
#define PCF85063_RTC 0x51

#define HAS_DRV2605 1

// MAX98357A I2S speaker
#define HAS_I2S
#define DAC_I2S_BCK 9
#define DAC_I2S_WS 10
#define DAC_I2S_DOUT 11
#define DAC_I2S_MCLK -1
// The amp sits on PMU rail BLDO2; gate it around playback
#define AUDIO_AMP_ENABLE(on)                                                                                              \
    do {                                                                                                                   \
        if (pmu_found)                                                                                                     \
            (on) ? PMU->enablePowerOutput(XPOWERS_BLDO2) : PMU->disablePowerOutput(XPOWERS_BLDO2);                         \
    } while (0)

#define HAS_GPS 1
#define GPS_BAUDRATE 38400
#define GPS_RX_PIN 44 // MCU RX <- GPS TX
#define GPS_TX_PIN 43 // MCU TX -> GPS RX
#define PIN_GPS_PPS 13

#define BUTTON_PIN 0 // Boot button

// Shared-SPI bus hygiene: the ST25R3916 NFC reader and SD slot hang off the
// radio's bus; their CS lines must not float or they corrupt MISO
#define NFC_CS 4
#define SD_CS 21

#define USE_SX1262
#define USE_SX1268
#define USE_SX1280 // the watch ships in SX1262 (sub-GHz) and SX1280 (2.4 GHz) SKUs; detect at runtime

// LoRa shares SPI2 with the SD card / NFC; the display owns SPI3 (QSPI)
#define LORA_SCK 35
#define LORA_MISO 33
#define LORA_MOSI 34
#define LORA_CS 36

#define LORA_RESET 47
#define LORA_DIO1 14 // SX1262 IRQ
#define LORA_BUSY 48

#define SX126X_CS LORA_CS
#define SX126X_DIO1 LORA_DIO1
#define SX126X_BUSY LORA_BUSY
#define SX126X_RESET LORA_RESET
#define SX126X_DIO2_AS_RF_SWITCH
// The radio module has a TCXO powered from DIO3; LilyGoLib runs it at RadioLib's
// default 1.6 V. Without this the oscillator never starts (RadioLib -707)
#define SX126X_DIO3_TCXO_VOLTAGE 1.6

#define SX128X_CS LORA_CS
#define SX128X_DIO1 LORA_DIO1
#define SX128X_BUSY LORA_BUSY
#define SX128X_RESET LORA_RESET

#define USE_VIRTUAL_KEYBOARD 1
#define DISPLAY_CLOCK_FRAME 1
