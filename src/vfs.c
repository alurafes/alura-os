#include "vfs.h"

#include "fs/ramfs.h"
#include "print.h"

vfs_t vfs;
void vfs_module_init()
{
    ramfs_prepare_directory("/", 1, &vfs.root);

    vfs_node_t* test_node;
    ramfs_prepare_directory("test", 0, &test_node);

    ((ramfs_node_t*)vfs.root->fs_data)->children[0] = (ramfs_node_t*)test_node->fs_data;
    
    printf("root: %s\n", vfs.root->name);

    vfs_dir_t entry;
    size_t index = 0;
    vfs_result_t result = vfs_readdir(vfs.root, index++, &entry);
    while (result == VFS_RESULT_OK)
    {
        printf("    %s\n", entry.name);
        result = vfs_readdir(vfs.root, index++, &entry);
    }

    vfs_node_t* found_node;
    vfs_resolve(&vfs, "/test", &found_node);

    printf("Looking up a node /test\n");
    printf("%x\n", found_node);
}

vfs_result_t vfs_mount(vfs_node_t* point, vfs_node_t* node)
{
    if (!point) return VFS_RESULT_INVALID_MOUNT_POINT;
    if (!node) return VFS_RESULT_INVALID_NODE;
    point->mount = node;
    return VFS_RESULT_OK;
}

vfs_result_t vfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry)
{
    if (!directory) return VFS_RESULT_INVALID_NODE;
    if (!directory->operations.readdir) return VFS_RESULT_NODE_OPERATIONS_UNSET;
    return directory->operations.readdir(directory, index, entry);
}

vfs_result_t vfs_resolve(vfs_t* vfs, const char* path, vfs_node_t** result)
{
    vfs_node_t* current = vfs->root;
    char component[VFS_NODE_NAME_LENGTH];

    while (*path)
    {
        while (*path == '/') path++;
        if (*path == '\0') break;

        size_t component_length = 0;
        while (*path && *path != '/') component[component_length++] = *path++;
        component[component_length] = '\0';

        if (current->mount != NULL) current = current->mount;

        if (!current->operations.lookup) return VFS_RESULT_NODE_OPERATIONS_UNSET;

        vfs_result_t result = current->operations.lookup(current, component, &current);
        if (result != VFS_RESULT_OK) return result;

        if (!current) return VFS_RESULT_NODE_NULL;
    }

    *result = current;
    return VFS_RESULT_OK;
}
