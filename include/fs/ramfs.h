#ifndef ALURA_FS_RAMFS_H
#define ALURA_FS_RAMFS_H

#include <stdint.h>
#include <stddef.h>

#include "vfs.h"
#include "kernel_heap.h"

#include "libc/string.h"

#define RAMFS_NODE_MAX_CHILDREN 64

extern vfs_node_operations_t ramfs_node_operations;

typedef struct ramfs_node_t {
    uint64_t id;
    // this kinda smells
    struct ramfs_node_t* children[RAMFS_NODE_MAX_CHILDREN];
    size_t children_count;
    char name[VFS_NODE_NAME_LENGTH];
    vfs_node_type type;
    void* data;
    size_t data_size;
} ramfs_node_t;

typedef enum ramfs_result_t {
    RAMFS_RESULT_OK = 0,
    RAMFS_RESULT_ERROR
} ramfs_result_t;

typedef struct ramfs_t {
    int64_t last_index;
    ramfs_node_t* root_node;
} ramfs_t;

// ---

extern ramfs_t ramfs;
void ramfs_driver_init();

ramfs_result_t ramfs_create_node(ramfs_t* ramfs, const char* name, vfs_node_type type, void* data, size_t data_size, ramfs_node_t** result);
resource_result_t ramfs_lookup(vfs_node_t* directory, const char* path, vfs_node_t** result);
resource_result_t ramfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry);
resource_result_t ramfs_read(vfs_node_t* file, size_t offset, void* buffer, size_t length, size_t* read_bytes);

#endif // ALURA_FS_RAMFS_H