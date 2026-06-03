#include "fs/ramfs.h"

vfs_node_operations_t ramfs_node_operations = (vfs_node_operations_t){
    .lookup = ramfs_lookup,
    .readdir = ramfs_readdir
};

vfs_result_t ramfs_lookup(vfs_node_t* directory, const char* path, vfs_node_t** result)
{
    ramfs_node_t* node = directory->fs_data;
    for (size_t i = 0; i < node->children_count; ++i)
    {
        ramfs_node_t* child = node->children[i];
        if (strcmp(child->name, path) == 0)
        {
            vfs_result_t queue_result = vfs_cache_query_node(&vfs, directory->cache_index, child->id, result);
            if (queue_result == VFS_RESULT_OK) return VFS_RESULT_OK;
            vfs_result_t create_result = vfs_create_node(&vfs, directory, child->id, child->name, &ramfs_node_operations, child, child->type, result);
            if (create_result != VFS_RESULT_OK) return create_result;
            return VFS_RESULT_OK;
        }
    }
    return VFS_RESULT_NOT_FOUND;
}

vfs_result_t ramfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry)
{
    ramfs_node_t* node = directory->fs_data;
    if (index >= node->children_count) return VFS_RESULT_OUT_OF_BOUNDS;

    ramfs_node_t* child = node->children[index];
    strcpy(entry->name, child->name);
    entry->type = child->type;

    return VFS_RESULT_OK;
}

// ---

ramfs_t ramfs;
void ramfs_driver_init()
{
    ramfs.last_index = 0;
}

ramfs_result_t ramfs_create_node(ramfs_t* ramfs, const char* name, vfs_node_type type, ramfs_node_t** result)
{
    ramfs_node_t* node = (ramfs_node_t*)kernel_heap_calloc(sizeof(ramfs_node_t));
    if (!node) return RAMFS_RESULT_ERROR;

    node->id = ramfs->last_index++;
    node->children_count = 0;
    strcpy(node->name, name); // todo: better copy function
    node->type = type;

    *result = node;

    return RAMFS_RESULT_OK;
}