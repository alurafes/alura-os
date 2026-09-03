#include "syscall.h"
#include "idt.h"
#include "vfs.h"
#include "elf_executable.h"
#include "task_manager.h"

#include "print.h"

#include "drivers/keyboard.h"
#include "terminal.h"

syscall_t syscall;

int32_t syscall_open()
{
    const char* path = (const char*)SYSCALL_GET_PARAMETER(0);
    int32_t flags = (int32_t)SYSCALL_GET_PARAMETER(1);

    // gotta get that /dev sorted out soon ish
    if (strcmp(path, "/dev/keyboard") == 0)
    {
        size_t index = 0;
        resource_result_t result = keyboard_open(SYSCALL_TASK, &index);
        if (result != RESOURCE_RESULT_OK) return -(int32_t)SYSCALL_RESULT_FAIL;
        return index;
    }
    if (strcmp(path, "/dev/terminal") == 0)
    {
        size_t index = 0;
        resource_result_t result = terminal_open(SYSCALL_TASK, &index);
        if (result != RESOURCE_RESULT_OK) return -(int32_t)SYSCALL_RESULT_FAIL;
        return index;
    }

    vfs_node_t* node = NULL;
    resource_result_t result = vfs_resolve(&vfs, path, &node);

    if (result != RESOURCE_RESULT_OK)
    {
        if (result != RESOURCE_RESULT_NOT_FOUND || !(flags & SYSCALL_O_CREAT)) return -(int32_t)SYSCALL_RESULT_FAIL;

        result = vfs_create(&vfs, path, VFS_NODE_TYPE_FILE, &node);
        if (result != RESOURCE_RESULT_OK) return -(int32_t)SYSCALL_RESULT_FAIL;
    }
    else if (flags & SYSCALL_O_TRUNC)
    {
        if (node->type != VFS_NODE_TYPE_FILE) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;
        vfs_truncate(node);
    }

    if (node->type != VFS_NODE_TYPE_FILE) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;

    size_t index = 0;
    result = resource_register(SYSCALL_TASK, RESOURCE_TYPE_FILE, node, &vfs_operations, &index);
    if (result != RESOURCE_RESULT_OK) return -(int32_t)SYSCALL_RESULT_FAIL;

    return index;
}

int32_t syscall_close()
{
    uint32_t resource_index = (uint32_t)SYSCALL_GET_PARAMETER(0);

    resource_t* resource = SYSCALL_TASK->resources[resource_index];
    if (!resource) return -(int32_t)SYSCALL_RESULT_FAIL;

    resource_remove(SYSCALL_TASK, resource_index);
    
    return SYSCALL_RESULT_OK;
}

int32_t syscall_read()
{
    uint32_t resource_index = (uint32_t)SYSCALL_GET_PARAMETER(0);
    void* buffer = (void*)SYSCALL_GET_PARAMETER(1);
    size_t length = (size_t)SYSCALL_GET_PARAMETER(2);

    if (SYSCALL_TASK->task_is_user)
    {
        uintptr_t buffer_start = (uintptr_t)buffer;
        uintptr_t buffer_end = buffer_start + length;
        if (buffer_start >= KERNEL_VIRTUAL_SPACE_START) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;
        if (buffer_end < buffer_start || buffer_end > KERNEL_VIRTUAL_SPACE_START) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;
    }
    if (resource_index >= TASK_MAX_RESOURCES) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;

    resource_t* resource = SYSCALL_TASK->resources[resource_index];
    if (!resource) return -(int32_t)SYSCALL_RESULT_FAIL;

    size_t read_bytes = 0;
    if (resource->operations.read == NULL) return read_bytes;

    resource_result_t result = resource->operations.read(resource, resource->offset, buffer, length, &read_bytes);

    if (result == RESOURCE_RESULT_WILL_BLOCK)
    {
        task_manager_block_task(&task_manager, SYSCALL_TASK, TASK_WAIT_REASON_IO, resource->data);
        task_manager_yield_current(&task_manager);

        SYSCALL_TASK->syscall_retry = 1;

        return -(int32_t)SYSCALL_RESULT_BUSY;
    }

    SYSCALL_TASK->syscall_retry = 0;

    if (result != RESOURCE_RESULT_OK) return -(int32_t)SYSCALL_RESULT_FAIL;

    resource->offset += read_bytes;

    return read_bytes;
}

int32_t syscall_write()
{
    uint32_t resource_index = (uint32_t)SYSCALL_GET_PARAMETER(0);
    void* buffer = (void*)SYSCALL_GET_PARAMETER(1);
    size_t length = (size_t)SYSCALL_GET_PARAMETER(2);

    syscall_execution_state_write_t* write_state = &SYSCALL_TASK->syscall_execution_state.write;
    if (!SYSCALL_TASK->syscall_retry)
    {
        write_state->offset = 0;
    }

    if (SYSCALL_TASK->task_is_user)
    {
        uintptr_t buffer_start = (uintptr_t)buffer;
        uintptr_t buffer_end = buffer_start + length;
        if (buffer_start >= KERNEL_VIRTUAL_SPACE_START) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;
        if (buffer_end < buffer_start || buffer_end > KERNEL_VIRTUAL_SPACE_START) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;
    }
    if (resource_index >= TASK_MAX_RESOURCES) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;

    resource_t* resource = SYSCALL_TASK->resources[resource_index];
    if (!resource) return -(int32_t)SYSCALL_RESULT_FAIL;

    size_t written_bytes = 0;

    if (resource->operations.write == NULL) return -(int32_t)SYSCALL_RESULT_FAIL;

    resource_result_t result = resource->operations.write(resource, resource->offset + write_state->offset, buffer + write_state->offset, length - write_state->offset, &written_bytes);
    write_state->offset += written_bytes;

    if (result == RESOURCE_RESULT_WILL_BLOCK)
    {
        task_manager_block_task(&task_manager, SYSCALL_TASK, TASK_WAIT_REASON_IO, resource);
        task_manager_yield_current(&task_manager);

        SYSCALL_TASK->syscall_retry = 1;

        return -(int32_t)SYSCALL_RESULT_BUSY;
    }

    SYSCALL_TASK->syscall_retry = 0;

    resource->offset += write_state->offset;

    if (result != RESOURCE_RESULT_OK && write_state->offset == 0) return -(int32_t)SYSCALL_RESULT_FAIL;

    return write_state->offset;
}

int32_t syscall_fork()
{
    task_t* child_task = task_manager_task_copy(&task_manager, SYSCALL_TASK, 1);
    child_task->parent = SYSCALL_TASK;
    task_manager_add_child_to_task(SYSCALL_TASK, child_task);
    return child_task->task_id;
}

int32_t syscall_execve()
{
    const char* path = (const char*)SYSCALL_GET_PARAMETER(0);
    char* const* argv = (char* const*)SYSCALL_GET_PARAMETER(1);

    if (SYSCALL_TASK->task_is_user)
    {
        if ((uintptr_t)path >= KERNEL_VIRTUAL_SPACE_START) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;
        if (argv != NULL && (uintptr_t)argv >= KERNEL_VIRTUAL_SPACE_START) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;
    }

    if (elf_load_into_task(SYSCALL_TASK, path, argv) != ELF_RESULT_OK) return -(int32_t)SYSCALL_RESULT_FAIL;
    return SYSCALL_RESULT_OK;
}

int32_t syscall_exit()
{
    int32_t return_code = (int32_t)SYSCALL_GET_PARAMETER(0);

    task_t *parent = SYSCALL_TASK->parent;

    if (
        parent != NULL &&
        parent->task_state == TASK_STATE_BLOCKED &&
        parent->wait_reason == TASK_WAIT_REASON_CHILD &&
        (parent->wait_object == (void*)SYSCALL_TASK->task_id ||
        parent->wait_object == (void*)-1)
    )
    {
        task_manager_unblock_task(&task_manager, parent);
    }

    task_manager_exit_task(&task_manager, SYSCALL_TASK, return_code);
    task_manager_yield_current(&task_manager);

    return SYSCALL_RESULT_OK;
}

int32_t syscall_waitpid()
{
    int32_t pid = (int32_t)SYSCALL_GET_PARAMETER(0);
    int32_t* result = (int32_t*)SYSCALL_GET_PARAMETER(1);

    task_t *child = NULL;

    if (pid == -1)
    {
        child = task_manager_find_zombie_child(&task_manager, SYSCALL_TASK);
    }
    else
    {
        child = task_manager_find_child(&task_manager, SYSCALL_TASK, pid);
        if (child != NULL && child->task_state != TASK_STATE_ZOMBIE)
        {
            child = NULL;
        }
    }

    // no zombies found
    if (child == NULL)
    {
        if (pid != -1)
        {
            task_t *existing_child = task_manager_find_child(&task_manager, SYSCALL_TASK, pid);
            if (existing_child == NULL)
            {
                SYSCALL_TASK->syscall_retry = 0;
                return -(int32_t)SYSCALL_RESULT_FAIL;
            }
        }

        task_manager_block_task(&task_manager, SYSCALL_TASK, TASK_WAIT_REASON_CHILD, (void*)pid);
        task_manager_yield_current(&task_manager);
        SYSCALL_TASK->syscall_retry = 1;

        return -(int32_t)SYSCALL_RESULT_BUSY;
    }

    int32_t return_code = child->return_code;
    int32_t child_pid = child->task_id;

    task_manager_destroy_task(&task_manager, child);

    SYSCALL_TASK->syscall_retry = 0;

    if (result != NULL)
    {
        *result = return_code;
    }

    return child_pid;
}

void* syscall_sbrk_allocate_physical_memory(size_t bytes)
{
    size_t bytes_to_allocate = ALIGN_UP(bytes);
    size_t pages_to_allocate = bytes_to_allocate / PAGE_SIZE;
    if (SYSCALL_TASK->heap_break + bytes_to_allocate > USER_HEAP_VIRTUAL_CEILING) return NULL;

    page_entry_t* current_page_directory_phys = memory_paging_get_current_page_directory_physical();

    uintptr_t start_address = SYSCALL_TASK->heap_break;
    uintptr_t address = start_address;
    for (size_t i = 0; i < pages_to_allocate; ++i)
    {
        void* physical_allocated = memory_bitmap_allocate();
        if (!physical_allocated)
        {
            for (uintptr_t unwind_address = start_address; unwind_address < address; unwind_address += PAGE_SIZE)
            {
                uintptr_t unwind_physical = memory_paging_virtual_to_physical(current_page_directory_phys, unwind_address);
                memory_paging_unmap_current(unwind_address);
                if (unwind_physical) memory_bitmap_free((void*)unwind_physical);
            }
            return NULL;
        }
        memory_paging_map_current((uint32_t)physical_allocated, (uint32_t)address, PAGE_PRESENT | PAGE_USER | PAGE_READ_WRITE);
        address += PAGE_SIZE;
    }
    void* allocated_address = (void*)SYSCALL_TASK->heap_break;
    SYSCALL_TASK->heap_break += bytes_to_allocate;
    return allocated_address;
}

void syscall_sbrk_free_physical_memory(size_t bytes)
{
    size_t bytes_to_free = ALIGN_UP(bytes);
    size_t pages_to_free = bytes_to_free / PAGE_SIZE;

    page_entry_t* current_page_directory_phys = memory_paging_get_current_page_directory_physical();

    uintptr_t address = SYSCALL_TASK->heap_break;
    for (size_t i = 0; i < pages_to_free; ++i)
    {
        address -= PAGE_SIZE;

        uintptr_t physical_address = memory_paging_virtual_to_physical(current_page_directory_phys, address);
        memory_paging_unmap_current(address);
        if (physical_address) memory_bitmap_free((void*)physical_address);
    }

    SYSCALL_TASK->heap_break -= bytes_to_free;
}

int32_t syscall_sbrk()
{
    int32_t delta = SYSCALL_GET_PARAMETER(0);

    if (!SYSCALL_TASK->task_is_user) return -(int32_t)SYSCALL_RESULT_FAIL;

    uintptr_t current_heap_break = SYSCALL_TASK->heap_break;

    if (delta == 0) return (int32_t)current_heap_break;

    if (current_heap_break == 0) return -(int32_t)SYSCALL_RESULT_FAIL;

    int64_t new_break = (int64_t)current_heap_break + (int64_t)delta;

    if (new_break >= (int64_t)USER_HEAP_VIRTUAL_CEILING) return -(int32_t)SYSCALL_RESULT_OUT_OF_MEMORY;
    if (new_break < (int64_t)SYSCALL_TASK->heap_start) return -(int32_t)SYSCALL_RESULT_FAIL;
    if (new_break >= (int64_t)KERNEL_VIRTUAL_SPACE_START) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;

    if (delta > 0)
    {
        if (syscall_sbrk_allocate_physical_memory((size_t)delta) == NULL) return -(int32_t)SYSCALL_RESULT_OUT_OF_MEMORY;
    }
    else
    {
        syscall_sbrk_free_physical_memory((size_t)(-(int64_t)delta));
    }

    return (int32_t)current_heap_break;
}

int32_t syscall_isatty()
{
    uint32_t resource_index = (uint32_t)SYSCALL_GET_PARAMETER(0);
    if (resource_index >= TASK_MAX_RESOURCES) return 0;
    resource_t* resource = SYSCALL_TASK->resources[resource_index];
    if (resource == NULL) return 0;
    return resource->type == RESOURCE_TYPE_KEYBOARD || resource->type == RESOURCE_TYPE_TERMINAL;
}

int32_t syscall_getpid()
{
    return (int32_t)SYSCALL_TASK->task_id;
}

int32_t syscall_lseek()
{
    uint32_t resource_index = (uint32_t)SYSCALL_GET_PARAMETER(0);
    int32_t offset = (int32_t)SYSCALL_GET_PARAMETER(1);
    int32_t whence = (int32_t)SYSCALL_GET_PARAMETER(2);

    if (resource_index >= TASK_MAX_RESOURCES) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;

    resource_t* resource = SYSCALL_TASK->resources[resource_index];
    if (resource == NULL) return -(int32_t)SYSCALL_RESULT_FAIL;

    int64_t new_offset;
    switch (whence)
    {
        case SYSCALL_LSEEK_SET:
        {
            new_offset = (int64_t)offset;
            break;
        }
        case SYSCALL_LSEEK_CUR:
        {
            new_offset = (int64_t)resource->offset + (int64_t)offset;
            break;
        }
        case SYSCALL_LSEEK_END:
        {
            if (resource->type != RESOURCE_TYPE_FILE) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;

            size_t file_size = 0;
            if (vfs_get_size((vfs_node_t*)resource->data, &file_size) != RESOURCE_RESULT_OK) return -(int32_t)SYSCALL_RESULT_FAIL;

            new_offset = (int64_t)file_size + (int64_t)offset;
            break;
        }
        default:
        {
            return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;
        }
    }

    if (new_offset < 0) return -(int32_t)SYSCALL_RESULT_BAD_PARAMETER;

    resource->offset = (size_t)new_offset;

    return (int32_t)resource->offset;
}

void syscall_handler(register_interrupt_data_t* data)
{
    syscall.caller_task = task_manager.task_current;
    syscall.current_register_data = data;

    if (!syscall.caller_task->syscall_retry)
    {
        syscall.caller_task->syscall_execution.index = data->eax;
        syscall.caller_task->syscall_execution.parameters[0] = data->ebx;
        syscall.caller_task->syscall_execution.parameters[1] = data->ecx;
        syscall.caller_task->syscall_execution.parameters[2] = data->edx;
        syscall.caller_task->syscall_execution.parameters[3] = data->esi;
        syscall.caller_task->syscall_execution.parameters[4] = data->edi;
        syscall.caller_task->syscall_execution.parameters[5] = 0; // will be changed when i do system v abi for x86 (stack)
    }

    switch (syscall.caller_task->syscall_execution.index)
    {
        case SYSCALL_OPEN:
        {
            data->eax = syscall_open();
            break;
        }
        case SYSCALL_CLOSE:
        {
            data->eax = syscall_close();
            break;
        }
        case SYSCALL_READ:
        {
            data->eax = syscall_read();
            break;
        }
        case SYSCALL_WRITE:
        {
            data->eax = syscall_write();
            break;
        }
        case SYSCALL_FORK:
        {
            data->eax = syscall_fork();
            break;
        }
        case SYSCALL_EXECVE:
        {
            int32_t syscall_result = syscall_execve();
            if (syscall_result == SYSCALL_RESULT_OK)
            {
                data->useresp = syscall.caller_task->task_esp;
                data->eip = syscall.caller_task->task_init_eip;
            }
            else
            {
                data->eax = syscall_result;
            }
            break;
        }
        case SYSCALL_EXIT:
        {
            syscall_exit();
            break;
        }
        case SYSCALL_WAITPID:
        {
            data->eax = syscall_waitpid();
            break;
        }
        case SYSCALL_SBRK:
        {
            data->eax = syscall_sbrk();
            break;
        }
        case SYSCALL_GETPID:
        {
            data->eax = syscall_getpid();
            break;
        }
        case SYSCALL_ISATTY:
        {
            data->eax = syscall_isatty();
            break;
        }
        case SYSCALL_LSEEK:
        {
            data->eax = syscall_lseek();
            break;
        }
    }
}