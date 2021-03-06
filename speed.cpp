//#define USE_SPI_MODE

extern "C"
{
    void app_main();
}

#include <chrono>
#include <iostream>

using namespace std;
typedef chrono::high_resolution_clock hires_clock_t;
// keep all the nonsense we need
// to set the sd_reader device up
// in another file to declutter the
// example
#include "sd_configuration.h"
void print_speed(double bps);
uint8_t block_buffer[16384];
void app_main()
{
    sd_reader reader = sd_configure();
    cout << "Size in GB: " << reader.size() / 1024.0 / 1024.0 / 1024.0 << endl;
    cout << "Free space in GB: " << reader.free() / 1024.0 / 1024.0 / 1024.0 << endl;
    // as long as an sd_reader is created we can use standard C and C++
    // file operations to work with files at paths rooted at the SD reader
    // mount_point - the filesystem uses POSIX style addressing
    
    for (int i = 0; i < sizeof(block_buffer); ++i)
    {
        block_buffer[i] = (uint8_t)(i & 0xFF);
    }
    FILE *fd = fopen("/sdcard/test.dat", "w");
    if(nullptr!=fd) {
        auto start_time = hires_clock_t::now();
        int i=0;
        for (; i < 256; i++)
        {
            size_t res = fwrite(block_buffer, sizeof(block_buffer), 1, fd);
            if(1!=res) {
                ESP_LOGE("perftest","sd write data failed .. wrote %llu bytes ",(unsigned long long)res*sizeof(block_buffer));
                break;
            }
        }
        auto end_time = hires_clock_t::now();
        double secs = chrono::duration_cast<chrono::milliseconds>(end_time - start_time).count() /1000.0;
        cout << "Wrote: "
            << (sizeof(block_buffer)*i)/1024.0/1024.0 
            << "MB in " 
            << secs
            << " s @ " ;
        print_speed(ftell(fd)/secs);
        cout << endl;
        
    }
    
    fclose(fd);
       
}
void print_speed(double bps) {
    if (bps >= (1024.0 * 1024.0 * 1024.0))
    {
        cout << bps / (1024.0 * 1024.0 * 1024.0);
        cout <<" GB/s";
    }
    if (bps >= (1024.0 * 1024.0))
    {
        cout << bps / (1024.0 * 1024.0);
        cout << " MB/s";
        return;
    }
    if (bps >= 1024)
    {
        cout << (bps / 1024.0);
        cout << " kB/s";
        return;
    }
    cout << bps;
    cout << " bytes/s";
}