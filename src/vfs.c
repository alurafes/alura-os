#include "vfs.h"

#include "fs/ramfs.h"
#include "print.h"

resource_operations_t vfs_operations = {
    .close = vfs_close
};

vfs_t vfs;
void vfs_module_init()
{
    vfs.last_cache_index = 0;
    vfs_create_node(&vfs, NULL, ramfs.root_node->id, ramfs.root_node->name, &ramfs_node_operations, ramfs.root_node, ramfs.root_node->type, &vfs.root);
    vfs_lock_node(vfs.root);
}

resource_result_t vfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry)
{
    if (!directory) return RESOURCE_RESULT_INVALID;
    if (!directory->operations.readdir) return RESOURCE_RESULT_BAD_PARAMETER;
    return directory->operations.readdir(directory, index, entry);
}

resource_result_t vfs_resolve(vfs_t* vfs, const char* path, vfs_node_t** result)
{
    vfs_node_t* current = vfs->root;
    vfs_lock_node(current);
    char component[VFS_NODE_NAME_LENGTH];

    while (*path)
    {
        while (*path == '/') path++;
        if (*path == '\0') break;

        
        size_t component_length = 0;
        while (*path && *path != '/') component[component_length++] = *path++;
        component[component_length] = '\0';
        
        if (current->mount != NULL) current = current->mount;
        
        if (!current->operations.lookup) return RESOURCE_RESULT_BAD_PARAMETER;
        
        vfs_node_t* next;

        resource_result_t result = current->operations.lookup(current, component, &next);
        if (result != RESOURCE_RESULT_OK) 
        {
            return result;
        }
        
        vfs_lock_node(next);
        vfs_release_node(current);

        current = next;

        if (!current) return RESOURCE_RESULT_NOT_FOUND;
    }

    *result = current;
    return RESOURCE_RESULT_OK;
}

resource_result_t vfs_cache_query_node(vfs_t* vfs, size_t cache_index, int64_t id, vfs_node_t** node)
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
            return RESOURCE_RESULT_OK;
        }
        node_head = node_head->next;
    }

    return RESOURCE_RESULT_NOT_FOUND;
}

resource_result_t vfs_create_node(vfs_t* vfs, vfs_node_t* parent, int64_t id, const char* name, vfs_node_operations_t* operations, void* fs_data, vfs_node_type type, vfs_node_t** result)
{
    if (!vfs || !operations || !result) return RESOURCE_RESULT_BAD_PARAMETER;
    vfs_node_t* node = (vfs_node_t*)kernel_heap_calloc(sizeof(vfs_node_t));
    if (!node) return RESOURCE_RESULT_ALLOCATION_ERROR;

    strcpy(node->name, name);
    node->operations = *operations;
    node->type = type;
    node->fs_data = fs_data;
    node->cache_index = parent ? parent->cache_index : (vfs->last_cache_index++);
    node->index = id;
    node->ref_count = 0;

    vfs_cache_put(vfs, node);

    *result = node;

    return RESOURCE_RESULT_OK;
}

resource_result_t vfs_cache_put(vfs_t* vfs, vfs_node_t* node)
{
    if (!vfs || !node) return RESOURCE_RESULT_BAD_PARAMETER;

    vfs_node_cache_t* cache_head_prev = NULL;
    vfs_node_cache_t* cache_head = vfs->cache;

    while (cache_head != NULL)
    {
        if (cache_head->index == node->cache_index) break;
        cache_head_prev = cache_head;
        cache_head = cache_head->next;
    }

    if (cache_head == NULL)
    {
        cache_head = (vfs_node_cache_t*)kernel_heap_calloc(sizeof(vfs_node_cache_t));
        if (!cache_head) return RESOURCE_RESULT_ALLOCATION_ERROR;

        cache_head->index = node->cache_index;
        if (cache_head_prev) cache_head_prev->next = cache_head;
        else vfs->cache = cache_head;
    }

    vfs_node_cache_node_t* node_head = cache_head->node;
    vfs_node_cache_node_t* node_head_prev = NULL;
    // check if already present. todo: need sets/maps plzzz future me 
    while (node_head != NULL)
    {
        if (node_head->node->index == node->index) return RESOURCE_RESULT_ALREADY_PRESENT; 
        node_head_prev = node_head;
        node_head = node_head->next;
    }

    node_head = (vfs_node_cache_node_t*)kernel_heap_calloc(sizeof(vfs_node_cache_node_t));
    if (!node_head) return RESOURCE_RESULT_ALLOCATION_ERROR;
    node_head->node = node;

    if (node_head_prev) node_head_prev->next = node_head;
    else cache_head->node = node_head;

    vfs_lock_node(node);
    printf("Node put in cache: %s\n", node->name);

    return RESOURCE_RESULT_OK;
}

resource_result_t vfs_lock_node(vfs_node_t* node)
{
    printf("Locking %s -> %d (+1)\n", node->name, node->ref_count);
    if (!node) return RESOURCE_RESULT_BAD_PARAMETER;
    node->ref_count++;
    return RESOURCE_RESULT_OK;
}

resource_result_t vfs_release_node(vfs_node_t* node)
{
    printf("Unlocking %s -> %d (-1)\n", node->name, node->ref_count);
    if (!node) return RESOURCE_RESULT_BAD_PARAMETER;
    node->ref_count--;
    vfs_cache_try_evict(&vfs, node);
    return RESOURCE_RESULT_OK;
}

resource_result_t vfs_cache_try_evict(vfs_t* vfs, vfs_node_t* node)
{
    if (!vfs || !node) return RESOURCE_RESULT_BAD_PARAMETER;
    if (!node || node->ref_count != 1) return RESOURCE_RESULT_STILL_IN_USE;
 
    printf("Trying to evict node from cache: %s\n", node->name);

    vfs_node_cache_t* cache_head_prev = NULL;
    vfs_node_cache_t* cache_head = vfs->cache;
    while (cache_head)
    {
        if (cache_head->index == node->cache_index) break;
        cache_head_prev = cache_head;
        cache_head = cache_head->next;
    }
    if (!cache_head) return RESOURCE_RESULT_NOT_FOUND;

    vfs_node_cache_node_t* node_head_prev = NULL;
    vfs_node_cache_node_t* node_head = cache_head->node;
    while (node_head)
    {
        if (node_head->node->index == node->index) break; 
        node_head_prev = node_head;
        node_head = node_head->next;
    }
    if (!node_head) return RESOURCE_RESULT_NOT_FOUND;

    if (node_head_prev) node_head_prev->next = node_head->next;
    else cache_head->node = node_head->next;
    
    kernel_heap_free(node_head);

    if (cache_head->node == NULL)
    {
        if (node_head_prev) cache_head_prev->next = cache_head->next;
        else vfs->cache = cache_head->next;
        kernel_heap_free(cache_head);
    }

    printf("Evicted %s\n", node->name);

    kernel_heap_free(node);

    return RESOURCE_RESULT_OK;
}

resource_result_t vfs_close(resource_t* resource)
{
    vfs_node_t* node = resource->data;
    return vfs_release_node(node);
}