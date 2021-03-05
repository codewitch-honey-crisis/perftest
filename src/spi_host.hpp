#ifndef HTCW_ESP32_SPI_HOST_HPP
#define HTCW_ESP32_SPI_HOST_HPP
extern "C" {
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/spi_common.h"
#include "sdkconfig.h"
}
namespace esp32 {
    // represents the root SPI host
    class spi_host {
        spi_host_device_t m_host_id;
        esp_err_t m_last_error;
        bool init(spi_host_device_t host_id, const spi_bus_config_t& bus_config,int dma_chain) {
            m_host_id = (spi_host_device_t)-1;
            esp_err_t ret = spi_bus_initialize(host_id,&bus_config,dma_chain);
            if(ESP_OK!=ret) {
                m_last_error=ret;
                ESP_LOGE("SPI","Failed to initialize SPI host");
                return false;
            }
            m_host_id = host_id;
            return true;
        }
        bool deinit() {
            esp_err_t ret = spi_bus_free(m_host_id);
            if(ESP_OK!=ret) {
                m_last_error=ret;
                ESP_LOGE("SPI","Failed to free SPI host");
                return false;
            }
            m_host_id = (spi_host_device_t)-1;
            return true;
        }
    public:
        spi_host(spi_host_device_t host_id,const spi_bus_config_t& bus_config,int dma_chain) {
            init(host_id,bus_config,dma_chain);
        }
        virtual ~spi_host() {
            if(initialized()) {
                deinit();
            }
        }
        spi_host(const spi_host& rhs)=delete;
        spi_host& operator=(const spi_host& rhs)=delete;
        spi_host(spi_host&& rhs) : m_host_id(rhs.m_host_id),m_last_error(rhs.m_last_error) {
            rhs.m_host_id=(spi_host_device_t)-1;
        }
        spi_host& operator=(spi_host&& rhs) {
            m_host_id = rhs.m_host_id;
            rhs.m_host_id=(spi_host_device_t)-1;
            m_last_error = rhs.m_last_error;
            return *this;
        }
        esp_err_t last_error() const {
            return m_last_error;
        }
        spi_host_device_t host_id() const {
            return m_host_id;
        }
        bool initialized() const {
            return ((spi_host_device_t)-1)!=m_host_id;
        }
    };
}
#endif