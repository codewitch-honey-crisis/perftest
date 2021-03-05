#ifndef HTCW_ESP32_SD_READER_HPP
#define HTCW_ESP32_SD_READER_HPP
extern "C"
{
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"

#ifdef CONFIG_IDF_TARGET_ESP32
#include "driver/sdmmc_host.h"
#endif
}
#include "spi_host.hpp"
namespace esp32
{
    class sd_reader
    {
        esp_err_t m_last_error;
        char m_mount_point[64];
        sdmmc_card_t *m_card;
        inline bool check_mount_result(esp_err_t ret) {
            if (ret != ESP_OK)
            {
                m_last_error=ret;
                if (ret == ESP_FAIL)
                {
                    ESP_LOGE("sd_reader", "Failed to mount filesystem. ");
                }
                else
                {
                    ESP_LOGE("sd_reader", "Failed to initialize the card (%s). "
                                          "Make sure SD card lines have pull-up resistors in place.",
                             esp_err_to_name(ret));
                }
                return false;
            }
            return true;
        }
        bool init(
            const char *mount_point,
            const esp_vfs_fat_sdmmc_mount_config_t &mount_config,
            const sdmmc_host_t &host,
            const sdspi_device_config_t& slot_config)
        {
            m_card = nullptr;
            const char* sz = nullptr!=mount_point?mount_point:default_mount_point;
            if(63<strlen(sz))
                return false;
            strcpy(m_mount_point,sz);
            return check_mount_result(esp_vfs_fat_sdspi_mount(m_mount_point, &host, &slot_config, &mount_config, &m_card));
        }

        bool init(
            const char *mount_point,
            const esp_vfs_fat_sdmmc_mount_config_t &mount_config,
            const sdmmc_host_t &host,
            const sdmmc_slot_config_t& slot_config)
        {
            m_card = nullptr;
            const char* sz = nullptr!=mount_point?mount_point:default_mount_point;
            if(63<strlen(sz))
                return false;
            strcpy(m_mount_point,sz);
            return check_mount_result(esp_vfs_fat_sdmmc_mount(m_mount_point, &host, &slot_config, &mount_config, &m_card));
        }
        bool deinit() {
            esp_err_t ret = esp_vfs_fat_sdcard_unmount(m_mount_point, m_card);
            if(ESP_OK!=ret) {
                m_last_error = ret;
                ESP_LOGE("sd_reader","Failed to unmount the card");
                return false;
            }
            return true;
        }
    public:
        static const char *default_mount_point;
        sd_reader(
            const char *mount_point, 
            const esp_vfs_fat_sdmmc_mount_config_t &mount_config,
            const sdmmc_host_t &host,
            const sdspi_device_config_t &slot_config) {
            init(mount_point,mount_config,host,slot_config);
        }
        sd_reader(
            const char *mount_point, 
            const esp_vfs_fat_sdmmc_mount_config_t &mount_config,
            const sdmmc_host_t &host,
            const sdmmc_slot_config_t &slot_config) {
            init(mount_point,mount_config,host,slot_config);
        }
        sd_reader()=delete;
        sd_reader(const sd_reader& rhs)=delete;
        sd_reader& operator=(const sd_reader& rhs)=delete;
        sd_reader(sd_reader&& rhs) : m_last_error(rhs.m_last_error),m_card(rhs.m_card) {
            rhs.m_card = nullptr;
            strcpy(m_mount_point,rhs.m_mount_point);
        }
        sd_reader& operator=(sd_reader&& rhs) {
            m_last_error=rhs.m_last_error;
            m_card=rhs.m_card;
            rhs.m_card = nullptr;
            strcpy(m_mount_point,rhs.m_mount_point);
            return *this;
        }
        virtual ~sd_reader() {
            if(initialized())
                deinit();
        }
        unsigned long long free() const {
            if(!initialized())
                return 0;
            FATFS* fs;
            DWORD dwClus;
            FRESULT f = f_getfree(m_mount_point,&dwClus,&fs);
            if(FR_OK!=f)
                return 0;
            return ((unsigned long long )fs->csize)*((unsigned long long )fs->ssize)*((unsigned long long )dwClus);
        }
        unsigned long long size() const {
            if(!initialized())
                return 0;
            FATFS* fs;
            DWORD dwClus;
            FRESULT f = f_getfree(m_mount_point,&dwClus,&fs);
            if(FR_OK!=f)
                return 0;
            return ((unsigned long long )fs->csize)*((unsigned long long )fs->ssize)*((fs->n_fatent - 2) );
            
        }
        
        esp_err_t last_error() const {
            return m_last_error;
        }
        bool initialized() const {
            return nullptr!=m_card;
        }
        const char* mount_point() const {
            return m_mount_point;
        }
        const sdmmc_card_t* card() const {
            return m_card;
        }
        
    };
    const char* sd_reader::default_mount_point= "/sdcard";
}
#endif