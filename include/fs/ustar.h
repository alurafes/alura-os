#ifndef ALURA_FS_USTAR
#define ALURA_FS_USTAR

#include <stddef.h>
#include <stdint.h>

#include "fs/ramfs.h"

#define TAR_BLOCK_SIZE 512

typedef struct tar_block_t {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char last_modified[12];
    char checksum[8];
    char type;
    char linked_name[100];
    char ustar[6];
    char ustar_version[2];
    char owner_name[32];
    char owner_group[32];
    char device_major[8];
    char device_minor[8];
    char filename_prefix[155];
    char padding[12];
} tar_block_t;

size_t tar_get_size_from_octal(const char* text, size_t length);
int tar_is_zero_block(const uint8_t* block);
void tar_build_path(char* dst, size_t dst_size, const char* prefix, const char* name);
ramfs_node_t* tar_create_ramfs_node(const void* tar_data, size_t tar_size);

#endif //ALURA_FS_USTAR