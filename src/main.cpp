// we'll be using fopen/fseek etc instead of i/o streams to avoid too much abstraction in the way
// of our results
#include <stdio.h>

#include "sdmmc_host.hpp"
#include "vfs.hpp"
#include <iostream>
using namespace std;
using namespace esp32;

extern "C" {
    void app_main();
}
void app_main() {
    multi_heap_info_t mhi;
    heap_caps_get_info(&mhi,MALLOC_CAP_DEFAULT);
    cout << "heap allocated "
        << mhi.total_allocated_bytes/1024.0
        << "kB, heap free "
        << mhi.total_free_bytes/1024.0
        << "kB"
        << endl;
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    sdmmc_host_t card_config = SDMMC_HOST_DEFAULT();
    card_config.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
    // 8 bit bus can'd do DDR
    if(slot_config.width==8) {
        card_config.flags&=~SDMMC_HOST_FLAG_DDR;
    }

    sdmmc_host_slot slot_1(SDMMC_HOST_SLOT_1,slot_config);

    if(!slot_1.initialized()) {
        cout << "Could not initalize SDMMC host slot 1" << endl;
        return;
    } else {
        cout << "Initialized SDMMC host slot 1" << endl;
    }

    sdmmc_card card(slot_1,card_config);

    if(!card.initialized()) {
        cout << "Could not initalize SD card" << endl;
        
    } else {
        cout << "Initialized SD card" << endl;
    }
    vfs_null null_fs;
    if(!vfs::mount("/null",&null_fs)) {
        cout << "Could not mount null filesystem" << endl;
        return;
    } else {
        cout << "Mounted null filesystem" << endl;
    }
    FILE* f = fopen("/null/foo","r");
    if(nullptr==f) {
        cout << "Could not open null file" << endl;
        return;
    }
    cout << "Reading from file" << endl;
    fgetc(f);
    cout << "Closing the file" << endl;
    fclose(f);
    vfs::unmount("/null");
    
}