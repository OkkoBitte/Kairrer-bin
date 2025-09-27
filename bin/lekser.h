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
void handleLine(struct leks_context_s* context, struct line_s * lines){
    // any use to 64bitCPU
    printf("LINE: ");
    for (int i = 0; i < lines->num; i++){
        printf("%s",lines->elements[i].value);
    }
    printf("\n");
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