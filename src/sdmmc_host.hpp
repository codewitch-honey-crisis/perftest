#ifndef HTCW_ESP32_SDMMC_HOST_HPP
#define HTCW_ESP32_SDMMC_HOST_HPP
extern "C"
{
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"
#include "driver/sdmmc_host.h"
}
#include <atomic>
namespace esp32 {
    class sdmmc_host_slot;
    class sdmmc_card;
    class sdmmc_host final {
        friend class sdmmc_host_slot;
        friend class sdmmc_card;
        static std::atomic_int m_count;
        static std::atomic<esp_err_t> m_last_error;
        sdmmc_host()=delete;
        sdmmc_host(const sdmmc_host& rhs)=delete;
        sdmmc_host& operator=(sdmmc_host& rhs)=delete;
        sdmmc_host(const sdmmc_host&& rhs)=delete;
        sdmmc_host& operator=(sdmmc_host&& rhs)=delete;
        ~sdmmc_host()=delete;
        static void last_error(esp_err_t error) {
            m_last_error = error;
        }
    public:
        // TODO: not thread safe yet
        static bool acquire() {
            if(0==m_count) {
                ++m_count;
                esp_err_t res = sdmmc_host_init();
                if(ESP_OK!=res) {
                    --m_count;
                    m_last_error=res;
                    return false;
                } 
                return true;
            }
            ++m_count;
            return true;
        }
        // TODO: not thread safe yet
        static bool release() {
            if(2>m_count) {
                --m_count;
                esp_err_t res = sdmmc_host_deinit();
                if(ESP_OK!=res) {
                    ++m_count;
                    m_last_error=res;
                    return false;
                } 
                return true;
            }
            --m_count;
            return true;
        }
        inline static bool initialized() { return 0<m_count; }
        inline static esp_err_t last_error() { return m_last_error; }
        
    };
    std::atomic_int sdmmc_host::m_count {0};
    std::atomic<esp_err_t> sdmmc_host::m_last_error {ESP_OK};

    class sdmmc_host_slot final {
        int m_id;
    public:
        sdmmc_host_slot(int slot,sdmmc_slot_config_t& configuration) : m_id(-1) {
            if(!sdmmc_host::acquire())
                return;
            esp_err_t res = sdmmc_host_init_slot(slot,&configuration);
            if(ESP_OK!=res) {
                sdmmc_host::last_error(res);
                return;
            }
            m_id = slot;
        }
        sdmmc_host_slot(const sdmmc_host_slot& rhs)=delete;
        sdmmc_host_slot& operator=(const sdmmc_host_slot& rhs)=delete;
        sdmmc_host_slot(sdmmc_host_slot&& rhs) : m_id(rhs.m_id) {
            rhs.m_id=-1;
            // the other one will inevitably go away so we need our own reference
            sdmmc_host::acquire(); 
        }
        sdmmc_host_slot& operator=(sdmmc_host_slot&& rhs) {
            m_id = rhs.m_id;
            rhs.m_id=-1;
            // the other one will inevitably go away so we need our own reference
            sdmmc_host::acquire(); 
            return *this;
        }
        size_t bus_width() const {
            if(!initialized()) return 0;
            // there's no error result for this method!
            return sdmmc_host_get_slot_width(m_id);
        }
        bool bus_width(size_t value) {
            if(!initialized()) return false;
            esp_err_t res = sdmmc_host_set_bus_width(m_id,value);
            if(ESP_OK!=res) {
                sdmmc_host::last_error(res);
                return false;
            }
            return true;
        }
        ~sdmmc_host_slot() {
            sdmmc_host::release();
            m_id=-1;
        }
        inline esp_err_t last_error() const {
            return sdmmc_host::last_error();
        }
        inline bool initialized() const {return -1!=m_id;}
        inline int id() const { return m_id; }
    };
    class sdmmc_card {
        sdmmc_card_t m_card;
public:
        sdmmc_card(const sdmmc_host_slot& slot,sdmmc_host_t& config) {
            memset(&m_card,0,sizeof(m_card));
            config.slot=slot.id();
            esp_err_t res = sdmmc_card_init(&config,&m_card);
            if(ESP_OK!=res) {
                sdmmc_host::last_error(res);
            }
        }
        sdmmc_card(const sdmmc_card& rhs)=delete;
        sdmmc_card& operator=(const sdmmc_card& rhs)=delete;
        sdmmc_card(sdmmc_card&& rhs) : m_card(rhs.m_card) {
            memset(&rhs.m_card,0,sizeof(rhs.m_card));
        }
        sdmmc_card& operator=(sdmmc_card&& rhs) {
            m_card = rhs.m_card;
            memset(&rhs.m_card,0,sizeof(rhs.m_card));
            return *this;
        }
        ~sdmmc_card() {
        }
        inline bool initialized() const {
            return 0!=m_card.max_freq_khz;
        }
        inline esp_err_t last_error() const {
            return sdmmc_host::last_error();
        }
        // returns the supported frequency in khz
        inline uint16_t max_frequency() const {
            return m_card.max_freq_khz;
        }
        inline bool ddr() const {
            return 0!=m_card.is_ddr;
        }
        inline bool memory() const {
            return 0!=m_card.is_mem;
        }
        inline bool mmc() const {
            return 0!=m_card.is_mmc;
        }
        inline bool sdio() const {
            return 0!=m_card.is_sdio;
        }
        bool read(void* destination,size_t start_sector,size_t sector_count) {
            esp_err_t res = sdmmc_read_sectors(&m_card,destination,start_sector,sector_count);
            if(ESP_OK!=res) {
                sdmmc_host::last_error(res);
                return false;
            }
            return true;
        }
        bool write(const void* source,size_t start_sector,size_t sector_count) {
            esp_err_t res = sdmmc_write_sectors(&m_card,source,start_sector,sector_count);
            if(ESP_OK!=res) {
                sdmmc_host::last_error(res);
                return false;
            }
            return true;
        }
    };
}
#endif