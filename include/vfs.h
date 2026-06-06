#ifndef ALURA_VFS_H
#define ALURA_VFS_H

#include <stddef.h>

#include "kernel_heap.h"
#include "resourse.h"

#define VFS_NODE_NAME_LENGTH 256

extern resource_operations_t vfs_operations;

typedef struct vfs_node_t vfs_node_t;
typedef struct vfs_dir_t vfs_dir_t;

typedef struct vfs_node_operations_t {
    resource_result_t (*lookup)(vfs_node_t* directory, const char* path, vfs_node_t** result);
    resource_result_t (*readdir)(vfs_node_t* directory, size_t index, vfs_dir_t* entry);
    resource_result_t (*read)(vfs_node_t* file, size_t offset, void* buffer, size_t length, size_t* read_bytes);
} vfs_node_operations_t;

typedef enum vfs_node_type {
    VFS_NODE_TYPE_FILE = 0,
    VFS_NODE_TYPE_DIRECTORY
} vfs_node_type;

typedef struct vfs_node_t {
    char name[VFS_NODE_NAME_LENGTH];
    vfs_node_t* mount;
    vfs_node_operations_t operations;
    vfs_node_type type;
    void* fs_data;
    int64_t cache_index;
    int64_t index;
    size_t ref_count;
} vfs_node_t;

typedef struct vfs_dir_t {
    char name[VFS_NODE_NAME_LENGTH];
    vfs_node_type type;
} vfs_dir_t;


// todo: i reaaaally need a hash map or something like that
typedef struct vfs_node_cache_node_t {
    vfs_node_t* node;
    struct vfs_node_cache_node_t* next;
} vfs_node_cache_node_t;

typedef struct vfs_node_cache_t {
    int64_t index;
    vfs_node_cache_node_t* node;
    struct vfs_node_cache_t* next;
} vfs_node_cache_t;

// ---

typedef struct vfs_t {
    vfs_node_t* root;
    vfs_node_cache_t* cache;
    int64_t last_cache_index;
} vfs_t;

extern vfs_t vfs;
void vfs_module_init();

resource_result_t vfs_create_node(vfs_t* vfs, vfs_node_t* parent, int64_t id, const char* name, vfs_node_operations_t* operations, void* fs_data, vfs_node_type type, vfs_node_t** result);
resource_result_t vfs_lock_node(vfs_node_t* node);
resource_result_t vfs_release_node(vfs_node_t* node);
resource_result_t vfs_resolve(vfs_t* vfs, const char* path, vfs_node_t** result);
resource_result_t vfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry);

resource_result_t vfs_cache_query_node(vfs_t* vfs, size_t cache_index, int64_t id, vfs_node_t** node);
resource_result_t vfs_cache_put(vfs_t* vfs, vfs_node_t* node);
resource_result_t vfs_cache_try_evict(vfs_t* vfs, vfs_node_t* node);

resource_result_t vfs_close(resource_t* resource);
resource_result_t vfs_read(resource_t* resource, size_t offset, void* buffer, size_t length, size_t* read_bytes);

#endif // ALURA_VFS_H