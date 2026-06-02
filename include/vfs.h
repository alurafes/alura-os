#ifndef ALURA_VFS_H
#define ALURA_VFS_H

#include <stddef.h>

#include "kernel_heap.h"

#define VFS_NODE_NAME_LENGTH 256

typedef enum vfs_result_t {
    VFS_RESULT_OK,
    VFS_RESULT_INVALID_MOUNT_POINT,
    VFS_RESULT_INVALID_NODE,
    VFS_RESULT_INVALID_NODE_TYPE,
    VFS_RESULT_NODE_OPERATIONS_UNSET,
    VFS_RESULT_NODE_NULL,
    VFS_RESULT_OUT_OF_BOUNDS,
    VFS_RESULT_NOT_FOUND
} vfs_result_t;

typedef struct vfs_node_t vfs_node_t;
typedef struct vfs_dir_t vfs_dir_t;

typedef struct vfs_node_operations_t {
    vfs_result_t (*lookup)(vfs_node_t* directory, const char* path, vfs_node_t** result);
    vfs_result_t (*readdir)(vfs_node_t* directory, size_t index, vfs_dir_t* entry);
} vfs_node_operations_t;

typedef enum vfs_node_type {
    VFS_NODE_TYPE_FILE,
    VFS_NODE_TYPE_DIRECTORY
} vfs_node_type;

typedef struct vfs_node_t {
    char name[VFS_NODE_NAME_LENGTH];
    // fs_node_t* parent;
    vfs_node_t* mount;
    vfs_node_operations_t operations;
    vfs_node_type type;
    void* fs_data;
} vfs_node_t;

typedef struct vfs_dir_t {
    char name[VFS_NODE_NAME_LENGTH];
    vfs_node_type type;
} vfs_dir_t;

typedef struct vfs_t {
    vfs_node_t* root;
} vfs_t;

extern vfs_t vfs;
void vfs_module_init();

vfs_result_t vfs_mount(vfs_node_t* point, vfs_node_t* node);
vfs_result_t vfs_resolve(vfs_t* vfs, const char* path, vfs_node_t** result);
vfs_result_t vfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry);

#endif // ALURA_VFS_H