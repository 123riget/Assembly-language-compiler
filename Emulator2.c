#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#define MAX_SIZE 100
#define FORMAT_SIZE 9
#define TABLE_SIZE 100
#define MAX_STR_LEN 33
#define MAX_INSTRUCTIONS 100000
int pc = 0; /*Program counter */
int sp = 0; /* stake pointer */
int regA = 0; /* register A */
int regB = 0; /* register B */
int inst_count = 0;
size_t size;
char filename[MAX_SIZE];
char option[MAX_SIZE];
/*size_t size;*/
int integer_machineCode[MAX_INSTRUCTIONS]={0};
typedef struct
{
    char b[FORMAT_SIZE];
    char a;
}format;
/*Structure for map*/
typedef struct Node{
    char *value;
    int key;
    struct Node *next;
}Node;
typedef struct Map{
    Node *table[TABLE_SIZE];
}Map;
Map mnemonic_without_operand;
Map mnemonic_with_operand;
Map offset_mnemonic;
/*End of structure*/
/*Structure for set<pair<int,pair<string,string>>> data structure*/
typedef struct{
    char *first;
    char *second;
}StringPair;
typedef struct{
    int first;
    StringPair second;
} IntStringPair;
typedef struct NodeP{
    IntStringPair data;
    struct NodeP* left;
    struct NodeP* right;
}NodeP;
NodeP* isa = NULL;
/*End of structure*/
/*Format of hexadecimal number */
format hex_bin[]={
        {"0000",'0'},
        {"0001",'1'},
        {"0010",'2'},
        {"0011",'3'},
        {"0100",'4'},
        {"0101",'5'},
        {"0110",'6'},
        {"0111",'7'},
        {"1000",'8'},
        {"1001",'9'},
        {"1010",'a'},
        {"1011",'b'},
        {"1100",'c'},
        {"1101",'d'},
        {"1110",'e'},
        {"1111",'f'}
    };
/*End of format of hexadecimal number */
/*Function of this code */
/*-------------------------------------------------------------------------------------------------------------------------------------*/
void exe(void);
void initial(void);
StringPair createStringPair(const char* first, const char* second);
IntStringPair createIntStringPair(int first, const char* s1, const char* s2);
int comparePairs(IntStringPair p1, IntStringPair p2);
void freeStringPair(StringPair sp);
void freeTree(NodeP* root);
void printTree(NodeP *root);
NodeP* insertPair(NodeP* root,IntStringPair data);
NodeP* searchPair(NodeP* root, IntStringPair data);
unsigned int hash(int key);
void initMap(Map *map);
void insert(Map *map, const char* key,int value);
int findM(Map *map,int key);
void freeMap(Map *map);
void dec_to_bin(int code,char *bin_str, int bits);
char* dec_2comp(int code,  int bits);
char *substring(const char *source,int start,int end);
int str_dec(const char* code, int bits);
int str_dec24(const char* code);
int snprintf(char *buffer,size_t size,const char *format, ...);
char* strdup(const char* str);
/*---------------------------------------------------------------------------------------------------------------------------------------*/
char machineCode[MAX_INSTRUCTIONS][MAX_STR_LEN];
char machineCode2[MAX_INSTRUCTIONS][MAX_STR_LEN];
int main(int argc, char **argv)
{
    FILE *fptr = NULL;
    char f_name[260];
    int i;
    initMap(&mnemonic_with_operand);
    initMap(&mnemonic_without_operand);
    initMap(&offset_mnemonic);
    if (argc !=3)
    {
        printf("Enter in correct format: .\\emu options filename.o\n");
        printf("-isa ISA(instruction set Architecture) \n");
        printf("-trace   trace of the code \n");
        printf("-write   memory writes \n");
        printf("-wipe   wipe written flags before execution\n");
        printf("-before   memory dump before execution \n");
        printf("-after    memory dump after execution \n");
        printf("-read     memory writes \n");
        return 0;
    }
    initial();
    strncpy(filename, argv[2],strchr(argv[2],'.')- argv[2]);
    filename[(strchr(argv[2],'.') - argv[2])+1]='\0';
    strncpy(option , argv[1],sizeof(option)-1);
    option[sizeof(option)-1]='\0';
    snprintf(f_name, sizeof(f_name),"%s.o",filename);
    fptr = fopen(f_name,"rb");
    if(fptr != NULL)
    {
        size = fread(integer_machineCode, sizeof(int), MAX_INSTRUCTIONS,fptr);
        if(size == 0 && ferror(fptr)){
            printf("Error reading the file %s.o\n",filename);
            fclose(fptr);
            return 1;
        }
        i=0;
        for(i = 0; i < 1000; i++)
        {
            strcpy(machineCode[i],dec_2comp(integer_machineCode[i],32));
        }
        if(strcmp(option , "-wipe") == 0){
            printf("Program Counter(PC),Stack Pointer(SP), Reg A, Reg B are reset\n");
            pc = sp = regA = regB = 0;
        }
        else if(strcmp(option, "-before") == 0)
        {
            printf("Memory dump before code execution :\n");
            for(i=0 ; i<50 ; i++)
            {
                if(i%4 == 0)
                printf("%s %s ",dec_2comp(i,32), machineCode[i]);
                else
                printf("%s ", machineCode[i]);
                if(i%4 == 3)
                printf("\n");
            }
            printf("\n\n");
        }
        else if(strcmp(option , "-isa") == 0)
        {
            printf("\nOpcode  Mnemonic  operand\n");
            printTree(isa);
            printf("        SET     value\n\n");
        }
        exe();
        for(i = 0; i < 1000; i++)
        {
            strcpy(machineCode2[i],dec_2comp(integer_machineCode[i],32));
        }
        if(strcmp(option, "-trace") == 0 )
        {
            printf("Program Counter: %s  Stack Pointer: %s  Register A: %s  Register B:%s  Instruction Count: %d\n",dec_2comp(pc,32),dec_2comp(sp,32),dec_2comp(regA,32),dec_2comp(regB,32),inst_count);
        }
        else if(strcmp(option,"-after") == 0)
        {
            printf("Memory dump after code execution \n");
            for(i=0 ;i<50 ; i++)
            {
                if(i % 4 == 0)
                printf("%s %s ", dec_2comp(i,32) , machineCode2[i]);
                else
                printf("%s ",machineCode2[i]);
                if(i % 4 == 3)
                printf("\n");
            }
            printf("\n\n");
        }
        if(inst_count == (int)size && strcmp(option,"-trace") == 0){
            printf("\nProgram executed\n");
        }
        fclose(fptr);
    }
    else
    printf("%s doesn't exist\n", filename);
    return 0;
}
void initial(void)
{
    insert(&mnemonic_without_operand,"add",6);
    insert(&mnemonic_without_operand,"sub",7);
    insert(&mnemonic_without_operand,"shl",8);
    insert(&mnemonic_without_operand,"shr",9);
    insert(&mnemonic_without_operand,"a2sp",11);
    insert(&mnemonic_without_operand,"sp2a",12);
    insert(&mnemonic_without_operand,"return",14);
    insert(&mnemonic_without_operand,"HALT",18);
    insert(&mnemonic_with_operand,"ldc",0);
    insert(&mnemonic_with_operand,"adc",1);
    insert(&mnemonic_with_operand,"ldl",2);
    insert(&mnemonic_with_operand,"stl",3);
    insert(&mnemonic_with_operand,"ldnl",4);
    insert(&mnemonic_with_operand,"stnl",5);
    insert(&mnemonic_with_operand,"adj",10);
    insert(&mnemonic_with_operand,"call",13);
    insert(&mnemonic_with_operand,"brz",15);
    insert(&mnemonic_with_operand,"brlz",16);
    insert(&mnemonic_with_operand,"br",17);
    insert(&mnemonic_with_operand,"SET",19);
    insert(&mnemonic_with_operand,"data",20);
    insert(&offset_mnemonic,"call",13);
    insert(&offset_mnemonic,"brz",15);
    insert(&offset_mnemonic,"brlz",16);
    insert(&offset_mnemonic,"br",17);
    isa = insertPair(isa,createIntStringPair(0,"ldc","value"));
    isa = insertPair(isa,createIntStringPair(1,"adc","value"));
    isa = insertPair(isa,createIntStringPair(2,"ldl","value"));
    isa = insertPair(isa,createIntStringPair(3,"stl","value"));
    isa = insertPair(isa,createIntStringPair(4,"ldnl","value"));
    isa = insertPair(isa,createIntStringPair(5,"ldnl","value"));
    isa = insertPair(isa,createIntStringPair(6,"add",""));
    isa = insertPair(isa,createIntStringPair(7,"sub",""));
    isa = insertPair(isa,createIntStringPair(8,"shl",""));
    isa = insertPair(isa,createIntStringPair(9,"shr",""));
    isa = insertPair(isa,createIntStringPair(10,"adj","value"));
    isa = insertPair(isa,createIntStringPair(11,"a2sp",""));
    isa = insertPair(isa,createIntStringPair(12,"sp2a",""));
    isa = insertPair(isa,createIntStringPair(13,"call","offset"));
    isa = insertPair(isa,createIntStringPair(14,"return",""));
    isa = insertPair(isa,createIntStringPair(15,"brz","offset"));
    isa = insertPair(isa,createIntStringPair(16,"brlz","offset"));
    isa = insertPair(isa,createIntStringPair(17,"br","offset"));
    isa = insertPair(isa,createIntStringPair(18,"HALT",""));
    return;
}
/*Program for the creation of set<pair <int, pair<string,string>>> data structure*/
StringPair createStringPair(const char* first, const char* second)
{
    StringPair sp;
    sp.first = strdup(first);
    sp.second = strdup(second);
    return sp;
}
IntStringPair createIntStringPair(int first, const char* s1, const char* s2)
{
    IntStringPair isp;
    isp.first = first;
    isp.second = createStringPair(s1,s2);
    return isp;
}
int comparePairs(IntStringPair p1, IntStringPair p2)
{
    int cmp1;
    if(p1.first != p2.first)
        return p1.first - p2.first;
    cmp1 = strcmp(p1.second.first,p2.second.first);
    if(cmp1 != 0)
        return cmp1;
    return strcmp(p1.second.second, p2.second.second);   
}
void freeStringPair(StringPair sp)
{
    free(sp.first);
    free(sp.second);
}
void freeTree(NodeP* root)
{
    if(root)
    {
        freeStringPair(root->data.second);
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}
NodeP* insertPair(NodeP* root,IntStringPair data)
{
    int cmp;
    if ( root == NULL )
    {
        NodeP* newNode = (NodeP*)malloc(sizeof(NodeP));
        newNode -> data = data;
        newNode -> left = newNode -> right = NULL;
        return newNode;
    }
    cmp = comparePairs(data , root->data);
    if(cmp<0)
    root -> left =insertPair(root->left ,data);
    else if(cmp>0)
    root->right = insertPair(root->right, data);
    /*Ignore duplicates*/
    return root;
}
NodeP* searchPair(NodeP* root, IntStringPair data)
{
    if(root == NULL|| comparePairs(data, root->data) == 0)
    return root;
    if(comparePairs(data , root->data)<0)
    return searchPair(root->left , data);
    return searchPair(root->right,data);
}
void printTree(NodeP *root)
{
    if(root){
        printTree(root->left);
        printf("%d   %s   %s\n",root->data.first, root->data.second.first , root->data.second.second);
        printTree(root->right);
    }
}
/*End of function */
/*Code to implement map function */
unsigned int hash(int key)
{
    return key%TABLE_SIZE;
}
/*initializing the map*/
void initMap(Map *map)
{
    int i;
    for(i=0;i< TABLE_SIZE;i++)
    map -> table[i] = NULL;
}
/*-----------------------------------------------------------------*/
/*Insert the key value pair into the map */
void insert(Map *map, const char* value,int key)
{
    unsigned int index = hash(key);
    Node *newNode = malloc(sizeof(Node));
    if(!newNode)
    {
        perror("Failed to allocate memory for node");
        exit(0);
    }
        newNode->key = key;
        newNode->value = strdup(value);
        newNode->next = NULL;
    /*Insert at the beginning of the list*/
    newNode -> next = map->table[index];
    map->table[index]=newNode;
}
/*Lookup a value by key*/
int findM(Map *map,int key)
{
    int found ;
    unsigned int index=hash(key);
    Node *current = map->table[index];
    while(current)
    {
        if(current->key == key)
        {
            found = 1;
            return found;
        }
        current= current->next;
    }
    found = 0;
    return 0;
}
/*free allocated memory*/
void freeMap(Map *map)
{
    int i ;
    for(i=0 ; i<TABLE_SIZE;i++)
    {
        Node *current= map->table[i];
        while(current)
        {
            Node *temp= current;
            current=current->next;
            free(temp->value);
            free(temp);
        }
    }
}
/*End of the code to implement map function */
/*---------------------------------------------------------------------------------------------------------*/
/*Function to convert decimal number to binary string*/
void dec_to_bin(int code,char *bin_str, int bits)
{
    int i; 
    for(i = 0; i< bits; i++)
    {
        bin_str[bits - i - 1] = (code & (1 << i)) ? '1' : '0' ;
    }
    bin_str[bits] = '\0';   /*Null terminate the string*/
}
/*Function to convert a decimal number to two's complement hexadecimal representation */
char* dec_2comp(int code,  int bits)
{
    char *hex_str= (char*)malloc((bits/4+1)*sizeof(char));
    char *bin_str;
    char *hex_result;
    int i,j,hex_index;
    char nibble[5];
    if(!hex_str)
    {
        perror("Memory allocation failed\n");
        exit(1);
    }
    /*char bin_str[bits+1];*/
    bin_str=(char*)malloc((bits+1)*sizeof(char));
    dec_to_bin(code, bin_str,bits);
    /*char hex_result [bits/4 + 1];*/
    hex_result = (char*)malloc((bits/4+1)*sizeof(char));
    hex_index = 0;
    for (i = 0; i < bits ; i+=4)
    {
        /*char nibble[5]={ bin_str[i],bin_str[i+1],bin_str[i+2],bin_str[i+3],'\0'};*/
        nibble[0]=bin_str[i];
        nibble[1]=bin_str[i+1];
        nibble[2]=bin_str[i+2];
        nibble[3]=bin_str[i+3];
        nibble[4]='\0';
        for(j = 0; j < 16; j++)
        {
            if(strcmp(hex_bin[j].b, nibble) == 0){
                hex_result[hex_index++] = hex_bin[j].a;
                break;
            }
        }
    }
    hex_result[hex_index]='\0';
    strcpy(hex_str,hex_result);
    free(bin_str);
    free(hex_result);
    return hex_str;
}
/*End of function */
/*------------------------------------------------------------------------------------*/
/*Function to mimick substring function of c++*/
char *substring(const char *source,int start,int end)
{
    int i,length;
    char *result = NULL;
    if(end<start || start < 0 || end > (int)strlen(source) )
    {
        return NULL;
    }
    if(start == end)
    {
        char *result = (char *)malloc(2*sizeof(char));
        result[0]=source[start];
        result[1]='\0';
        return result;
    }
    length = end - start ; /*Previously it was int length = end - start + 1;*/
    result = (char *)malloc ((length+1)*sizeof(char));
    if(result==NULL)
    {
        return NULL; 
    }
    for(i=0 ;i<length;i++)
    {
        result[i] = source[start+i];
    }
    result[length] = '\0';
    return result;
}
/*End of function */
int str_dec(const char* code, int bits)
{
    int required_length = bits/4;
    int original_length;
    unsigned int y;
    /*char padded_code[required_length+1];*/
    char *padded_code = (char*)malloc((required_length+1)*sizeof(char));
    original_length = strlen(code);
    if(original_length < required_length)
    {
        memset(padded_code, '0', required_length-original_length);
        strcpy(padded_code+ (required_length - original_length), code);
    } else {
        strcpy(padded_code,code);
    }
    padded_code[required_length] ='\0';
    y = (unsigned int )strtoul(padded_code,NULL,16);
    free(padded_code);
    return (int)y;
}
int str_dec24(const char* code)
{
    /*Determining if the number is negative based on the first digit*/
    int original_length;
    char padded_code [9];
    unsigned int y;
    int is_negative = (code[0] == '8' || code[0] == '9' || (code[0] >= 'a' && code[0] <= 'f'));
    original_length = strlen(code);
    /*Padding with F or 0 based on sign*/
    if(is_negative)
    {
        memset(padded_code , 'f', 8 - original_length);
    }
    else
    {
        memset(padded_code, '0', 8 - original_length);
    }
    /*copying the original code after padding */
    strcpy(padded_code + (8 - original_length),code);
    padded_code[8] = '\0';
    y = (unsigned int ) strtoul(padded_code, NULL , 16);
    /*Casting to signed integer and returning */
    return (int )y;
}
void exe(void)
{
    int execute = 0;
    for (pc = 0; pc <(int) size; pc++)
    {
        int opcode,operand;
        execute++;
        inst_count++;
        opcode = str_dec(substring(machineCode[pc],6,8),8);
        operand = str_dec24(substring(machineCode[pc],0,6));
        if(findM(&mnemonic_without_operand,opcode)!=0)
        {
            execute++;
            if(strcmp(option,"-trace")==0)
            {
                printf("Program Counter: %s  Stack pointer: %s  Register A: %s  Register B:%s  Instruction Count: %d  \n",dec_2comp(pc,32),dec_2comp(sp,32),dec_2comp(regA,32),dec_2comp(regB,32),inst_count);
            }
            if(opcode == 6)
            {
                regA= regB + regA;
            }
            else if(opcode == 7)
            {
                regA = regB - regA;
            }
            else if(opcode == 8)
            {
                regA = (regB << regA);
            }
            else if(opcode == 9)
            {
                regA = (regB >> regA);
            }
            else if(opcode == 11)
            {
                sp = regA;
                regA = regB;
            }
            else if(opcode == 12)
            {
                regB = regA;
                regA = sp;
            }
            else if(opcode == 14)
            {
                pc = regA;
                regA = regB;
            }
            else if(opcode == 18)
            {
                printf("\nProgram execution finished \n");
                return;
            }
            else
            {
                printf("\nInvalid instruction , Execution stopped\n");
                return;
            }
        }
        else if(findM(&mnemonic_with_operand,opcode)!=0)
        {
            if(strcmp(option,"-trace")==0)
            {
            printf("Program Counter: %s  Stack pointer: %s  Register A: %s  Register B:%s  Instruction Count: %d  \n",dec_2comp(pc,32),dec_2comp(sp,32),dec_2comp(regA,32),dec_2comp(regB,32),inst_count);
            }
            if(findM(&offset_mnemonic,opcode)!=0)
            {
                if(opcode == 13)
                {
                    regB = regA;
                    regA = pc;
                    pc = pc + operand;
                }
                else if(opcode == 15)
                {
                    if(regA == 0)
                    {
                        pc = pc + operand;
                    }
                }
                else if(opcode == 16)
                {
                    if(regA < 0)
                    {
                        pc = pc + operand;
                    }
                }
                else if(opcode == 17)
                {
                    pc = pc + operand;
                }
                else 
                {
                    printf("\nInvalid Instruction , Execution stopped\n");
                    return;
                }
            }
            else
            {
                execute++;
                if(opcode == 0)
                {
                    regB = regA;
                    regA = operand;
                }
                else if(opcode == 1)
                {
                    regA = regA + operand;
                }
                else if(opcode == 2)
                {
                    regB = regA;
                    if(sp + operand < 0 || sp + operand > 100000)
                    {
                        if(strcmp(option,"-trace")==0)
                        {
                            printf("Error: Invalid Memory Access\n");
                        }
                        return;
                    }
                    regA = integer_machineCode[sp + operand];
                    if(strcmp(option,"-read")==0)
                    {
                        printf("Reading Memory  %s finds  %s \n",dec_2comp(sp+operand,32),dec_2comp(regA,32));
                    }
                }
                else if(opcode == 3)
                {
                    if(sp + operand < 0 || sp + operand > 100000)
                    {
                        if(strcmp(option,"-trace")==0)
                        {
                            printf("Error: Invalid memory Access\n");
                        }
                        return ;
                    }
                    if(strcmp(option,"-write")==0)
                    {
                        printf("Writing memory  %s was  %s",dec_2comp(sp+operand,32),dec_2comp(integer_machineCode[sp+operand],32));
                    }
                    integer_machineCode[sp + operand] = regA;
                    if(strcmp(option,"-write")==0)
                    {
                        printf(" now  %s\n",dec_2comp(regA,32));
                    }
                    regA = regB;
                }
                else if(opcode == 4)
                {
                    if(regA + operand < 0 || regA + operand > 100000)
                    {
                        if(strcmp(option , "-trace")==0)
                        {
                            printf("Error: Invalid Memory Access\n");
                            return;
                        }
                    }
                    if(strcmp(option,"-read")==0)
                    {
                        printf("Reading Memory  %s finds  %s \n",dec_2comp(regA + operand, 32),dec_2comp(integer_machineCode[regA + operand],32));
                    }
                    regA = integer_machineCode[regA + operand];
                }
                else if(opcode == 5)
                {
                    if(regA + operand < 0 || regA + operand > 100000)
                    {
                        if(strcmp(option,"-trace")==0)
                        {
                            printf("Error: Invalid Memory Access\n");
                            return;
                        }
                    }
                    if(strcmp(option,"-write")==0)
                    {
                        printf("Writing memory  %s was  %s",dec_2comp(regA + operand,32),dec_2comp(integer_machineCode[regA + operand],32));
                    }
                    integer_machineCode[regA + operand] = regB;
                    if(strcmp(option,"-write")==0)
                    {
                        printf(" now  %s \n",dec_2comp(regB,32));
                    }
                }
                else if(opcode == 10)
                {
                    sp = sp + operand;
                }
                else
                {
                    printf("Invalid instruction, Execution stopped\n");
                    return;
                }
            }
        }
        if(pc >= (int)size)
        {
            if(strcmp(option,"-trace")==0)
            {
                printf("Error: Invalid Memory Access \n");
            }
            return;
        }
    }
}
int snprintf(char *buffer,size_t size,const char *format, ...)
{
    int written;
    char temp_buffer[10240];
    va_list args;
    va_start(args, format);
    written = vsprintf(temp_buffer,format,args);
    va_end(args);
    if(written< (int )size)
    {
        strcpy(buffer,temp_buffer);
    }
    else
    {
        strncpy(buffer,temp_buffer,size-1);
        buffer[size-1]='\0';
    }
    return written;
}
/*Implementation of strdup()*/
char* strdup(const char* str){
    char* new_str = (char*) malloc(strlen(str)+1);
    if(new_str)
    {
        strcpy(new_str,str);
    }
    return new_str;
}
/*End of strdup() function*/