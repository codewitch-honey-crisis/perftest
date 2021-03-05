#ifndef HTCW_ESP32_VFS_HPP
#define HTCW_ESP32_VFS_HPP
#include <sys/stat.h>
#include <sys/dirent.h>
#include <utime.h>
#include "esp_vfs.h"
#include <atomic>
namespace esp32 {
    class vfs_driver {
    public:
        virtual ssize_t write(int fd, const void * data, size_t size)=0;
        virtual off_t lseek(int fd, off_t size, int mode)=0;
        virtual ssize_t read(int fd, void * dst, size_t size)=0;
        virtual ssize_t pread(int fd, void *dst, size_t size, off_t offset)=0;
        virtual ssize_t pwrite(int fd, const void *src, size_t size, off_t offset)=0;
        virtual int open(const char * path, int flags, int mode)=0;
        virtual int close(int fd)=0;
        virtual int fstat(int fd, struct stat * st)=0;
        virtual int fsync(int fd)=0;
#ifdef CONFIG_VFS_SUPPORT_DIR    
        virtual int stat(const char * path, struct stat * st)=0;
        virtual int link(const char* n1, const char* n2)=0;
        virtual int unlink(const char *path)=0;
        virtual int rename(const char *src, const char *dst)=0;
        virtual DIR* opendir(const char* name)=0;
        virtual dirent* readdir(DIR* pdir)=0;
        virtual int readdir_r(DIR* pdir, struct dirent* entry, struct dirent** out_dirent)=0;
        virtual long telldir(DIR* pdir)=0;
        virtual void seekdir(DIR* pdir, long offset)=0;
        virtual int closedir(DIR* pdir)=0;
        virtual int mkdir(const char* name, mode_t mode)=0;
        virtual int rmdir(const char* name)=0;
        virtual int access(const char *path, int amode)=0;
        virtual int truncate(const char *path, off_t length)=0;
        virtual int utime(const char *path, const struct utimbuf *times)=0;
#endif // CONFIG_VFS_SUPPORT_DIR
    };
    class vfs_null : public vfs_driver {
        std::atomic_flag m_opened_file;
        virtual ssize_t write(int fd, const void * data, size_t size) {
            return size;
        }
        virtual off_t lseek(int fd, off_t size, int mode) {
            return off_t();
        }
        virtual ssize_t read(int fd, void * dst, size_t size){
            memset(dst,0,size);
            return size;
        }
        virtual ssize_t pread(int fd, void *dst, size_t size, off_t offset) {
            memset(dst,0,size);
            return size;
        }
        virtual ssize_t pwrite(int fd, const void *src, size_t size, off_t offset) {
            return size;
        }
        virtual int open(const char * path, int flags, int mode) {
           if(!m_opened_file.test_and_set()) {                
                return 0;
            }
            return -1;
        }
        virtual int close(int fd) {
            if(m_opened_file.test_and_set()) {
                m_opened_file.clear();
                return 0;
            }
            return -1;
        }
        virtual int fstat(int fd, struct stat * st) {
            st->st_size=0;
            st->st_mode=S_IRWXU|S_IRWXU | S_IRWXG | S_IRWXO | S_IFREG;
            st->st_mtime = 0;
            st->st_atime = 0;
            st->st_ctime = 0;
            return 0;
        }
        virtual int fsync(int fd) {
            return 0;
        }
#ifdef CONFIG_VFS_SUPPORT_DIR
        virtual int stat(const char * path, struct stat * st) {
            return -1;
        };
        virtual int link(const char* n1, const char* n2) {
            return -1;
        };
        virtual int unlink(const char *path) {
            return -1;
        };
        virtual int rename(const char *src, const char *dst) {
            return -1;
        }
        virtual DIR* opendir(const char* name) {
            return nullptr;
        }
        virtual dirent* readdir(DIR* pdir) {
            return nullptr;
        }
        virtual int readdir_r(DIR* pdir, struct dirent* entry, struct dirent** out_dirent) {
            return -1;
        }
        virtual long telldir(DIR* pdir) {
            return -1;
        }
        virtual void seekdir(DIR* pdir, long offset) {
            return;
        }
        virtual int closedir(DIR* pdir) {
            return -1;
        }
        virtual int mkdir(const char* name, mode_t mode) {
            return -1;
        }
        virtual int rmdir(const char* name) {
            return -1;
        }
        virtual int access(const char *path, int amode) {
            return -1;
        }
        virtual int truncate(const char *path, off_t length) {
            return -1;
        }
        virtual int utime(const char *path, const struct utimbuf *times) {
            return -1;
        }
#endif // CONFIG_VFS_SUPPORT_DIR
    };
    class vfs final {
        static std::atomic<esp_err_t> m_last_error;
        vfs()=delete;
        vfs(const vfs& rhs)=delete;
        vfs& operator=(vfs& rhs)=delete;
        vfs(const vfs&& rhs)=delete;
        vfs& operator=(vfs&& rhs)=delete;
        ~vfs()=delete;

        static ssize_t write(void* ctx, int fd, const void * data, size_t size) { return reinterpret_cast<vfs_driver*>(ctx)->write(fd,data,size); }
        static off_t lseek(void* ctx, int fd, off_t size, int mode) { return reinterpret_cast<vfs_driver*>(ctx)->lseek(fd,size,mode); }
        static ssize_t read(void* ctx, int fd, void * dst, size_t size) { return reinterpret_cast<vfs_driver*>(ctx)->read(fd,dst,size); }
        static ssize_t pread(void* ctx, int fd, void *dst, size_t size, off_t offset) { return reinterpret_cast<vfs_driver*>(ctx)->pread(fd,dst,size,offset); }
        static ssize_t pwrite(void* ctx, int fd, const void *src, size_t size, off_t offset) { return reinterpret_cast<vfs_driver*>(ctx)->pwrite(fd,src,size,offset); }
        static int open(void* ctx, const char * path, int flags, int mode) { return reinterpret_cast<vfs_driver*>(ctx)->open(path,flags,mode); }
        static int close(void* ctx, int fd) { return reinterpret_cast<vfs_driver*>(ctx)->close(fd); }
        static int fstat(void* ctx, int fd, struct stat * st) { return reinterpret_cast<vfs_driver*>(ctx)->fstat(fd,st); }
        static int fsync(void* ctx, int fd) { return reinterpret_cast<vfs_driver*>(ctx)->fsync(fd); }
#ifdef CONFIG_VFS_SUPPORT_DIR    
        static int stat(void* ctx, const char * path, struct stat * st) { return reinterpret_cast<vfs_driver*>(ctx)->stat(path,st); }
        static int link(void* ctx, const char* n1, const char* n2) { return reinterpret_cast<vfs_driver*>(ctx)->link(n1,n2); }
        static int unlink(void* ctx, const char *path) { return reinterpret_cast<vfs_driver*>(ctx)->unlink(path); }
        static int rename(void* ctx, const char *src, const char *dst) { return reinterpret_cast<vfs_driver*>(ctx)->rename(src,dst); }
        static DIR* opendir(void* ctx, const char* name) { return reinterpret_cast<vfs_driver*>(ctx)->opendir(name); }
        static dirent* readdir(void* ctx, DIR* pdir) { return reinterpret_cast<vfs_driver*>(ctx)->readdir(pdir); }
        static int readdir_r(void* ctx, DIR* pdir, struct dirent* entry, struct dirent** out_dirent) { return reinterpret_cast<vfs_driver*>(ctx)->readdir_r(pdir,entry,out_dirent); }
        static long telldir(void* ctx, DIR* pdir) { return reinterpret_cast<vfs_driver*>(ctx)->telldir(pdir); }
        static void seekdir(void* ctx, DIR* pdir, long offset) { return reinterpret_cast<vfs_driver*>(ctx)->seekdir(pdir,offset); }
        static int closedir(void* ctx, DIR* pdir) { return reinterpret_cast<vfs_driver*>(ctx)->closedir(pdir); }
        static int mkdir(void* ctx, const char* name, mode_t mode) { return reinterpret_cast<vfs_driver*>(ctx)->mkdir(name,mode); }
        static int rmdir(void* ctx, const char* name) { return reinterpret_cast<vfs_driver*>(ctx)->rmdir(name); }
        static int access(void* ctx, const char *path, int amode) { return reinterpret_cast<vfs_driver*>(ctx)->access(path,amode); }
        static int truncate(void* ctx, const char *path, off_t length) { return reinterpret_cast<vfs_driver*>(ctx)->truncate(path,length); }
        static int utime(void* ctx, const char *path, const struct utimbuf *times) { return reinterpret_cast<vfs_driver*>(ctx)->utime(path,times); }
#endif // CONFIG_VFS_SUPPORT_DIR  
    public:

        static bool mount(const char* mount_point,vfs_driver* driver) {
            if(nullptr==mount_point || nullptr==driver) {
                m_last_error=ESP_ERR_INVALID_ARG;
                return false;
            }

            // should be const but I needed to shut the compiler up
            esp_vfs_t vfs = {};
            
            vfs.flags = ESP_VFS_FLAG_CONTEXT_PTR;
            vfs.write_p = &vfs::write;
            vfs.lseek_p= &vfs::lseek;
            vfs.read_p=&vfs::read;
            vfs.pread_p=&vfs::pread;
            vfs.pwrite_p=&vfs::pwrite;
            vfs.open_p=&vfs::open;
            vfs.close_p=&vfs::close;
            vfs.fstat_p=&vfs::fstat;
            vfs.fsync_p=&vfs::fsync;
#ifdef CONFIG_VFS_SUPPORT_DIR
            vfs.stat_p=&vfs::stat;
            vfs.link_p=&vfs::link;
            vfs.unlink_p=&vfs::unlink;
            vfs.rename_p=&vfs::rename;
            vfs.opendir_p=&vfs::opendir;
            vfs.readdir_p=&vfs::readdir;
            vfs.readdir_r_p=&vfs::readdir_r;
            vfs.telldir_p=&vfs::telldir;
            vfs.seekdir_p=&vfs::seekdir;
            vfs.closedir_p=&vfs::closedir;
            vfs.mkdir_p=&vfs::mkdir;
            vfs.rmdir_p=&vfs::rmdir;
            vfs.access_p=&vfs::access;
            vfs.truncate_p=&vfs::truncate;
            vfs.utime_p=&vfs::utime;
#endif
            
            
            esp_err_t res = esp_vfs_register(mount_point,&vfs,driver);
            if(ESP_OK!=res) {
                m_last_error = res;
                return false;
            }
            return true;
        }
        static bool unmount(const char* mount_point) {
            if(nullptr==mount_point) {
                m_last_error=ESP_ERR_INVALID_ARG;
                return false;
            }
            esp_err_t res = esp_vfs_unregister(mount_point);
            if(ESP_OK!=res) {
                m_last_error = res;
                return false;
            }
            return true;
        }
        
    };
    std::atomic<esp_err_t> vfs::m_last_error {ESP_OK};

}
#endif