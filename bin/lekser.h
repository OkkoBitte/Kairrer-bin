struct leks_context_s{
    struct pars_s * pars;
    struct operation_mode_s* cmd;
    struct machine_code_s* out_machine_code;
    int capacity;
    int code_size;
    uint8_t * code_bufer;
    int ident;
};


void addData(struct leks_context_s* context, uint8_t data){
    if (context->code_size >= context->capacity){
        context->capacity = context->capacity + (context->capacity * 0.2);
        context->code_bufer = realloc(context->code_bufer, context->capacity);
    }

    context->code_bufer[context->code_size] = data;
    context->code_size++;
}


void addMultipleData(struct leks_context_s* context, uint8_t* data, int size){
    for(int i = 0; i < size; i++) {
        addData(context, data[i]);
    }
}




void add16BitData(struct leks_context_s* context, uint16_t data){
    addData(context, (uint8_t)(data & 0xFF));        
    addData(context, (uint8_t)((data >> 8) & 0xFF)); 
}
void add32BitData(struct leks_context_s* context, uint32_t data) {
    addData(context, (uint8_t)(data & 0xFF));
    addData(context, (uint8_t)((data >> 8) & 0xFF));
    addData(context, (uint8_t)((data >> 16) & 0xFF));
    addData(context, (uint8_t)((data >> 24) & 0xFF));
}


const char* registers[] = {
    "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};


int getRegisterNumber(char* reg_name) {
    if (reg_name[0] != 'r') return -1;
    return atoi(&reg_name[1]) - 1;
}

uint8_t getRexPrefix(int dest_reg, int src_reg) {
    uint8_t rex = 0x48;
    if (dest_reg >= 8) rex |= 0x01; 
    if (src_reg >= 8) rex |= 0x04;  
    return rex;
}

uint8_t getModRM(int src_reg, int dest_reg) {
    // MOD=11 (регистр-регистр), REG=src_reg, R/M=dest_reg
    return 0xC0 + ((src_reg & 7) << 3) + (dest_reg & 7);
}



// out 1 line from to END operator
struct line_s getComLine(struct leks_context_s* context) {
    struct line_s retLines;
    retLines.num = 0;  
    
    
    int max_elements = context->pars->num - context->ident;
    retLines.elements = malloc(max_elements * sizeof(struct element_s));
    
    while (context->ident < context->pars->num) {
        struct element_s current = context->pars->elements[context->ident];
        
  
   
        retLines.elements[retLines.num] = current;
        retLines.num++;
        context->ident++;
        if (current.type == OPERATOR && current.otype == O_END) { break; }
    
        
    }
    
    return retLines;
}

// use 1 line
// void handleLine(struct leks_context_s* context, struct line_s * lines){
//     // any use to 64bitCPU
//     printf("LINE: ");
//     for (int i = 0; i < lines->num; i++){
//         printf("%s",lines->elements[i].value);
//     }
//     printf("\n");
// }


void handleLine(struct leks_context_s* context, struct line_s* line) {
    printf("Processing line with %d elements: ", line->num);
    for (int i = 0; i < line->num; i++) {
        printf("%s ", line->elements[i].value);
    }
    printf("\n");
    

    if (line->num == 2) {
        if (line->elements[0].type == WORD && 
            line->elements[1].type == OPERATOR && 
            line->elements[1].otype == O_END) {
            
            if (strcmp(line->elements[0].value, "s") == 0) {
                printf("Generating SYSCALL (with end)\n");
                add16BitData(context, 0x050F); // SYSCALL
            } 
            else if (strcmp(line->elements[0].value, "r") == 0) {
                printf("Generating RET (with end)\n");
                addData(context, 0xC3); // RET
            }
        }
        return;
    }
    
    
    if (line->num < 4) return;
    

    struct element_s dest_elem = line->elements[0];
    struct element_s op_elem = line->elements[1];
    struct element_s src_elem = line->elements[2];
    

    if (dest_elem.type != WORD || dest_elem.value[0] != 'r') {
        printf("First element is not a register: %s\n", dest_elem.value);
        return;
    }
    
    int dest_reg = getRegisterNumber(dest_elem.value);
    if (dest_reg < 0 || dest_reg > 15) {
        printf("Invalid register number: %s\n", dest_elem.value);
        return;
    }
    
    printf("Processing register operation: %s %s %s\n", 
           dest_elem.value, op_elem.value, src_elem.value);
    
    if (op_elem.otype == O_TO) { 
        if (src_elem.type == VALUE && src_elem.vtype == VNUMBER) {
         
            int value = atoi(src_elem.value);
            printf("MOV r%d, %d\n", dest_reg + 1, value);
            
            //  MOV r64, imm32
            uint8_t rex = getRexPrefix(dest_reg, 0);
            addData(context, rex);
            addData(context, 0xB8 + (dest_reg & 7));
            add32BitData(context, value);
            
        } 
        else if (src_elem.type == WORD && src_elem.value[0] == 'r') {
            // MOV (rX < rY)
            int src_reg = getRegisterNumber(src_elem.value);
            if (src_reg < 0 || src_reg > 15) return;
            
            printf("MOV r%d, r%d\n", dest_reg + 1, src_reg + 1);
            
            // MOV r64, r64
            uint8_t rex = getRexPrefix(dest_reg, src_reg);
            addData(context, rex);
            addData(context, 0x89);
            addData(context, getModRM(src_reg, dest_reg));
        }
    }
    else if (op_elem.otype == O_ADD) { // + 
        if (src_elem.type == WORD && src_elem.value[0] == 'r') {
            // ADD (rX + rY)
            int src_reg = getRegisterNumber(src_elem.value);
            if (src_reg < 0 || src_reg > 15) return;
            
            printf("ADD r%d, r%d\n", dest_reg + 1, src_reg + 1);
            
            // ADD r64, r64
            uint8_t rex = getRexPrefix(dest_reg, src_reg);
            addData(context, rex);
            addData(context, 0x01);
            addData(context, getModRM(src_reg, dest_reg));
        }
    }
    else if (op_elem.otype == O_REMOVE) { // - 
        if (src_elem.type == WORD && src_elem.value[0] == 'r') {
            // SUB (rX - rY)
            int src_reg = getRegisterNumber(src_elem.value);
            if (src_reg < 0 || src_reg > 15) return;
            
            printf("SUB r%d, r%d\n", dest_reg + 1, src_reg + 1);
            
            // SUB r64, r64
            uint8_t rex = getRexPrefix(dest_reg, src_reg);
            addData(context, rex);
            addData(context, 0x29);
            addData(context, getModRM(src_reg, dest_reg));
        }
    }
}


void leksering(struct pars_s** pars, struct operation_mode_s *cmd, struct machine_code_s **out_machine_code) {

    *out_machine_code = malloc(sizeof(struct machine_code_s));


    
    struct leks_context_s context;
    context.cmd = cmd;
    context.pars = *pars;
    context.out_machine_code = *out_machine_code;
    context.capacity = cmd->cmp.maxReadBuffer;
    context.code_bufer = malloc(context.capacity);
    context.code_size = 0;
    context.ident = 0;

    struct line_s* lines = malloc(context.pars->num*sizeof(struct line_s));
    int wremLineCount = 0;
    while (context.ident < context.pars->num) {
        lines[wremLineCount] = getComLine(&context);
        wremLineCount++;
        
       
    }
    context.ident = 0;
  
    for (context.ident; context.ident < wremLineCount; context.ident++) {
        struct line_s currentLine = lines[context.ident];
        
        handleLine(&context, &currentLine);

    }

    (*out_machine_code)->code = realloc(context.code_bufer, context.code_size);
    (*out_machine_code)->size = context.code_size;
    

    if (*pars != NULL) {
        for (int i = 0; i < context.pars->num; i++) {
            free(context.pars->elements[i].value);
        }
        free(context.pars->elements);
        free(*pars);
        *pars = NULL;
    }
    
    cmd->useDinamiceRam += context.code_size;
}