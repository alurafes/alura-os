#include "fs/devfs.h"

#include "drivers/keyboard.h"
#include "terminal.h"
#include "libc/string.h"

typedef struct devfs_entry_t {
    const char* name;
    vfs_node_operations_t operations;
    void* fs_data;
    resource_type_t resource_type;
} devfs_entry_t;

// pretty crappy so far, but way better than hardcoding in read syscall
static devfs_entry_t devfs_entries[] = {
    { "keyboard", { .read = keyboard_read }, &keyboard, RESOURCE_TYPE_KEYBOARD },
    { "terminal", { .write = terminal_write }, &terminal, RESOURCE_TYPE_TERMINAL },
};

#define DEVFS_ENTRY_COUNT (sizeof(devfs_entries) / sizeof(devfs_entries[0]))

vfs_node_operations_t devfs_node_operations = {
    .lookup = devfs_lookup,
    .readdir = devfs_readdir
};

resource_result_t devfs_lookup(vfs_node_t* directory, const char* name, vfs_node_t** result)
{
    for (size_t i = 0; i < DEVFS_ENTRY_COUNT; ++i)
    {
        if (strcmp(devfs_entries[i].name, name) != 0) continue;

        resource_result_t cached_result = vfs_cache_query_node(&vfs, directory->cache_index, (int64_t)i, result);
        if (cached_result == RESOURCE_RESULT_OK) return RESOURCE_RESULT_OK;

        return vfs_create_node(&vfs, directory, (int64_t)i, devfs_entries[i].name, &devfs_entries[i].operations, devfs_entries[i].fs_data, VFS_NODE_TYPE_FILE, devfs_entries[i].resource_type, result);
    }

    return RESOURCE_RESULT_NOT_FOUND;
}

resource_result_t devfs_readdir(vfs_node_t* directory, size_t index, vfs_dir_t* entry)
{
    (void)directory;
    if (index >= DEVFS_ENTRY_COUNT) return RESOURCE_RESULT_NOT_FOUND;

    strcpy(entry->name, devfs_entries[index].name);
    entry->type = VFS_NODE_TYPE_FILE;

    return RESOURCE_RESULT_OK;
}

void devfs_module_init()
{
    vfs_node_t* dev_directory = NULL;
    resource_result_t result = vfs_resolve(&vfs, "/dev", &dev_directory);
    if (result != RESOURCE_RESULT_OK)
    {
        result = vfs_create(&vfs, "/dev", VFS_NODE_TYPE_DIRECTORY, &dev_directory);
        if (result != RESOURCE_RESULT_OK) return;
    }

    vfs_node_t* devfs_root = NULL;
    result = vfs_create_node(&vfs, NULL, -1, "dev", &devfs_node_operations, NULL, VFS_NODE_TYPE_DIRECTORY, RESOURCE_TYPE_FILE, &devfs_root);
    if (result != RESOURCE_RESULT_OK) return;

    vfs_lock_node(devfs_root);
    dev_directory->mount = devfs_root;
}
