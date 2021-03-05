#ifndef HTCW_ESP32_VFS_FAST_FAT32_HPP
#define HTCW_ESP32_VFS_FAST_FAT32_HPP
#include "vfs.hpp"
namespace esp32
{
    enum vfs_fast_fat32_disk_status : uint8_t 
    {
        not_initialized=0x01,
        no_disk=0x02,
        write_protected=0x04
    };
    enum vfs_fast_fat32_attributes : uint8_t
    {
        read_only = 0x01,
        hidden = 0x02,
        system = 0x04,
        directory = 0x10,
        archive = 0x20
    };
    enum vfs_fast_fat32_hal_result : uint8_t
    {
        success = 0,
        io_error = 1,
        write_protected = 2,
        not_ready = 3,
        invalid_paramter = 4
    };
    enum vfs_fast_fat32_ioctl_command : uint8_t
    {
        // generic commands
        control_sync = 0,     // Complete pending write process
        get_sector_count = 1, // Get media sector count
        get_sector_size = 2,  // Get sector size
        get_block_size = 3,   // Get erase block size
        control_trim = 4,     // Inform device that the data on the block of sectors is no longer

        // not used
        control_power = 5,  // Get/Set power status
        control_lock = 6,   // Lock/Unlock media removal
        control_eject = 7,  // Eject media
        control_format = 8, // Create physical format on the media

        // MMC/SDC specific ioctl command
        mmc_get_type = 10,       // Get card type
        mmc_get_csd = 11,        // Get CSD
        mmc_get_cid = 12,        // Get CID
        mmc_get_ocr = 13,        // Get OCR
        mmc_get_sdstat = 14,     // Get SD status
        isdio_read = 55,         // Read data form SD iSDIO register
        isdio_write = 56,        // Write data to SD iSDIO register
        isdio_masked_write = 57, // Masked write data to SD iSDIO register

        // ATA/CF specific ioctl command
        ata_get_revision = 20,     // Get F/W revision
        ata_get_model = 21,        // Get model name
        ata_get_serial_number = 22 // Get serial number
    };
    class vfs_fast_fat32_hal
    {
        virtual vfs_fast_fat32_disk_status initialize(uint8_t pdrv) = 0;
        virtual vfs_fast_fat32_disk_status status(uint8_t pdrv) = 0;
        virtual vfs_fast_fat32_hal_result read(uint8_t pdrv, void *buffer, uint32_t sector, unsigned int count) = 0;
        virtual vfs_fast_fat32_hal_result write(uint8_t pdrv, const void *buffer, uint32_t sector, unsigned int count) = 0;
        virtual vfs_fast_fat32_hal_result ioctl(uint8_t pdrv, vfs_fast_fat32_ioctl_command command, void *buffer) = 0;
    };
    class vfs_fast_fat32 : public vfs_driver
    {
        struct object_id
        {
            uint16_t mount_id;                  // volume mount id
            vfs_fast_fat32_attributes attributes; // attribute
            uint8_t stat;                       // stat
            uint32_t start_cluster;
            uint32_t size; // size when start_cluster!=0
        };
        struct file_system_entry
        {
            object_id id;
        };
        struct file_entry : public file_system_entry
        {
            uint8_t flags;
            uint8_t error;
            uint32_t position;
            uint32_t cluster;
            uint32_t sector;
            uint32_t directory_sector;
            uint8_t *directory_pointer;
            uint8_t buffer[16384];
        };
        struct directory_entry : public file_system_entry
        {
            uint32_t position;
            uint32_t cluster;
            uint32_t sector;
            uint8_t *directory_pointer;
            uint8_t fn[12];
        };
        struct file_info
        {
            uint32_t size;
            uint16_t modified_date;
            uint16_t modified_time;
            vfs_fast_fat32_attributes attributes;
            char name[13];
        };
        static uint32_t get_time()
        {
            time_t t = time(NULL);
            struct tm tmr;
            localtime_r(&t, &tmr);
            int year = tmr.tm_year < 80 ? 0 : tmr.tm_year - 80;
            return ((uint32_t)(year) << 25) | ((uint32_t)(tmr.tm_mon + 1) << 21) | ((uint32_t)tmr.tm_mday << 16) | (uint32_t)(tmr.tm_hour << 11) | (uint32_t)(tmr.tm_min << 5) | (uint32_t)(tmr.tm_sec >> 1);
        }
    };
}
#endif
