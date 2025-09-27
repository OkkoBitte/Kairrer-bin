#define ERR 1
#define SERR -1
#define OK 0

struct compilator_s{
    int maxReadBuffer;
};

struct operation_mode_s{
    char* startFile;
    int maxRamUse;
    int useDinamiceRam;
    struct compilator_s cmp;
};


typedef struct {
    int status;
    char why;
    char *data;
    int size;
}getFileChars_ti;


enum TNAME{
    TO,FROM,
    ADD, REMOVE,
    RAM,

    NUMBER, STRING,

    TAB, NLINE, ENDL,

    ANY
};

struct tokens_s{
    enum TNAME type;
    char value;

    int num;
};


enum ETNAME{
    WORD, OPERATOR, VALUE
};
enum VTYPE{
    VNUMBER, VSTRING,
    VHEX
};
enum OTYPE{
    O_TO, O_FROM,
    O_ADD, O_REMOVE,

    O_END
};
struct element_s{
    enum ETNAME type; 

    enum VTYPE vtype; // if value
    enum OTYPE otype; // if operator
    char* value; // word
};

struct line_s{
    struct element_s * elements;
    int num; // sektal hamu element
};

struct pars_s{
    struct element_s* elements; // [],[],[]
    int num;
};



struct machine_code_s{
    uint8_t* code;
    int size;
};

