#ifndef ALURA_RESOURCE_H
#define ALURA_RESOURCE_H

#include <stddef.h>

#include "kernel_heap.h"
#include "task_manager.h"

typedef enum resource_result_t {
    RESOURCE_RESULT_OK = 0,
    RESOURCE_RESULT_INVALID,
    RESOURCE_RESULT_NOT_FOUND,
    RESOURCE_RESULT_BAD_PARAMETER,
    RESOURCE_RESULT_ALREADY_PRESENT,
    RESOURCE_RESULT_STILL_IN_USE,
    RESOURCE_RESULT_ALLOCATION_ERROR,
    RESOURCE_RESULT_NO_FREE_SLOTS,
} resource_result_t;

typedef enum resource_type_t {
    RESOURCE_TYPE_FILE = 0
} resource_type_t;

typedef struct resource_t resource_t;

typedef struct resource_operations_t {
    resource_result_t (*close)(resource_t* resource);
    resource_result_t (*read)(resource_t* resource, size_t offset, void* buffer, size_t length, size_t* read_bytes);
} resource_operations_t;

typedef struct resource_t {
    resource_type_t type;
    void* data;
    resource_operations_t operations;
} resource_t;

resource_result_t resource_register(task_t* task, resource_type_t type, void* data, resource_operations_t* operations, size_t* result);
resource_result_t resource_remove(task_t* task, size_t index);

#endif // ALURA_RESOURCE_H