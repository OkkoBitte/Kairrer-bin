

static char specsim[] = {' ', '\n',  ';', '<', '>', '+', '-'}; 


static int isSpecsim(char c){
    for(int i = 0; i < sizeof(specsim); i++){
        if(c == specsim[i]) return OK;
    }
    return ERR;
}
static int isNumber(char c){
    switch (c) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return OK;
        default: return ERR;
    }

}
static int isChar(char c){
    if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return OK;
    return ERR;
}

struct tokens_s getTokenFromSim(char sim){
    struct tokens_s ret;
    switch (sim) {
        case ' ':
            ret.type = TAB;
            ret.value = sim;
            break;
        case '\n':
            ret.type = NLINE;
            ret.value = sim;
            break;
        case ';':
            ret.type = ENDL;
            ret.value = sim;
            break;
        case '<':
            ret.type = TO;
            ret.value = sim;
            break;
        case '>':
            ret.type = FROM;
            ret.value = sim;
            break;
        case '+':
            ret.type = ADD;
            ret.value = sim;
            break;
        case '-':
            ret.type = REMOVE;
            ret.value = sim;
            break;
        default:
            
            ret.type = ANY; 
            ret.value = sim; 
            break;
    }
    return ret;
}

char* getTokenSimFromType(enum TNAME type){
    switch (type) {
        case TAB:
            return "TAB";
        case NLINE:
            return "NLINE";
        case ENDL:
            return "ENDL";
        case TO:
            return "TO";
        case FROM:
            return "FROM";
        case ADD:
            return "ADD";
        case REMOVE:
            return "REMOVE";
        case NUMBER:
            return "NUMBER";
        case STRING:
            return "STRING";
        case ANY:
            return "ANY";
        default:
            return "UNKNOWN";
    }
}


void tokenizer(getFileChars_ti *rch, struct operation_mode_s *cmd, struct tokens_s ** tokens){
    
    int pos = 0;
    size_t sizeout = sizeof(struct tokens_s) * rch->size;
    *tokens  = malloc(sizeout);
    cmd->useDinamiceRam += sizeout;
    int tokenCount = 0;
    char *data = rch->data;
    while(data[pos] != '\0' && tokenCount < cmd->maxRamUse){
        if(isSpecsim(data[pos]) == OK){
            (*tokens)[tokenCount] = getTokenFromSim(data[pos]);
            tokenCount++;
            pos++;
        }
        else if(isNumber(data[pos]) == OK){
            (*tokens)[tokenCount].type = NUMBER;
            (*tokens)[tokenCount].value = data[pos];
            tokenCount++;
            pos++;
        }
        else if(isChar(data[pos]) == OK){
            (*tokens)[tokenCount].type = STRING;
            (*tokens)[tokenCount].value = data[pos];
            tokenCount++;
            pos++;
        
        }
        else{
            (*tokens)[tokenCount].type = ANY;
            (*tokens)[tokenCount].value = data[pos];
            tokenCount++;
            pos++;
        }
    }
    (*tokens)->num = tokenCount;
    

    free(rch->data);
    cmd->useDinamiceRam -= rch->size;
    return;

}