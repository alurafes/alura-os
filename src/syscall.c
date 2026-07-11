#include "syscall.h"
#include "idt.h"
#include "vfs.h"
#include "elf_executable.h"

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

int32_t syscall_read(task_t* task, uint32_t resource_index, void* buffer, size_t length)
{
    if (task->task_is_user && (uintptr_t)buffer >= KERNEL_VIRTUAL_SPACE_START) return -(int32_t)RESOURCE_RESULT_BAD_PARAMETER;
    if (resource_index >= TASK_MAX_RESOURCES) return -(int32_t)RESOURCE_RESULT_BAD_PARAMETER;

    resource_t* resource = task->resources[resource_index];
    if (!resource) return -(int32_t)RESOURCE_RESULT_INVALID;

    size_t read_bytes = 0;
    // todo: passing 0 as offset. Gotta switch to system v abi soon (horrible stack parameters stuff)
    resource_result_t result = resource->operations.read(resource, 0, buffer, length, &read_bytes);
    if (result != RESOURCE_RESULT_OK) return -(int32_t)result;

    return read_bytes;
}

int32_t syscall_fork(register_interrupt_data_t* data, task_t* task)
{
    task_t* child_task = task_manager_task_copy(&task_manager, task, 1);

    printf("Created new task: %d\n", child_task->task_id);

    return child_task->task_id;
}

int32_t syscall_execve(task_t* task, const char* path)
{
    elf_load_into_task(task, path);
    return 0;
}

int32_t syscall_print(task_t* task, const char* message)
{
    printf("<task %d>: %s", task->task_id, message);
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
        case SYSCALL_READ:
        {
            data->eax = syscall_read(task, data->ebx, (void*)data->ecx, data->edx);
            break;
        }
        case SYSCALL_FORK:
        {
            data->eax = syscall_fork(data, task);
            break;
        }
        case SYSCALL_EXECVE:
        {
            syscall_execve(task, (const char*)data->ebx);
            data->useresp = task->task_esp;
            data->eip = task->task_init_eip;
            break;
        }
        case 10:
        {
            data->eax = syscall_print(task, (const char*)data->ebx);
            break;
        }
    }
}