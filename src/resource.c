#include "resourse.h"

#include "print.h"

resource_result_t resource_register(task_t* task, resource_type_t type, void* data, resource_operations_t* operations, size_t* result)
{
    if (!task || !operations || !result) return RESOURCE_RESULT_BAD_PARAMETER;
    int32_t free_index = -1;
    for (size_t i = 0; i < TASK_MAX_RESOURCES; ++i)
    {
        if (!task->resources[i]) {
            free_index = i;
            break;
        }
    }
    if (free_index == -1) return RESOURCE_RESULT_NO_FREE_SLOTS;


    resource_t* resource = kernel_heap_calloc(sizeof(resource_t));
    resource->type = type;
    resource->data = data;
    resource->operations = *operations;

    task->resources[free_index] = resource;
    *result = free_index;
    return RESOURCE_RESULT_OK;
}

resource_result_t resource_remove(task_t* task, size_t index)
{
    if (!task || index >= TASK_MAX_RESOURCES) return RESOURCE_RESULT_BAD_PARAMETER;
    resource_t* resource = task->resources[index];
    if (!resource) return RESOURCE_RESULT_OK;

    kernel_heap_free(resource);
    // todo: should this own the resource itself? a problem for future me

    task->resources[index] = NULL;

    return RESOURCE_RESULT_OK;
}