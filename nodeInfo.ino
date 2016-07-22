//nodeInfo.ino








const char * const RST_REASONS[] =
{
    "REASON_DEFAULT_RST",
    "REASON_WDT_RST",
    "REASON_EXCEPTION_RST",
    "REASON_SOFT_WDT_RST",
    "REASON_SOFT_RESTART",
    "REASON_DEEP_SLEEP_AWAKE",
    "REASON_EXT_SYS_RST"
};

const char * const FLASH_SIZE_MAP_NAMES[] =
{
    "FLASH_SIZE_4M_MAP_256_256",
    "FLASH_SIZE_2M",
    "FLASH_SIZE_8M_MAP_512_512",
    "FLASH_SIZE_16M_MAP_512_512",
    "FLASH_SIZE_32M_MAP_512_512",
    "FLASH_SIZE_16M_MAP_1024_1024",
    "FLASH_SIZE_32M_MAP_1024_1024"
};
#ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif

String getESPStatusUpdate()
{
	String infoStrin = "";

    const rst_info * resetInfo = system_get_rst_info();
    infoStrin += "\nsystem_get_rst_info() reset reason: ";
    infoStrin += RST_REASONS[resetInfo->reason];
    infoStrin += "\nsystem_get_free_heap_size(): ";
    infoStrin += system_get_free_heap_size();
    infoStrin += "\nsystem_get_os_print(): ";
    infoStrin += system_get_os_print();
    infoStrin += "\nsystem_get_os_print(): ";
    infoStrin += system_get_os_print();
    infoStrin += "\nsystem_get_chip_id(): 0x";
    infoStrin += system_get_chip_id();
    infoStrin += "\nsystem_get_sdk_version(): ";
    infoStrin += system_get_sdk_version();
    infoStrin += "\nsystem_get_boot_version(): ";
    infoStrin += system_get_boot_version();
    infoStrin += "\nsystem_get_userbin_addr(): 0x";
    infoStrin += system_get_userbin_addr();
    infoStrin += "\nsystem_get_boot_mode(): ";
    infoStrin += (system_get_boot_mode() == 0) ? "\nSYS_BOOT_ENHANCE_MODE" : "\nSYS_BOOT_NORMAL_MODE";
    infoStrin += "\nsystem_get_cpu_freq(): ";
    infoStrin += system_get_cpu_freq();
    infoStrin += "\nsystem_get_flash_size_map(): ";
    infoStrin += FLASH_SIZE_MAP_NAMES[system_get_flash_size_map()];

    return infoStrin;
}