#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 

#include "bin/structs.h"
#include "bin/tokenizer.h"
#include "bin/parser.h"
#include "bin/lekser.h"





static void getFileChars(getFileChars_ti *ret, struct operation_mode_s *cmd){
    
    FILE *f = fopen(cmd->startFile, "r"); 
    if (f == NULL) {
        ret->why = 'f';
        ret->status = ERR;
        return;
    }
    
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    
    ret->data = malloc(file_size + 1);
    ret->size = file_size + 1;
    cmd->useDinamiceRam += file_size + 1;

    if (!ret->data) {
        ret->why = 'm'; 
        ret->status = ERR;
        fclose(f);
        return;
    }
    
    size_t bytes_read = fread(ret->data, 1, file_size, f);
    ret->data[bytes_read] = '\0';
    
    fclose(f);
    ret->status = OK;

}


void write_machine_code_to_file(struct machine_code_s *mcs, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        printf("Ошибка создания файла %s\n", filename);
        return;
    }

    //  ELF 64-bit
    unsigned char elf_header[64] = {
        // ELF magic
        0x7f, 'E', 'L', 'F', 
        // 64-bit, little endian, version 1
        0x02, 0x01, 0x01, 0x00,
        // Padding
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // ET_EXEC, EM_X86_64
        0x02, 0x00, 0x3e, 0x00,
        // EV_CURRENT
        0x01, 0x00, 0x00, 0x00,
        // Entry point: 0x400000 + 0x78 = 0x400078
        0x78, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Program header offset: 0x40
        0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Section header offset: 0 (none)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Flags: 0
        0x00, 0x00, 0x00, 0x00,
        // ELF header size: 64
        0x40, 0x00,
        // Program header size: 56
        0x38, 0x00,
        // Number of program headers: 1
        0x01, 0x00,
        // Section header size: 0, number: 0, index: 0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    // Program header
    unsigned char program_header[56] = {
        // PT_LOAD
        0x01, 0x00, 0x00, 0x00,
        // Flags: RWE
        0x07, 0x00, 0x00, 0x00,
        // Offset: 0
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Virtual address: 0x400000
        0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Physical address: 0x400000
        0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,
        // File size: header + code + exit
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Memory size: same as file size
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Alignment: 0x1000
        0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };


    // unsigned char exit_code[] = {
    //     0x48, 0xc7, 0xc0, 0x3c, 0x00, 0x00, 0x00, // mov rax, 60
    //     0x48, 0x31, 0xff,                         // xor rdi, rdi
    //     0x0f, 0x05                                // syscall
    // };

 
    uint64_t code_size = mcs->size ;//+ sizeof(exit_code);
    uint64_t file_size = sizeof(elf_header) + sizeof(program_header) + code_size;


    memcpy(&program_header[32], &code_size, 8);  // p_filesz
    memcpy(&program_header[40], &code_size, 8);  // p_memsz

    fwrite(elf_header, 1, sizeof(elf_header), f);
    fwrite(program_header, 1, sizeof(program_header), f);


    unsigned char padding[120 - sizeof(elf_header) - sizeof(program_header)];
    memset(padding, 0, sizeof(padding));
    fwrite(padding, 1, sizeof(padding), f);


    fwrite(mcs->code, 1, mcs->size, f);
    // fwrite(exit_code, 1, sizeof(exit_code), f);

    fclose(f);
    chmod(filename, 0755);

    printf("ELF: created By: %s (%ld байт)\n", filename, file_size);
}
int main(int argc, char *argv[]) {  
    struct operation_mode_s smod;

    if (argc < 2) {
        printf("Use: %s <filename>\n", argv[0]);
        return 1;
    }
    else if(argc == 2){
        smod.cmp.maxReadBuffer = 4096;
        smod.startFile = argv[1];
        smod.maxRamUse = 1000000;

        getFileChars_ti rch;
        struct tokens_s  *getToken=NULL;
        struct pars_s *parsed=NULL;
        struct machine_code_s*machine_code=NULL;

        getFileChars(&rch, &smod);
        tokenizer(&rch, &smod, &getToken);
        parsing(&getToken, &smod, &parsed);
        leksering(&parsed, &smod, &machine_code);
        
        write_machine_code_to_file(machine_code, "b.out");

        
        
    }
    
    return OK;
}
