#include "syscall.h"
#include "idt.h"
#include "vfs.h"

#include "print.h"

int32_t syscall_open(task_t* task, const char* path)
{
    vfs_node_t* node = NULL;
    resource_result_t result = vfs_resolve(&vfs, path, &node);
    if (result != RESOURCE_RESULT_OK) return -(int32_t)result;
    
    if (node->type != VFS_NODE_TYPE_FILE) return -(int32_t)RESOURCE_RESULT_INVALID;
    
    size_t index = 0;
    result = resource_register(task, RESOURCE_TYPE_FILE, node, &vfs_operations, &index);
    if (result != RESOURCE_RESULT_OK) return -(int32_t)result;

    return index;
}

int32_t syscall_close(task_t* task, uint32_t resource_index)
{
    resource_t* resource = task->resources[resource_index];
    if (!resource) return -(int32_t)RESOURCE_RESULT_INVALID;

    resource->operations.close(resource);
    resource_remove(task, resource_index);
    
    return 0;
}

void syscall_handler(register_interrupt_data_t* data)
{
    task_t* task = task_manager.task_current;
    switch (data->eax)
    {
        case SYSCALL_OPEN:
        {
            data->eax = syscall_open(task, (const char*)data->ebx);
            break;
        }
        case SYSCALL_CLOSE:
        {
            data->eax = syscall_close(task, data->ebx);
            break;
        }
    }
}