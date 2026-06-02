#ifndef ALURA_FS_RAMFS_H
#define ALURA_FS_RAMFS_H

#include <stddef.h>

#include "vfs.h"
#include "kernel_heap.h"

#include "libc/string.h"

typedef struct ramfs_node_t ramfs_node_t;

typedef struct ramfs_node_t {
    ramfs_node_t** children;
    size_t children_count;
    vfs_node_t* node;
} ramfs_node_t;

// todo: temp
void ramfs_prepare_directory(const char* name, size_t children_count, vfs_node_t** result);
vfs_result_t ramfs_lookup(vfs_node_t* directory, const char* path, vfs_node_t** result);
vfs_result_t ramfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry);

#endif // ALURA_FS_RAMFS_H