#ifndef HTCW_SD_CONFIGURATION_H
#define HTCW_SD_CONFIGURATION_H
#include "spi_host.hpp"
#include "sd_reader.hpp"
using namespace esp32;

// when i try using this i it drops from .75MB/s to .5MB/s
#define SDMMC_HOST_HIGHSPEED() {\
    .flags = SDMMC_HOST_FLAG_8BIT | \
             SDMMC_HOST_FLAG_4BIT | \
             SDMMC_HOST_FLAG_1BIT | \
             SDMMC_HOST_FLAG_DDR, \
    .slot = SDMMC_HOST_SLOT_1, \
    .max_freq_khz = SDMMC_FREQ_HIGHSPEED, \
    .io_voltage = 3.3f, \
    .init = &sdmmc_host_init, \
    .set_bus_width = &sdmmc_host_set_bus_width, \
    .get_bus_width = &sdmmc_host_get_slot_width, \
    .set_bus_ddr_mode = &sdmmc_host_set_bus_ddr_mode, \
    .set_card_clk = &sdmmc_host_set_card_clk, \
    .do_transaction = &sdmmc_host_do_transaction, \
    .deinit = &sdmmc_host_deinit, \
    .io_int_enable = sdmmc_host_io_int_enable, \
    .io_int_wait = sdmmc_host_io_int_wait, \
    .command_timeout_ms = 0, \
}
// this is used by the integrated flash memory controller. use it at your peril:
#define SDMMC_HOST_0() {\
    .flags = SDMMC_HOST_FLAG_8BIT | \
             SDMMC_HOST_FLAG_4BIT | \
             SDMMC_HOST_FLAG_1BIT | \
             SDMMC_HOST_FLAG_DDR, \
    .slot = SDMMC_HOST_SLOT_0, \
    .max_freq_khz = SDMMC_FREQ_DEFAULT, \
    .io_voltage = 3.3f, \
    .init = &sdmmc_host_init, \
    .set_bus_width = &sdmmc_host_set_bus_width, \
    .get_bus_width = &sdmmc_host_get_slot_width, \
    .set_bus_ddr_mode = &sdmmc_host_set_bus_ddr_mode, \
    .set_card_clk = &sdmmc_host_set_card_clk, \
    .do_transaction = &sdmmc_host_do_transaction, \
    .deinit = &sdmmc_host_deinit, \
    .io_int_enable = sdmmc_host_io_int_enable, \
    .io_int_wait = sdmmc_host_io_int_wait, \
    .command_timeout_ms = 0, \
}
// ESP32-S2 and ESP32-C3 doesn't have an SD Host peripheral, always use SPI:
#if CONFIG_IDF_TARGET_ESP32S2 ||CONFIG_IDF_TARGET_ESP32C3
#ifndef USE_SPI_MODE
#define USE_SPI_MODE
#endif // USE_SPI_MODE
// on ESP32-S2, DMA channel must be the same as host id
#define SPI_DMA_CHAN    host.slot
#endif //CONFIG_IDF_TARGET_ESP32S2

// DMA channel to be used by the SPI peripheral
#ifndef SPI_DMA_CHAN
#define SPI_DMA_CHAN    1
#endif //SPI_DMA_CHAN

// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.

#ifdef USE_SPI_MODE
// Pin mapping when using SPI mode.
// With this mapping, SD card can be used both in SPI and 1-line SD mode.
// Note that a pull-up on CS line is required in SD mode.
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define PIN_NUM_MISO GPIO_NUM_2
#define PIN_NUM_MOSI GPIO_NUM_15
#define PIN_NUM_CLK  GPIO_NUM_14
#define PIN_NUM_CS   GPIO_NUM_13

#elif CONFIG_IDF_TARGET_ESP32C3
#define PIN_NUM_MISO GPIO_NUM_18
#define PIN_NUM_MOSI GPIO_NUM_9
#define PIN_NUM_CLK  GPIO_NUM_8
#define PIN_NUM_CS   GPIO_NUM_19
#endif //CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#endif //USE_SPI_MODE

static inline sd_reader sd_configure() {
      // Configure the SD reader. There are quite a few parameters and configurations

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed=false,
        .max_files=5,
        .allocation_unit_size=16*1024
    };
#ifndef USE_SPI_MODE
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config=SDMMC_SLOT_CONFIG_DEFAULT();
    gpio_set_pull_mode(GPIO_NUM_15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(GPIO_NUM_2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(GPIO_NUM_4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_mode(GPIO_NUM_12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    gpio_set_pull_mode(GPIO_NUM_13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes
#else
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_config = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
        .flags=0,
        .intr_flags=0
    };
    spi_host spi((spi_host_device_t)host.slot,bus_config,SPI_DMA_CHAN);
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = (spi_host_device_t)host.slot;
#endif
    return sd_reader(
        sd_reader::default_mount_point,
        mount_config,
        host,
        slot_config
    );
}
#endif