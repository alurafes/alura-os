#include "fs/ramfs.h"

void ramfs_prepare_directory(const char* name, size_t children_count, vfs_node_t** result)
{
    vfs_node_t* root = (vfs_node_t*)kernel_heap_calloc(sizeof(vfs_node_t));
    if (!root) 
    {
        // todo: I STILL DONT HAVE A PANIC FUNCTION
        return;
    }
    strcpy(root->name, name);
    root->operations.lookup = ramfs_lookup;
    root->operations.readdir = ramfs_readdir;
    root->type = VFS_NODE_TYPE_DIRECTORY;

    ramfs_node_t* node = (ramfs_node_t*)kernel_heap_calloc(sizeof(ramfs_node_t));
    node->node = root;
    node->children_count = children_count;
    if (children_count > 0) {
        node->children = (ramfs_node_t**)kernel_heap_calloc(sizeof(ramfs_node_t*) * children_count);
    }

    root->fs_data = node;
    *result = root;
}

vfs_result_t ramfs_lookup(vfs_node_t* directory, const char* path, vfs_node_t** result)
{
    ramfs_node_t* node = directory->fs_data;
    for (size_t i = 0; i < node->children_count; ++i)
    {
        ramfs_node_t* child = node->children[i];
        if (strcmp(child->node->name, path) == 0)
        {
            *result = child->node;
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
    strcpy(entry->name, child->node->name);
    entry->type = child->node->type;

    return VFS_RESULT_OK;
}