#include "fs/ustar.h"
#include "print.h"

size_t tar_get_size_from_octal(const char* text, size_t length)
{
    size_t value = 0;
    for (size_t i = 0; i < length; ++i)
    {
        if (text[i] < '0' || text[i] > '7') break;
        value = (value << 3) + (text[i] - '0');
    }
    return value;
}

int tar_is_zero_block(const uint8_t* block)
{
    for (size_t i = 0; i < TAR_BLOCK_SIZE; i++)
    {
        if (block[i] != 0) return 0;
    }

    return 1;
}

ramfs_node_t* tar_create_ramfs_node(const void* tar_data, size_t tar_size)
{
    const uint8_t* ptr = tar_data;
    const uint8_t* end = ptr + tar_size;

    ramfs_node_t* root;
    ramfs_result_t result = ramfs_create_node(&ramfs, "/", VFS_NODE_TYPE_DIRECTORY, NULL, 9, &root);
    if (result != RAMFS_RESULT_OK) return NULL;

    while (ptr + TAR_BLOCK_SIZE <= end)
    {
        tar_block_t* header = (tar_block_t*)ptr;
        if (tar_is_zero_block(ptr)) break;

        char full_name[VFS_NODE_NAME_LENGTH];
        size_t file_size = tar_get_size_from_octal(header->size, sizeof(header->size));
        size_t blocks = (file_size + TAR_BLOCK_SIZE - 1) / TAR_BLOCK_SIZE;
        vfs_node_type type = (header->type == '5') ? VFS_NODE_TYPE_DIRECTORY : VFS_NODE_TYPE_FILE;

        if (header->filename_prefix[0]) tar_build_path(full_name, sizeof(full_name), header->filename_prefix, header->name);
        else
        {
            strncpy(full_name, header->name, sizeof(full_name) - 1);
            full_name[sizeof(full_name) - 1] = '\0';
        }

        char* path = full_name;
        if (path[0] == '.') path += 1;
        if (path[0] == '\0' || strcmp(path, "/") == 0)
        {
            ptr += TAR_BLOCK_SIZE;
            ptr += blocks * TAR_BLOCK_SIZE;
            continue;
        }

        if (type == VFS_NODE_TYPE_DIRECTORY)
        {
            size_t len = strlen(full_name);

            if (len && full_name[len - 1] == '/') full_name[len - 1] = '\0';
            ramfs_create_path(root, path, VFS_NODE_TYPE_DIRECTORY, NULL);
        }
        else
        {
            ramfs_node_t* node;
            ramfs_create_path(root, path, VFS_NODE_TYPE_FILE, &node);
            node->data_size = file_size;

            if (file_size)
            {
                node->data = kernel_heap_calloc(file_size);
                memcpy(node->data, ptr + TAR_BLOCK_SIZE, file_size);
            }
        }

        ptr += TAR_BLOCK_SIZE;
        ptr += blocks * TAR_BLOCK_SIZE;
    }

    return root;
}

void tar_build_path(char* dst, size_t dst_size, const char* prefix, const char* name)
{
    size_t pos = 0;

    while (*prefix && pos + 1 < dst_size) dst[pos++] = *prefix++;
    if (pos && pos + 1 < dst_size) dst[pos++] = '/';
    while (*name && pos + 1 < dst_size) dst[pos++] = *name++;

    dst[pos] = '\0';
}