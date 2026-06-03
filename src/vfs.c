#include "vfs.h"

#include "fs/ramfs.h"
#include "print.h"

vfs_t vfs;
void vfs_module_init()
{
    vfs.last_cache_index = 0;

    ramfs_node_t* root_node = NULL;
    ramfs_create_node(&ramfs, "/", VFS_NODE_TYPE_DIRECTORY, &root_node);
    ramfs_create_node(&ramfs, "dir_a", VFS_NODE_TYPE_DIRECTORY, &root_node->children[0]);
    ramfs_create_node(&ramfs, "dir_b", VFS_NODE_TYPE_DIRECTORY, &root_node->children[1]);
    ramfs_create_node(&ramfs, "dir_c", VFS_NODE_TYPE_DIRECTORY, &root_node->children[2]);
    root_node->children_count = 3;
    vfs_create_node(&vfs, NULL, root_node->id, root_node->name, &ramfs_node_operations, root_node, root_node->type, &vfs.root);

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

    vfs_resolve(&vfs, "/dir_a", &found_node);
    printf("Looking up a node /dir_a\n");
    printf("%x\n", found_node);
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

vfs_result_t vfs_cache_query_node(vfs_t* vfs, size_t cache_index, int64_t id, vfs_node_t** node)
{
    vfs_node_cache_t* cache_head = vfs->cache;
    while (cache_head != NULL)
    {
        if (cache_head->index == cache_index) break;
        cache_head = cache_head->next;
    }

    vfs_node_cache_node_t* node_head = cache_head->node;
    while (node_head != NULL)
    {
        if (node_head->node->index == id)
        {
            *node = node_head->node;
            return VFS_RESULT_OK;
        }
        node_head = node_head->next;
    }

    return VFS_RESULT_NOT_FOUND;
}

vfs_result_t vfs_create_node(vfs_t* vfs, vfs_node_t* parent, int64_t id, const char* name, vfs_node_operations_t* operations, void* fs_data, vfs_node_type type, vfs_node_t** result)
{
    if (!vfs || !operations || !result) return VFS_RESULT_BAD_PARAMETER;
    vfs_node_t* node = (vfs_node_t*)kernel_heap_calloc(sizeof(vfs_node_t));
    if (!node) return VFS_RESULT_ERROR;

    strcpy(node->name, name);
    node->operations = *operations;
    node->type = type;
    node->fs_data = fs_data;
    node->cache_index = parent ? parent->cache_index : (vfs->last_cache_index++);
    node->index = id;

    vfs_cache_put(vfs, node);

    *result = node;

    return VFS_RESULT_OK;
}

vfs_result_t vfs_cache_put(vfs_t* vfs, vfs_node_t* node)
{
    vfs_node_cache_t* cache_head_last = NULL;
    vfs_node_cache_t* cache_head = vfs->cache;
    uint32_t cache_allocated = 0;
    while (cache_head != NULL)
    {
        if (cache_head->index == node->cache_index) break;
        cache_head = cache_head->next;
        if (cache_head != NULL) cache_head_last = cache_head;
    }
    if (cache_head == NULL)
    {
        cache_head = (vfs_node_cache_t*)kernel_heap_calloc(sizeof(vfs_node_cache_t));
        if (!cache_head) return VFS_RESULT_ERROR;
        cache_allocated = 1;
        cache_head->index = node->cache_index;
        if (cache_head_last == NULL) vfs->cache = cache_head;
        else cache_head_last->next = cache_head;
    }

    vfs_node_cache_node_t* node_head = cache_head->node;
    vfs_node_cache_node_t* node_head_last = NULL;
    // check if already present. todo: need sets/maps plzzz future me 
    while (node_head != NULL)
    {
        if (node_head->node->index == node->index) return VFS_RESULT_NODE_ALREADY_PRESENT; 
        node_head = node_head->next;
        if (node_head != NULL) node_head_last = node_head;
    }

    node_head = (vfs_node_cache_node_t*)kernel_heap_calloc(sizeof(vfs_node_cache_node_t));
    if (!node_head)
    {
        if (cache_allocated) kernel_heap_free(cache_head);
        return VFS_RESULT_ERROR;
    }
    node_head->node = node;

    if (node_head_last == NULL) cache_head->node = node_head;
    else node_head_last->next = node_head;

    return VFS_RESULT_OK;
}