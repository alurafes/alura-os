#include "fs/ramfs.h"
#include "print.h"

#include "fs/ustar.h"
#include "memory_paging.h"

vfs_node_operations_t ramfs_node_operations = (vfs_node_operations_t){
    .lookup = ramfs_lookup,
    .readdir = ramfs_readdir,
    .read = ramfs_read
};

resource_result_t ramfs_lookup(vfs_node_t* directory, const char* path, vfs_node_t** out)
{
    ramfs_node_t* node = directory->fs_data;
    for (size_t i = 0; i < node->children_count; ++i)
    {
        ramfs_node_t* child = node->children[i];
        if (strcmp(child->name, path) == 0)
        {
            resource_result_t queue_result = vfs_cache_query_node(&vfs, directory->cache_index, child->id, out);
            if (queue_result == RESOURCE_RESULT_OK) return RESOURCE_RESULT_OK;
            resource_result_t create_result = vfs_create_node(&vfs, directory, child->id, child->name, &ramfs_node_operations, child, child->type, out);
            if (create_result != RESOURCE_RESULT_OK) return create_result;
            return RESOURCE_RESULT_OK;
        }
    }
    return RESOURCE_RESULT_NOT_FOUND;
}

resource_result_t ramfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry)
{
    ramfs_node_t* node = directory->fs_data;
    if (index >= node->children_count) return RESOURCE_RESULT_NOT_FOUND;

    ramfs_node_t* child = node->children[index];
    strcpy(entry->name, child->name);
    entry->type = child->type;

    return RESOURCE_RESULT_OK;
}

// ---

ramfs_t ramfs;
void ramfs_driver_init(multiboot_info_t* multiboot)
{
    ramfs.last_index = 0;

    multiboot_module_t* mods = (multiboot_module_t*)multiboot->mods_addr;
    multiboot_module_t* initrd = (multiboot_module_t*)physical_to_virtual(&mods[0]);
        
    void* tar_data = (void*)physical_to_virtual((void*)initrd->mod_start);
    size_t tar_size = initrd->mod_end - initrd->mod_start;

    ramfs.root_node = tar_create_ramfs_node(tar_data, tar_size);
}

ramfs_result_t ramfs_create_node(ramfs_t* ramfs, const char* name, vfs_node_type type, void* data, size_t data_size, ramfs_node_t** out)
{
    ramfs_node_t* node = (ramfs_node_t*)kernel_heap_calloc(sizeof(ramfs_node_t));
    if (!node) return RAMFS_RESULT_ERROR;

    node->id = ramfs->last_index++;
    node->children_count = 0;
    strncpy(node->name, name, VFS_NODE_NAME_LENGTH - 1);
    node->type = type;
    node->data = data;
    node->data_size = data_size;

    *out = node;

    return RAMFS_RESULT_OK;
}

resource_result_t ramfs_read(vfs_node_t* file, size_t offset, void* buffer, size_t length, size_t* read_bytes)
{
    ramfs_node_t* node = file->fs_data;

    if (offset >= node->data_size)
    {
        *read_bytes = 0;
        return RESOURCE_RESULT_OK;
    }

    size_t remaining = node->data_size - offset;
    size_t length_to_read = (length < remaining) ? length : remaining;

    memcpy(buffer, (uint8_t*)node->data + offset, length_to_read);

    *read_bytes = length_to_read;
    return RESOURCE_RESULT_OK;
}

ramfs_result_t ramfs_add_child(ramfs_node_t* parent, ramfs_node_t* child)
{
    if (parent->children_count >= RAMFS_NODE_MAX_CHILDREN) return RAMFS_RESULT_MAX_CHILDREN;
    parent->children[parent->children_count++] = child;
    return RAMFS_RESULT_OK;
}

ramfs_result_t ramfs_find_child(ramfs_node_t* parent, const char* name, ramfs_node_t** out)
{
    for (size_t i = 0; i < parent->children_count; ++i)
    {
        ramfs_node_t* child = parent->children[i];
        if (strcmp(child->name, name) == 0)
        {
            *out = child;
            return RAMFS_RESULT_OK;
        }
    }
    return RAMFS_RESULT_NOT_FOUND;
}

ramfs_result_t ramfs_get_or_create_directory(ramfs_node_t* root, char* path, ramfs_node_t** out)
{
    char buffer[VFS_NODE_NAME_LENGTH];
    strncpy(buffer, path, VFS_NODE_NAME_LENGTH - 1);
    buffer[VFS_NODE_NAME_LENGTH - 1] = '\0';

    ramfs_node_t* current = root;

    char* token = strtok(path, "/");
    while (token)
    {
        ramfs_node_t* child;
        ramfs_result_t result = ramfs_find_child(current, token, &child);

        if (result == RAMFS_RESULT_NOT_FOUND)
        {
            result = ramfs_create_node(&ramfs, token, VFS_NODE_TYPE_DIRECTORY, NULL, 0, &child);
            if (result != RAMFS_RESULT_OK) return result;

            ramfs_add_child(current, child);
        }

        current = child;
        token = strtok(NULL, "/");
    }

    *out = current;
    return RAMFS_RESULT_OK;
}

ramfs_result_t ramfs_create_path(ramfs_node_t* root, char* path, vfs_node_type type, ramfs_node_t** out)
{
    char buffer[VFS_NODE_NAME_LENGTH];
    strncpy(buffer, path, VFS_NODE_NAME_LENGTH - 1);
    buffer[VFS_NODE_NAME_LENGTH - 1] = '\0';

    char* last_slash = strrchr(path, '/');

    ramfs_result_t result;

    ramfs_node_t* parent = root;
    char* name = path;

    if (last_slash)
    {
        *last_slash = '\0';

        result = ramfs_get_or_create_directory(root, path, &parent);
        if (result != RAMFS_RESULT_OK) return result;
        name = last_slash + 1;
    }

    ramfs_node_t* node;
    result = ramfs_find_child(parent, name, &node);

    if (result == RAMFS_RESULT_NOT_FOUND)
    {
        result = ramfs_create_node(&ramfs, name, type, NULL, 0, &node);
        if (result != RAMFS_RESULT_OK) return result;
        result = ramfs_add_child(parent, node);
        if (result != RAMFS_RESULT_OK) return result; // todo: need to revert stuff above
    }

    if (out != NULL) *out = node;
    return RAMFS_RESULT_OK;
}