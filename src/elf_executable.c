#include "elf_executable.h"

#include "print.h"

elf_result_t elf_load_and_execute(const char *path)
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

    // todo: user or not user; kill task on failure, bla bla
    task_t* task = task_manager_task_create(&task_manager, (void*)header.e_entry, 1, 1);

    for (size_t i = 0; i < header.e_phnum; ++i)
    {
        Elf32_Phdr program_header;

        result = vfs_read(&resource, header.e_phoff + i * sizeof(program_header), (void*)&program_header, sizeof(program_header), &read_bytes);
        if (result != RESOURCE_RESULT_OK) 
        {
            vfs_release_node(elf_node);
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
                return ELF_RESULT_ALLOCATION_ERROR;
            }
            page_entry_t* task_page_directory = bounce_alloc(task->task_cr3);
            memory_paging_result_t paging_result = memory_paging_map(task_page_directory, (uint32_t)memory, address, PAGE_PRESENT | (task->task_is_user ? PAGE_USER : 0) | PAGE_READ_WRITE);
            bounce_free((uintptr_t)task_page_directory);
            if (paging_result != MEMORY_PAGING_RESULT_OK)
            {
                memory_bitmap_free(memory);
                vfs_release_node(elf_node);
                return ELF_RESULT_ALLOCATION_ERROR;
            }
        }

        memory_paging_set((page_entry_t*)task->task_cr3);

        result = vfs_read(&resource, program_header.p_offset, (void*)program_header.p_vaddr, program_header.p_memsz, &read_bytes);
        if (result != RESOURCE_RESULT_OK) 
        {
            // todo: unmap and free memory!!
            vfs_release_node(elf_node);
            return ELF_RESULT_READ_ERROR;
        }

        memset((void*)(program_header.p_vaddr + program_header.p_filesz), 0, program_header.p_memsz - program_header.p_filesz);

        memory_paging_set(kernel_page_directory_phys);
    }

    return ELF_RESULT_OK;
}