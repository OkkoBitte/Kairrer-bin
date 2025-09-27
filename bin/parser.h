static char* getStrNum(int *pos, struct tokens_s *tokens){
    int start = *pos;
    int length = 0;

    while (tokens[*pos].type == STRING || tokens[*pos].type== NUMBER) {
        length++;
        (*pos)++;
    }

    char* result = malloc(length + 1);
    for (int i = 0; i < length; i++) {
        result[i] = tokens[start + i].value;
    }
    result[length] = '\0';

    return result;
}

static void skipTab(int *pos, struct tokens_s *tokens){
    
    while(tokens[*pos].type == TAB){
        (*pos)++;
    }
}

static char* getNumber(int *pos, struct tokens_s *tokens){
    int lenght = 0;
    int start = *pos;
    while(tokens[*pos].type == NUMBER){
        (*pos)++;
        lenght++;
    }
    char* ret = malloc(lenght + 1);
    for(int i=0;i<lenght;i++){
        ret[i] = tokens[start + i].value;
    }
    ret[lenght] = '\0';
    return ret;

}
static struct element_s getOperator(int *pos, struct tokens_s *tokens){
    struct element_s ret;
    enum TNAME type = tokens[*pos].type;
    
    ret.value = malloc(2); 
    ret.value[0] = tokens[*pos].value;
    ret.value[1] = '\0';
    
    if(type == TO){
        ret.type = OPERATOR;
        ret.otype = O_TO;
    }
    else if(type == FROM){
        ret.type = OPERATOR;
        ret.otype = O_FROM;
    }
    else if(type == ADD){
        ret.type = OPERATOR;
        ret.otype = O_ADD;
    }
    else if(type == REMOVE){
        ret.type = OPERATOR;
        ret.otype = O_REMOVE;
    }
    else if(type == ENDL){
        ret.type = OPERATOR;
        ret.otype = O_END;
    }
    (*pos)++;
    return ret;
}
char* getStrFromEType(enum ETNAME type){
    switch(type){
        case WORD:
            return "WORD";
        case OPERATOR:
            return "OPERATOR";
        case VALUE:
            return "VALUE";

    }
}   
void parsing(struct tokens_s** tokens, struct operation_mode_s* opm, struct pars_s** ret){
    int pos = 0;
    int token_count = (*tokens)->num;
    size_t sizeout = sizeof(struct pars_s) + (sizeof(struct element_s) * token_count);
    size_t sizeout2 = sizeof(struct element_s) * token_count;
    *ret = malloc(sizeout);
    (*ret)->elements = malloc(sizeout2);
    opm->useDinamiceRam += sizeout+sizeout2;  
    int element_count = 0;

    while (pos < token_count) {
        skipTab(&pos, *tokens);
        enum TNAME current_type = (*tokens)[pos].type;
        char current_value = (*tokens)[pos].value;
        
       
        
        if (pos >= token_count) break;
        if(current_type == STRING){
           char* word = getStrNum(&pos, *tokens);
            (*ret)->elements[element_count].type = WORD;
            (*ret)->elements[element_count].value = word;
            element_count++;
        }
        else if(current_type == TO ||current_type == FROM ||current_type == ADD ||current_type == REMOVE || current_type == ENDL) {
            (*ret)->elements[element_count] = getOperator(&pos, *tokens);
            element_count++;
        }
        else if(current_type == NUMBER){
            char* number = getNumber(&pos, *tokens);
            (*ret)->elements[element_count].type = VALUE;
            (*ret)->elements[element_count].vtype = VNUMBER;
            (*ret)->elements[element_count].value = number;
            element_count++;
        }
        else if(current_type == ENDL){
            (*ret)->elements[element_count].type = OPERATOR;
            (*ret)->elements[element_count].vtype = O_END;
            (*ret)->elements[element_count].value = malloc(2);
            (*ret)->elements[element_count].value[0] = current_value;
            (*ret)->elements[element_count].value[1] = '\0';
            
            element_count++;
            pos++;
        }
        else{
            pos++;
        }

      
    }
    (*ret)->num = element_count;
    opm->useDinamiceRam -= (*tokens)->num * sizeof(struct tokens_s);
    free(*tokens);
    *tokens = NULL;
    
}