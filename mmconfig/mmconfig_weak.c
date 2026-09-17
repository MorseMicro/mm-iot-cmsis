#include "mmconfig.h"


/** A default mm_litral_configs to avoid compile failure if the user
 * doesn't define mm_litral_configs in the application code.
 */
__attribute__((weak)) const struct mmconfig_literal_config mm_litral_configs[]={
    {NULL,NULL}
};

__attribute__((weak)) const struct mmhal_flash_partition_config *
mmhal_get_factory_mmconfig_partition(void)
{
    return NULL;
}
