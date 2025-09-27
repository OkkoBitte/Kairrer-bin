#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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
    FILE *f = fopen(filename, "wb");  // "wb" - binary write
    if (f == NULL) {
        printf("Ошибка создания файла %s\n", filename);
        return;
    }
    
    size_t written = fwrite(mcs->code, 1, mcs->size, f);
    fclose(f);
    
    if (written == mcs->size) {
        printf("Машинный код записан в %s (%d байт)\n", filename, mcs->size);
    } else {
        printf("Ошибка записи в файл %s\n", filename);
    }
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
        
        //write_machine_code_to_file(machine_code, "b.out");

        
        
    }
    
    return OK;
}
