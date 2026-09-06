#ifndef ALURA_FS_DEVFS_H
#define ALURA_FS_DEVFS_H

#include "vfs.h"

extern vfs_node_operations_t devfs_node_operations;

void devfs_module_init();

resource_result_t devfs_lookup(vfs_node_t* directory, const char* name, vfs_node_t** result);
resource_result_t devfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry);

#endif // ALURA_FS_DEVFS_H
