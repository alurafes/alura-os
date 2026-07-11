#include "elf_executable.h"

#include "print.h"

elf_result_t elf_load_into_task(task_t* task, const char* path)
{
    if (!path) return ELF_RESULT_BAD_PARAMETER;

    vfs_node_t* elf_node;
    resource_result_t result = vfs_resolve(&vfs, path, &elf_node);
    if (result != RESOURCE_RESULT_OK) return ELF_RESULT_INVALID_ELF;

    if (elf_node->type != VFS_NODE_TYPE_FILE) 
    {
        vfs_release_node(elf_node);
        return ELF_RESULT_INVALID_ELF;
    }
    
    Elf32_Ehdr header;

    // todo: kinda hacky
    resource_t resource = {
        .data = elf_node,
        .operations = vfs_operations,
        .type = RESOURCE_TYPE_FILE
    };

    size_t read_bytes = 0;
    result = vfs_read(&resource, 0, (void*)&header, sizeof(header), &read_bytes);

    if (result != RESOURCE_RESULT_OK) 
    {
        vfs_release_node(elf_node);
        return ELF_RESULT_READ_ERROR;
    }

    if (header.e_ident[EI_MAG0] != 0x7F 
        || header.e_ident[EI_MAG1] != 'E' 
        || header.e_ident[EI_MAG2] != 'L' 
        || header.e_ident[EI_MAG3] != 'F')
    {
        vfs_release_node(elf_node);
        return ELF_RESULT_INVALID_ELF;
    }

    page_entry_t* new_task_page_directory_phys = NULL;
    if (memory_paging_create_page_directory((uint32_t*)&new_task_page_directory_phys) != MEMORY_PAGING_RESULT_OK)
    {
        return ELF_RESULT_ALLOCATION_ERROR;
    }

    page_entry_t* current_page_directory_phys = memory_paging_get_current_page_directory_physical();
    page_entry_t* new_task_page_directory = (page_entry_t*)bounce_alloc((uintptr_t)new_task_page_directory_phys);
    uint32_t new_task_esp = 0;
    task_manager_prepare_new_stack(new_task_page_directory, task->task_is_user, header.e_entry, &new_task_esp);

    for (size_t i = 0; i < header.e_phnum; ++i)
    {
        Elf32_Phdr program_header;

        result = vfs_read(&resource, header.e_phoff + i * sizeof(program_header), (void*)&program_header, sizeof(program_header), &read_bytes);
        if (result != RESOURCE_RESULT_OK) 
        {
            vfs_release_node(elf_node);
            bounce_free((uintptr_t)new_task_page_directory);
            return ELF_RESULT_READ_ERROR;
        }

        if (program_header.p_type != PT_LOAD) continue;

        uint32_t segment_start = ALIGN_DOWN(program_header.p_vaddr);
        uint32_t segment_end = ALIGN_UP(program_header.p_vaddr + program_header.p_memsz);

        for (uint32_t address = segment_start; address < segment_end; address += PAGE_SIZE)
        {
            void* memory = memory_bitmap_allocate();
            if (!memory)
            {
                vfs_release_node(elf_node);
                bounce_free((uintptr_t)new_task_page_directory);
                return ELF_RESULT_ALLOCATION_ERROR;
            }
            memory_paging_result_t paging_result = memory_paging_map(new_task_page_directory, (uint32_t)memory, address, PAGE_PRESENT | (task->task_is_user ? PAGE_USER : 0) | PAGE_READ_WRITE);
            if (paging_result != MEMORY_PAGING_RESULT_OK)
            {
                memory_bitmap_free(memory);
                vfs_release_node(elf_node);
                bounce_free((uintptr_t)new_task_page_directory);
                return ELF_RESULT_ALLOCATION_ERROR;
            }
        }

        uint32_t bytes_to_copy = program_header.p_filesz;
        uint32_t file_offset = program_header.p_offset;
        uint32_t virt = program_header.p_vaddr;

        while (bytes_to_copy > 0)
        {
            uint32_t page_off = virt & (PAGE_SIZE - 1);
            uint32_t chunk = PAGE_SIZE - page_off;
            if (chunk > bytes_to_copy) chunk = bytes_to_copy;

            uintptr_t phys = memory_paging_virtual_to_physical(new_task_page_directory_phys, virt);
            void* kaddr = (void*)bounce_alloc(phys);
            result = vfs_read(&resource, file_offset, (void*)((uintptr_t)kaddr + page_off), chunk, &read_bytes);
            if (result != RESOURCE_RESULT_OK)
            {
                bounce_free((uintptr_t)kaddr);
                bounce_free((uintptr_t)new_task_page_directory);
                vfs_release_node(elf_node);
                return ELF_RESULT_READ_ERROR;
            }
            bounce_free((uintptr_t)kaddr);
            virt += chunk;
            file_offset += chunk;
            bytes_to_copy -= chunk;
        }

        if (program_header.p_memsz > program_header.p_filesz)
        {
            uint32_t zero_start = program_header.p_vaddr + program_header.p_filesz;
            uint32_t zero_len = program_header.p_memsz - program_header.p_filesz;

            while (zero_len > 0)
            {
                uint32_t page_off = zero_start & (PAGE_SIZE - 1);
                uint32_t chunk = PAGE_SIZE - page_off;
                if (chunk > zero_len) chunk = zero_len;

                uintptr_t phys = memory_paging_virtual_to_physical(new_task_page_directory_phys, zero_start);
                void* kaddr = (void*)bounce_alloc(phys);
                if (!kaddr)
                {
                    bounce_free((uintptr_t)new_task_page_directory);
                    vfs_release_node(elf_node);
                    return ELF_RESULT_ALLOCATION_ERROR;
                }

                memset((void*)((uintptr_t)kaddr + page_off), 0, chunk);

                bounce_free((uintptr_t)kaddr);

                zero_start += chunk;
                zero_len -= chunk;
            }
        }
    }

    task->task_init_eip = header.e_entry;
    task->task_cr3 = (uint32_t)new_task_page_directory_phys;
    task->task_esp = new_task_esp;

    memory_paging_queue_to_destroy(current_page_directory_phys);

    // page_entry_t* current_page_directory = bounce_alloc();

    // if (current_page_directory_phys != kernel_page_directory_phys) memory_paging_free_page_directory(current_page_directory);
    // bounce_free((uintptr_t)current_page_directory);

    return ELF_RESULT_OK;
}

elf_result_t elf_load_and_execute(const char *path)
{
    task_t* task = task_manager_task_create(&task_manager, NULL, 1, 1);
    return elf_load_into_task(task, path);
}