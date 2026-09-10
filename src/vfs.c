#include "vfs.h"

#include "fs/ramfs.h"
#include "print.h"
#include "syscall.h"

resource_operations_t vfs_operations = {
    .close = vfs_close,
    .read = vfs_read,
    .write = vfs_write,
    .ioctl = vfs_ioctl,
};

vfs_t vfs;
void vfs_module_init()
{
    vfs.last_cache_index = 0;
    vfs_create_node(&vfs, NULL, ramfs.root_node->id, ramfs.root_node->name, &ramfs_node_operations, ramfs.root_node, ramfs.root_node->type, RESOURCE_TYPE_FILE, &vfs.root);
    vfs_lock_node(vfs.root);
}

resource_result_t vfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry)
{
    if (!directory) return RESOURCE_RESULT_INVALID;
    if (!directory->operations.readdir) return RESOURCE_RESULT_BAD_PARAMETER;
    return directory->operations.readdir(directory, index, entry);
}

resource_result_t vfs_get_size(vfs_node_t* node, size_t* out_size)
{
    if (!node || !out_size) return RESOURCE_RESULT_BAD_PARAMETER;
    if (!node->operations.size) return RESOURCE_RESULT_BAD_PARAMETER;
    return node->operations.size(node, out_size);
}

resource_result_t vfs_truncate(vfs_node_t* node)
{
    if (!node) return RESOURCE_RESULT_BAD_PARAMETER;
    if (node->type != VFS_NODE_TYPE_FILE) return RESOURCE_RESULT_INVALID;
    if (!node->operations.truncate) return RESOURCE_RESULT_BAD_PARAMETER;
    return node->operations.truncate(node);
}

static void vfs_split_path(char* path_copy, char** out_name, const char** out_parent_path)
{
    char* last_slash = strrchr(path_copy, '/');
    char* name = path_copy;
    const char* parent_path = "/";

    if (last_slash)
    {
        *last_slash = '\0';
        name = last_slash + 1;
        if (path_copy[0] != '\0') parent_path = path_copy;
    }

    *out_name = name;
    *out_parent_path = parent_path;
}

resource_result_t vfs_create(vfs_t* vfs, const char* path, vfs_node_type type, vfs_node_t** result)
{
    if (!vfs || !path || !result) return RESOURCE_RESULT_BAD_PARAMETER;

    char path_copy[VFS_NODE_NAME_LENGTH];
    strncpy(path_copy, path, VFS_NODE_NAME_LENGTH - 1);
    path_copy[VFS_NODE_NAME_LENGTH - 1] = '\0';

    char* name;
    const char* parent_path;
    vfs_split_path(path_copy, &name, &parent_path);

    if (*name == '\0') return RESOURCE_RESULT_BAD_PARAMETER;

    vfs_node_t* parent = NULL;
    resource_result_t resolve_result = vfs_resolve(vfs, parent_path, &parent);
    if (resolve_result != RESOURCE_RESULT_OK) return resolve_result;

    if (parent->type != VFS_NODE_TYPE_DIRECTORY || !parent->operations.create)
    {
        vfs_release_node(parent);
        return RESOURCE_RESULT_BAD_PARAMETER;
    }

    vfs_node_t* child = NULL;
    resource_result_t create_result = parent->operations.create(parent, name, type, &child);
    vfs_release_node(parent);
    if (create_result != RESOURCE_RESULT_OK) return create_result;

    vfs_lock_node(child);
    *result = child;
    return RESOURCE_RESULT_OK;
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
        
        if (current->mount != NULL)
        {
            current = current->mount;
            vfs_lock_node(current);
        }

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

resource_result_t vfs_create_node(vfs_t* vfs, vfs_node_t* parent, int64_t id, const char* name, vfs_node_operations_t* operations, void* fs_data, vfs_node_type type, resource_type_t resource_type, vfs_node_t** result)
{
    if (!vfs || !operations || !result) return RESOURCE_RESULT_BAD_PARAMETER;
    vfs_node_t* node = (vfs_node_t*)kernel_heap_calloc(sizeof(vfs_node_t));
    if (!node) return RESOURCE_RESULT_ALLOCATION_ERROR;

    strcpy(node->name, name);
    node->operations = *operations;
    node->type = type;
    node->resource_type = resource_type;
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

    return RESOURCE_RESULT_OK;
}

resource_result_t vfs_lock_node(vfs_node_t* node)
{
    if (!node) return RESOURCE_RESULT_BAD_PARAMETER;
    node->ref_count++;
    return RESOURCE_RESULT_OK;
}

resource_result_t vfs_release_node(vfs_node_t* node)
{
    if (!node) return RESOURCE_RESULT_BAD_PARAMETER;
    node->ref_count--;
    vfs_cache_try_evict(&vfs, node);
    return RESOURCE_RESULT_OK;
}

resource_result_t vfs_cache_try_evict(vfs_t* vfs, vfs_node_t* node)
{
    if (!vfs || !node) return RESOURCE_RESULT_BAD_PARAMETER;
    if (!node || node->ref_count != 1) return RESOURCE_RESULT_STILL_IN_USE;

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
        if (cache_head_prev) cache_head_prev->next = cache_head->next;
        else vfs->cache = cache_head->next;
        kernel_heap_free(cache_head);
    }

    if (node->operations.release) node->operations.release(node);

    kernel_heap_free(node);

    return RESOURCE_RESULT_OK;
}

resource_result_t vfs_close(resource_t* resource)
{
    vfs_node_t* node = resource->data;
    return vfs_release_node(node);
}

resource_result_t vfs_read(resource_t* resource, size_t offset, void* buffer, size_t length, size_t* read_bytes)
{
    if (!resource || !buffer || !length || !read_bytes) return RESOURCE_RESULT_BAD_PARAMETER;
    vfs_node_t* node = resource->data;

    if (node->type != VFS_NODE_TYPE_FILE) return RESOURCE_RESULT_INVALID;
    if (!node->operations.read) return RESOURCE_RESULT_BAD_PARAMETER;

    return node->operations.read(node, offset, buffer, length, read_bytes);
}

resource_result_t vfs_write(resource_t* resource, size_t offset, void* buffer, size_t length, size_t* written_bytes)
{
    if (!resource || !buffer || !length || !written_bytes) return RESOURCE_RESULT_BAD_PARAMETER;
    vfs_node_t* node = resource->data;

    if (node->type != VFS_NODE_TYPE_FILE) return RESOURCE_RESULT_INVALID;
    if (!node->operations.write) return RESOURCE_RESULT_BAD_PARAMETER;

    return node->operations.write(node, offset, buffer, length, written_bytes);
}

resource_result_t vfs_ioctl(resource_t* resource, int32_t command, int32_t argument)
{
    if (!resource) return RESOURCE_RESULT_BAD_PARAMETER;
    vfs_node_t* node = resource->data;

    if (!node->operations.ioctl) return RESOURCE_RESULT_BAD_PARAMETER;

    return node->operations.ioctl(node, command, argument);
}

resource_result_t vfs_open(vfs_t* vfs, task_t* task, const char* path, int32_t flags, size_t* result)
{
    vfs_node_t* node = NULL;
    resource_result_t resolve_result = vfs_resolve(vfs, path, &node);

    if (resolve_result != RESOURCE_RESULT_OK)
    {
        if (resolve_result != RESOURCE_RESULT_NOT_FOUND || !(flags & SYSCALL_O_CREAT)) return resolve_result;

        resolve_result = vfs_create(vfs, path, VFS_NODE_TYPE_FILE, &node);
        if (resolve_result != RESOURCE_RESULT_OK) return resolve_result;
    }
    else if (flags & SYSCALL_O_TRUNC)
    {
        if (node->type != VFS_NODE_TYPE_FILE) return RESOURCE_RESULT_BAD_PARAMETER;
        vfs_truncate(node);
    }

    if (node->type != VFS_NODE_TYPE_FILE) return RESOURCE_RESULT_BAD_PARAMETER;

    return resource_register(task, node->resource_type, node, &vfs_operations, flags, result);
}

resource_result_t vfs_link(vfs_t* vfs, const char* old_path, const char* new_path)
{
    if (!vfs || !old_path || !new_path) return RESOURCE_RESULT_BAD_PARAMETER;

    vfs_node_t* source = NULL;
    resource_result_t resolve_result = vfs_resolve(vfs, old_path, &source);
    if (resolve_result != RESOURCE_RESULT_OK) return resolve_result;

    // only plain files can be hardlinked
    if (source->type != VFS_NODE_TYPE_FILE || source->resource_type != RESOURCE_TYPE_FILE)
    {
        vfs_release_node(source);
        return RESOURCE_RESULT_BAD_PARAMETER;
    }

    char path_copy[VFS_NODE_NAME_LENGTH];
    strncpy(path_copy, new_path, VFS_NODE_NAME_LENGTH - 1);
    path_copy[VFS_NODE_NAME_LENGTH - 1] = '\0';

    char* name;
    const char* parent_path;
    vfs_split_path(path_copy, &name, &parent_path);

    if (*name == '\0')
    {
        vfs_release_node(source);
        return RESOURCE_RESULT_BAD_PARAMETER;
    }

    vfs_node_t* parent = NULL;
    resolve_result = vfs_resolve(vfs, parent_path, &parent);
    if (resolve_result != RESOURCE_RESULT_OK)
    {
        vfs_release_node(source);
        return resolve_result;
    }

    if (parent->type != VFS_NODE_TYPE_DIRECTORY || !parent->operations.link)
    {
        vfs_release_node(parent);
        vfs_release_node(source);
        return RESOURCE_RESULT_BAD_PARAMETER;
    }

    resource_result_t link_result = parent->operations.link(parent, name, source);

    vfs_release_node(parent);
    vfs_release_node(source);

    return link_result;
}

resource_result_t vfs_unlink(vfs_t* vfs, const char* path)
{
    if (!vfs || !path) return RESOURCE_RESULT_BAD_PARAMETER;

    char path_copy[VFS_NODE_NAME_LENGTH];
    strncpy(path_copy, path, VFS_NODE_NAME_LENGTH - 1);
    path_copy[VFS_NODE_NAME_LENGTH - 1] = '\0';

    char* name;
    const char* parent_path;
    vfs_split_path(path_copy, &name, &parent_path);

    if (*name == '\0') return RESOURCE_RESULT_BAD_PARAMETER;

    vfs_node_t* parent = NULL;
    resource_result_t resolve_result = vfs_resolve(vfs, parent_path, &parent);
    if (resolve_result != RESOURCE_RESULT_OK) return resolve_result;

    if (parent->type != VFS_NODE_TYPE_DIRECTORY || !parent->operations.unlink)
    {
        vfs_release_node(parent);
        return RESOURCE_RESULT_BAD_PARAMETER;
    }

    resource_result_t unlink_result = parent->operations.unlink(parent, name);
    vfs_release_node(parent);

    return unlink_result;
}