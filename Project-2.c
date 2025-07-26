#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#define MAX_SIZE 100
#define MAX_LINES 100000
#define MAX_TOKENS_PER_LINE 10
#define MAX_TOKEN_LENGTH 50
#define SIZE 100
#define TABLE_SIZE 100
/* typedef struct
{
    char a[MAX_SIZE];
    int opcode;
}Mnemonic;*/
typedef struct
{
    char b[MAX_SIZE];
    char a;
}format;
typedef struct
{
    char a[MAX_SIZE];
}string;
typedef struct {
    char key[50];
    int value;
} KeyValuePair;
typedef struct{
    int line_num;
    char *message;
}Error;
/*Structure for map*/
typedef struct Node{
    char *key;
    int value;
    struct Node *next;
}Node;
typedef struct Map{
    Node *table[TABLE_SIZE];
}Map;
/*Vector implementation in C*/
typedef struct{
    char **data;
    size_t size;
    size_t capacity;
}StringVector;
StringVector used_labels;
/*end*/
/*Structure for warning message*/
typedef struct Warning{
    int line;
    char *message;
}Warning;
typedef struct WarningList{
    Warning *warnings;
    int size;
    int capacity;
} WarningList;
WarningList warnings;
/*end of structure for warning message*/
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
        {"1010",'A'},
        {"1011",'B'},
        {"1100",'C'},
        {"1101",'D'},
        {"1110",'E'},
        {"1111",'F'}
    };
/*-----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/*All the functions used*/
void firstpass(char *s,int line_num,int *p);
void labels_using_SET(void);
int str_to_dec(const char* num,size_t bits,int line_num);
void secondpass(void);
void machineCodeFunc(int instr_type, int iter ,const char *mnemonic,const char* operand,const char *label,int line_num);
void labels_not_used(Map* labels,StringVector *used_labels,WarningList *warnings);
char* dec_2comp(int num, int bits);
void writelist(const char* filename);
int compareStrings(const void *a,const void *b);
void sortUsedLabels(StringVector *vector);
void writelog(const  char *filename, Map *labels,StringVector *used_labels,WarningList *warnings, Error **errors,int error_count);
void writeobj(const char *filename);
void print_warn(WarningList *list);
void print_errors(Error **errorArray,int size);
void insertC(char *original, const char *toInsert, int pos);
int validop(char* s);
void add_token(char *lines[MAX_LINES][MAX_TOKENS_PER_LINE],int line_index,const char *token,int *token_count);
int find(const char *key);
char *substring(const char *source,int start,int end);
int compareErrors(const void *a, const void *b);
int addError(Error **errorArray, int *size,int line_num, const char *message);
char* strdup(const char* str);
void freeErrors(Error **errorArray,int size);
void initial(void);
bool validLabel(const char *s);
unsigned int hash(const char* key);
void initMap(Map *map);
void insert(Map *map, const char* key,int value);
int findM(Map *map,const char *key);
void freeMap(Map *map);
void initStringVector(StringVector *vector);
void addString(StringVector *vector, const char *str);
void freeStringVector(StringVector *vector);
int find_in_vector(StringVector *vector,const char* key);
void initWarnings(WarningList *list);
void addWarning(WarningList *list,int line ,const char* message);
void freeWarnings(WarningList *list);
size_t get_array_length(char **array);
void trim(char *str);
int snprintf(char *buffer,size_t size,const char *format, ...);
/*----------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
Map labels;
Map mnemonic_without_operand;
Map mnemonic_with_operand;
Map offset_mnemonic;
/*End of structure map*/
/*Error handling data*/
Error *errors= NULL;
int size = 0;
/*End*/
KeyValuePair data[SIZE]; /*Array to store key value pairs*/
int data_size=0; /* Number of elements in the array*/
char *source_code[MAX_LINES][MAX_TOKENS_PER_LINE]={{NULL}};
int token_count[MAX_LINES]={0};
char *machineCode[MAX_LINES][MAX_TOKENS_PER_LINE]={{NULL}};
int token_countM[MAX_LINES]={0};
int integer_machineCode[MAX_LINES]={0};
int main(int argc,char **argv)
{
    /*Variable declaration starts*/
    FILE *inputfile;
    size_t i=0;
    char filename[MAX_SIZE]="";
    char buffer[256];
    /*Variable declaration ends*/
    initMap(&labels);
    initMap(&mnemonic_with_operand);
    initMap(&mnemonic_without_operand);
    initMap(&offset_mnemonic);
    initStringVector(&used_labels);
    initWarnings(&warnings);
    initial();
    if(argc != 2)
    {
        printf("Invalid command format, It should have been ./asm filename.asm");
        return 1;
    }
    inputfile = fopen(argv[1],"r");
    for(i=0; i<strlen(argv[1]);i++)
    {
        if(argv[1][i]=='.')
        {
            break;
        }
        filename[i] = argv[1][i];
    }
    filename[i]='\0';
    if(!inputfile)
    {
        printf("Error opening the file: %s",argv[1]);
        return 1;
    }
    else
    {
        int line_num=1;
        int program_counter=0;
        while(fgets(buffer,sizeof(buffer),inputfile))
        {
            char s[MAX_SIZE]="";
            for(i=0;i < strlen(buffer);i++)
            {
                if(buffer[i] == ';')
                {
                    break;
                }
                s[i] =buffer[i];
            }
            s[i]='\0';
            trim(s);
            firstpass(s,line_num,&program_counter);
            line_num++;
        }
        /*if(errors==NULL)
        {
            if(warnings.size>0)
            {
                print_warn(&warnings);
            }
        }*/
        if(errors!=NULL)
        {
            print_errors(&errors,size);
            if(warnings.size > 0)
            {
                print_warn(&warnings);
            }
            exit(1);
        }
        labels_using_SET();
        secondpass();
        labels_not_used(&labels,&used_labels,&warnings);
        writelist(filename);
        writelog(filename,&labels,&used_labels,&warnings,&errors,size);
        writeobj(filename);
        if(warnings.size>0)
        {
            print_warn(&warnings);
        }
        /*if(errors==NULL)
        {
            if(warnings.size>0)
            {
                print_warn(&warnings);
            }
        }
        else
        {
            print_errors(&errors,size);
            if(warnings.size > 0)
            {
                print_warn(&warnings);
            }
        }*/
        fclose(inputfile);
    }
    freeErrors(&errors,size);
    freeMap(&labels);
    freeMap(&offset_mnemonic);
    freeMap(&mnemonic_with_operand);
    freeMap(&mnemonic_without_operand);
    freeStringVector(&used_labels);
    freeWarnings(&warnings);
    return 0;
}
/*Code for the first pass*/
void firstpass(char *s,int line_num,int *p)
{
    /*Variable declaration starts*/
    int firstpass_count=0;
    int len_s=0;
    char s1[MAX_SIZE];
    int i=0,j=0;
    string wordsInLine[5];
    int len_wordsInLine[5]={0};
    char *token= NULL;
    char *mnemonic = NULL;
    char *labelname = NULL;
    /*Variable declaration ends*/
    if(strcmp(s,"")!=0)
    {
        len_s=strlen(s);
        for(i=0 ; i<len_s; i++)
        {
            s1[j] = s[i];
            j++;
            if(s[i]==':')
            {
                s1[j]=' ';
                j++;
            }
        }
        s1[j]='\0';
        for(i=0; i<5 ; i++)
        {
            strcpy(wordsInLine[i].a,"");
        }
        i=0;
        token= strtok(s1," ");
        while(token!=NULL && i<5)
        {
            trim(token);
            strcpy(wordsInLine[i].a,token);
            len_wordsInLine[i] = strlen(token);
            i++;
            token = strtok(NULL," ");
        }
        for(i=0;i<5;i++)
        {
            if(strcmp(wordsInLine[i].a,"")!=0)
            {
                add_token(source_code,*p,wordsInLine[i].a,token_count);
            }
            firstpass_count++;
        }
        if((wordsInLine[0].a)[len_wordsInLine[0]-1] == ':')
        {
            labelname = substring(wordsInLine[0].a,0,len_wordsInLine[0]-1);
            if(!(validLabel(labelname)))
            {
                addError(&errors, &size, line_num, "Invalid label name , label name should only include alphanumeric character and start with an alphabet");
            }
            else
            {
                if(findM(&labels,labelname)!=-1)
                {
                    addError(&errors, &size, line_num,"Duplicate label names not allowed");
                }
                else
                {
                    insert(&labels,labelname,*p);
                    if(!(len_wordsInLine[1]==0 &&len_wordsInLine[2]==0))
                    {
                        (*p)++;
                    }
                }
            }
            if(strcmp(wordsInLine[1].a,"")!=0&&(wordsInLine[1].a)[len_wordsInLine[1]-1]!=':')
            {
                mnemonic= wordsInLine[1].a;
                if(findM(&mnemonic_without_operand,mnemonic)!=-1)
                {
                    if(strcmp(wordsInLine[2].a,"")!=0)
                    {
                        addError(&errors,&size,line_num,"Unexpected operand, No operand allowed");
                    }
                }
                else if(findM(&mnemonic_with_operand,mnemonic)!=-1)
                {
                    if(strcmp(wordsInLine[3].a,"")!=0)
                    {
                        addError(&errors, &size,line_num,"Unexpected operand, only one operand allowed");
                    }
                }
                else
                {
                    addError(&errors,&size,line_num,"No such instruction");
                }
            }
            else if(strcmp(wordsInLine[1].a,"")!=0 && (wordsInLine[1].a)[strlen(wordsInLine[1].a)-1] == ':')
            {
                if(validLabel(substring(wordsInLine[1].a,0,strlen(wordsInLine[1].a)-1)))
                {
                    insert(&labels,substring(wordsInLine[1].a,0,strlen(wordsInLine[1].a)-1),*p);
                }
            }
        }
        else
        {
            mnemonic=wordsInLine[0].a;
            if(findM(&mnemonic_without_operand,mnemonic)!=-1)
            {
                if(strcmp(wordsInLine[1].a,"")!=0)
                {
                    addError(&errors,&size,line_num,"Unexpected Operand, No operand allowed");
                }
            }
            else if(findM(&mnemonic_with_operand,mnemonic)!=-1)
            {
                if(strcmp(wordsInLine[2].a,"")!=0)
                {
                    addError(&errors,&size,line_num, "Unexpected operand,only one operand allowed");
                }
            }
            else
            {
                addError(&errors,&size,line_num,"No such instruction");
            }
            (*p)++;
        }
    }
}
void labels_using_SET(void)
{
    int i;
    for(i=0; i< MAX_LINES;i++)
    {
        if(source_code[i][0]==NULL)
        {
            continue;
        }
        if(token_count[i] == 3 && strcmp(source_code[i][1],"SET")==0 && findM(&labels,substring(source_code[i][0],0,strlen(source_code[i][0])-1))!=-1)
        {
            insert(&labels,substring(source_code[i][0],0,strlen(source_code[i][0])-1),str_to_dec(source_code[i][2], 32,0));
        }
        else if(token_count[i] == 4 && strcmp(source_code[i][2],"SET")==0 && findM(&labels,substring(source_code[i][1],0,strlen(source_code[i][1])-1))!=-1)
        {
            insert(&labels,substring(source_code[i][1],0,strlen(source_code[i][1])-1),str_to_dec(source_code[i][3], 32,0));
        }
    }
}
int str_to_dec(const char* p,size_t bits,int line_num)
{
    char *num=(char*)p;
    size_t i;
    int val=0;
    int len=strlen(num);
    char sw[MAX_SIZE];
    char *endptr;
    long int y;
    if(strcmp(num,"0")==0)
    {
        return 0;
    }
    if(bits!=8)
    {
        if(validop(num) == 5)
        {
            return findM(&labels,num);
        }
        else if(validop(num)==1)
        {
            addError(&errors,&size,line_num,"No such label");
            return 0;
        }
        if(num[0] =='+')
        {
            if(num[1] == 0)
            {
                if(validop(substring(num,2,len))==3)
                {
                    int mul = 1;
                    for(i = len - 1;i >= 2; i--)
                    {
                        val += mul*(num[i] - 48);
                        mul *= 8;
                    }
                    return val;
                }
                else 
                {
                    addError(&errors,&size,line_num,"valid operand is required");
                }
            }
            else
            {
                if(validop(substring(num,1,len)) == 3 || validop(substring(num,1,len)) == 2 )
                {
                    int mul = 1;
                    for(i = len - 1; i >= 1 ; i--)
                    {
                        val += mul * (num[i] - 48);
                        mul *= 10;
                    }
                    return val;
                }
                else
                {
                    addError(&errors, &size,line_num, "valid operand is required");
                }
            }
        }
        else if(num[0] == '-')
        {
            if(num[1] == 0)
            {
                if(validop(substring(num,2,len)) == 3)
                {
                    int mul = 1;
                    for(i = len-1; i >= 2; i--)
                    {
                        val += mul * (num[i] - 48);
                        mul *= 8;
                    }
                    return -1*val;
                }
                else
                {
                    addError(&errors,&size,line_num,"valid operand is required");
                }
            }
            else
            {
                if(validop(substring(num,1,len)) == 3 || validop(substring(num,1,len)) == 2)
                {
                    int mul = 1;
                    for ( i = len - 1; i >= 1 ; i--)
                    {
                        val += mul * (num[i] - 48);
                        mul *= 10;
                    }
                    return -1*val;
                }
                else
                {
                    addError(&errors,&size,line_num,"valid operand is required");
                }
            }
        }
        else if(num[0] == '0' && num[1] == 'x')
        {
            bool hx= true;
            for(i = 2; i < strlen(num) ; i++)
            {
                if(!((num[i] >= '0' && num[i] <= '9') || (num[i] <= 'f' && num[i] >= 'a') || (num[i] <= 'F' && num[i] >= 'A')))
                {
                    hx = false;
                }
            }
            if(hx)
            {
                strcpy(sw,substring(num,2,len));
                y=0;
                if(num[2] == '8' || num[2] == '9' || (num[2] >= 'A' && num[2]<='F'))
                {
                    while(strlen(sw) < bits/4)
                    {
                        insertC(sw,"F",0);
                    }
                }
                else 
                {
                    while(strlen(sw)< bits/4)
                    {
                        insertC(sw,"0",0);
                    }
                }
                y = strtol(sw,&endptr,16);
                if(*endptr != '\0')
                {
                    printf("Invalid character found after conversion\n");
                }
                return (int)y;
            }
            else
            {
                addError(&errors,&size,line_num,"valid operand is required");
            }
        }
        else if(num[0] == '0')
        {
            if(validop(substring(num,1,len)) ==3 )
            {
                int mul = 1;
                for(i = len - 1; i>= 1; i--)
                {
                    val += mul * (num[i] - 48);
                    mul *= 8;
                }
                return val;
            }
            else
            {
                addError(&errors,&size,line_num,"valid operand is required");
            }
        }
        else
        {
            if(validop(num) == 3 || validop(num) == 2)
            {
                return (int)strtol(num,&endptr, 10);
            }
            else
            {
                addError(&errors,&size,line_num,"valid operand is required");
            }
        }
    }
    else
    {
        if(findM(&mnemonic_with_operand,num)!=-1)
        {
            return findM(&mnemonic_with_operand,num);
        }
        else if(findM(&mnemonic_without_operand,num)!=-1)
        {
            return findM(&mnemonic_without_operand,num);
        }
        else if(findM(&offset_mnemonic,num)!=-1)
        {
            return findM(&offset_mnemonic,num);
        }
        else 
        {
            return 0;
        }
    }
    return 1;
}
/*Code for second pass*/
void secondpass(void)
{
    int secondpass_count = 0;
    int i,r1,r2;
    char* mnemonic;
    char* operand;
    for(i=0 ; i<MAX_LINES;i++)
    {
        secondpass_count++;
        if(source_code[i][0]==NULL)
        {
            continue;
        }
        if(token_count[i] == 1)
        {
            machineCodeFunc(0, i ,source_code[i][0],"","",i);
            continue;
        }
        mnemonic=NULL;
        operand= NULL;
        r1 = 0;
        r2 = 0;
        if(source_code[i][0][strlen(source_code[i][0])-1] == ':')
            r1 = 1;
        if(source_code[i][1][strlen(source_code[i][1])-1] == ':')
            r2 = 1;
        if(r1 && r2)
        {
            mnemonic = (char*) malloc(strlen(source_code[i][2])+1);
            strcpy(mnemonic,source_code[i][2]);
            if(findM(&mnemonic_without_operand,mnemonic)!=-1)
            {
                machineCodeFunc(0, i ,mnemonic, "", "",i);
            }
            else
            {
                if(token_count[i] >= 4)
                {
                    operand = (char*)malloc(strlen(source_code[i][2])+1);
                    strcpy(operand, source_code[i][2]);
                }
                if(validLabel(operand) && findM(&labels,operand)!=-1)
                {
                    addString(&used_labels,operand);
                }
                machineCodeFunc(1, i ,mnemonic, operand,substring(source_code[i][1],0,strlen(source_code[i][1])-1),i);
            }
        }
        else if(r1 && !r2)
        {
            secondpass_count++;
            if(mnemonic!=NULL)
            free(mnemonic);
            mnemonic = (char*) malloc(strlen(source_code[i][1])+1);
            strcpy(mnemonic,source_code[i][1]);
            if(findM(&mnemonic_without_operand,mnemonic)!=-1)
            {
                machineCodeFunc(0,i,mnemonic,"","",i);
            }
            else
            {
                if(token_count[i]>=3)
                {
                    if(operand!=NULL)
                    free(operand);
                    operand = (char*)malloc(strlen(source_code[i][2])+1);
                    strcpy(operand, source_code[i][2]);
                }
                if(validLabel(operand) && findM(&labels,operand)!=-1)
                {
                    addString(&used_labels,operand);
                }
                machineCodeFunc(1, i, mnemonic, operand,substring(source_code[i][0],0,strlen(source_code[i][0])-1),i);
            }
        }
        else
        {
            secondpass_count++;
            if(mnemonic!=NULL)
            free(mnemonic);
            mnemonic=(char*)malloc(strlen(source_code[i][0])+1);
            strcpy(mnemonic,source_code[i][0]);
            if(findM(&mnemonic_without_operand,mnemonic)!=-1)
            {
                machineCodeFunc(0, i, mnemonic,"", "",i);
            }
            else
            {
                if(operand!=NULL)
                free(operand);
                operand = (char*) malloc(strlen(source_code[i][1])+1);
                strcpy(operand,source_code[i][1]);
                if(validLabel(operand) && findM(&labels,operand)!=-1)
                {
                    addString(&used_labels,operand);
                }
                machineCodeFunc(1, i, mnemonic, operand, "",i);
            }
        }
        if(mnemonic!=NULL)
        free(mnemonic);
        if(operand!=NULL)
        free(operand);
    }
}
/*End of function for second pass*/
/*Machine code function */
void machineCodeFunc(int instr_type, int iter ,const char *mnemonic,const char* operand,const char *label,int line_num)
{
    /*Variable declaration starts*/
    char *a=NULL;
    int val;
    int val_op,val_mno;
    char *s1= NULL;
    char *s2 = NULL;
    char *s3 = NULL;
    char *final_result = NULL;
    if(instr_type == 0)
    {
        if(findM(&mnemonic_without_operand, mnemonic) !=-1 )
        {
            integer_machineCode[iter] = findM(&mnemonic_without_operand,mnemonic);
            /*char *a;*/
            a=dec_2comp(findM(&mnemonic_without_operand,mnemonic),32);
            add_token(machineCode,iter,a,token_countM);
            free(a);
        }
        else
        {
            char buffer[256];
            snprintf(buffer,sizeof(buffer),"invalid mnemonic: %s",mnemonic);
            addError(&errors,&size,line_num,buffer);
        }
    }
    else
    {
        if(strcmp(mnemonic,"data")==0)
        {
            addString(&used_labels,label);
            val = str_to_dec(operand,32,line_num);/*int val*/
            integer_machineCode[iter]= val;
            a=dec_2comp(val,32); /*char *a*/
            add_token(machineCode, iter ,a,token_countM);
            free(a);
        }
        else if(strcmp(mnemonic,"SET")==0)
        {
            addString(&used_labels,label);
            val = str_to_dec(operand, 32,line_num);/*int val*/
            integer_machineCode[iter] = val;
            a=dec_2comp(val,32);/*char *a*/
            add_token(machineCode, iter ,a,token_countM);
            free(a);
        }
        else if(findM(&mnemonic_with_operand,mnemonic)!=-1)
        {
            val_op = str_to_dec(operand,24,line_num);/*int val_op*/
            val_mno = 0;/*int val_mno*/
            if(findM(&offset_mnemonic,mnemonic)!=-1)
            {
                val_mno = findM(&offset_mnemonic,mnemonic);
                if(validLabel(operand))
                {
                    val_op = val_op - iter - 1;
                }
            }
            else if(findM(&mnemonic_with_operand, mnemonic)!=-1)
            {
                val_mno = findM(&mnemonic_with_operand,mnemonic);
            }
            integer_machineCode[iter]= val_op << 8 | val_mno;
            s1 = dec_2comp(val_op, 24);/*char *s1*/
            s2=NULL;/*char *s2*/
            if(val_op<0)
            {
                s2=substring(s1,2,8); 
            }
            else
            {
                s2 = s1;
            }
            s3= dec_2comp(val_mno, 8);/*char *s3*/
            final_result= (char*)malloc(strlen(s2)+strlen(s3)+1);/*char *final_result*/
            final_result[0]='\0';
            strcat(final_result,s2);
            strcat(final_result,s3);
            add_token(machineCode,iter,final_result,token_countM);
            free(s3);
            if(s2!=s1)
            free(s2);
            free(final_result);
        }
        else
        {
            char buffer[256];
            snprintf(buffer,sizeof(buffer),"invalid mnemonic :%s", mnemonic);
            addError(&errors,&size,line_num,buffer);
        }
    }
}
/*Code for labels_not_used*/
void labels_not_used(Map* labels,StringVector *used_labels,WarningList *warnings)
{
    int i;
    for(i=0;i<TABLE_SIZE;i++)
    {
        Node *current=labels->table[i];
        while(current)
        {
            if(!find_in_vector(used_labels, current->key))
            {
                char *message = malloc(50 + strlen(current->key));
                sprintf(message,"Unused label %s",current->key);
                addWarning(warnings, 0, message);
                free(message);
            }
            current = current->next;
        }
    } 
}
/*End of function labels not used*/
char* dec_2comp(int num, int bits)
{
    /*Calculating the number of hexadecimal digits if needed*/
    unsigned int mask;
    unsigned int twos_complement;
    int hex_digits = bits/4;
    char* hex_str;
    if(num>=0)
    {
        /*Allocating memory for the resulting string*/
        hex_str=(char*) malloc(hex_digits + 1);
        if(hex_str == NULL)
        {
            perror("Failed to allocate memory in hex strings");
            return NULL;
        }
        /*For negative number management using two's complement*/
        mask =~0U;/*Unsigned int mask*/
        twos_complement = num&mask;/*unsigned int twos_complement*/
        /*Formatting the number into the string with leading zeroes and the appropriate width */
        snprintf(hex_str,hex_digits+1,"%0*x",hex_digits,twos_complement);
    }
    else
    {
        hex_digits = 8;
        hex_str=(char*) malloc(hex_digits+1);
        if(hex_str == NULL)
        {
            perror("Failed to allocate memory in hex strings");
            return NULL;
        }
        /*For negative number management using two's complement*/
        mask =~0U;
        twos_complement = num&mask;
        /*Formatting the number into the string with leading zeroes and the appropriate width */
        snprintf(hex_str,hex_digits+1,"%0*x",hex_digits,twos_complement);
    }
    return hex_str;
}
/*Function to write in list named filename*/
void writelist(const char* filename)
{
    int i;
    char filepath[256];
    FILE *fptr;
    char *comp_str = NULL;
    snprintf(filepath, sizeof(filepath),"%s.lst",filename);
    fptr = fopen(filepath,"w");
    if(!fptr)
    {
        perror("Failed to open file");
        return ;
    }
    for(i=0 ;i< MAX_LINES; i++)
    {
        if(!source_code[i][0] && !machineCode[i][0])
        {
            continue;/*No more data*/
        }
        comp_str = dec_2comp(i,32); /*char *comp_str*/
        if(!comp_str)
        {
            fprintf(stderr,"Failed to compute two's complement for line %d\n",i);
            continue;
        }
        fprintf(fptr, "%s ",comp_str);
        free(comp_str);
        if(machineCode[i][0] && token_countM[i]>0)
        {
            fprintf(fptr,"%s ",machineCode[i][0]);
        }
        if(source_code[i][0] && token_count[i]>0)
        {
            int j;
            for(j = 0;j<token_count[i];j++)
            {
                fprintf(fptr,"%s ",source_code[i][j]);
            }
        }
        fprintf(fptr,"\n");
    }
    fclose(fptr);
}
/*End of file */
/*Code for writelog file*/
int compareStrings(const void *a,const void *b)
{
    char *str1 = *(char **)a;
    char *str2 = *(char **)b;
    return strcmp(str1,str2);
}
void sortUsedLabels(StringVector *vector)
{
    qsort(vector->data, vector->size, sizeof(char*), compareStrings);
}
void writelog(const  char *filename, Map *labels,StringVector *used_labels,WarningList *warnings, Error **errors,int error_count){
    char log_file_name[256];
    int i=0;
    size_t unique_count=0;
    FILE *fptr = NULL;
    snprintf(log_file_name, sizeof(log_file_name),"%s.log",filename);
    fptr = fopen(log_file_name,"w");
    if(fptr == NULL){
        perror("Failed to open log file");
        return;
    }
    fprintf(fptr,"Labels        :       value\n");
    for(i=0;i<TABLE_SIZE;i++)
    {
        Node *current = labels->table[i];
        while(current)
        {
            fprintf(fptr,"%s        :       %d\n",current->key,current->value);
            current = current->next;
        }
    }
    fprintf(fptr,"\nUsed labels\n");
    sortUsedLabels(used_labels);
    /*sorting the used labels alphabetically*/
    unique_count = 0;
    for(i=0;i<(int)used_labels->size;i++)
    {
        if(strcmp(used_labels->data[unique_count],used_labels->data[i])!=0)
        {
            unique_count++;
            used_labels->data[unique_count]=used_labels->data[i];
        }
    }
    used_labels->size = unique_count + 1;
    for(i = 0;i <(int) used_labels->size; i++)
    {
        fprintf(fptr,"%s\n",used_labels->data[i]);
    }
    fprintf(fptr,"\nWarnings\n");
    for(i=0;i< warnings->size;i++)
    {
        fprintf(fptr,"Line %d: %s\n", warnings->warnings[i].line,warnings->warnings[i].message);
    }
    fprintf(fptr,"\nErrors\n");
    for(i=0;i<error_count;i++)
    {
        fprintf(fptr,"Line %d: %s\n", errors[i]->line_num, errors[i]->message);
    }
    fclose(fptr);
}
/*Function to write object file*/
void writeobj(const char *filename)
{
    FILE *obj_ptr;
    char output_file[256];
    int sz=0;
    int i=0;
    snprintf(output_file,sizeof(output_file),"%s.o",filename);
    /*opening the file for binary writing*/
    obj_ptr=fopen(output_file,"wb");
    if(obj_ptr == NULL)
    {
        perror("Failed to open file for writing");
        return;
    }
    /*counting the number of non empty lines in source code*/
    for(i=0;i<MAX_LINES;i++)
    {
        if(source_code[i][0] == NULL || token_count[i]== 0)
        {
            continue;
        }
        sz++;
    }
    /*write the integer machine code to the file*/
    fwrite(integer_machineCode,sizeof(int32_t),sz,obj_ptr);
    /*close the file*/
    fclose(obj_ptr);
}
/*End of function to write object file*/
/*------------------------------------------------------------------------------------------*/
void print_warn(WarningList *list)
{
    int i=0;
    printf("Warnings:\n");
    for(i=0; i< list->size;i++)
    {
        printf("Line %d: %s\n",list->warnings[i].line,list->warnings[i].message);
    }
    printf("\n");
}
/*--------------------------------------------------------------------------------------------*/
/*Function for error implementation*/
/*--------------------------------------------------------------------------------------------*/
void print_errors(Error **errorArray,int size)
{
    int i=0;
    printf("Errors : \n");
    for(i=0 ; i< size;i++)
    {
        printf("Line %d: %s\n",(*errorArray)[i].line_num,(*errorArray)[i].message);
    }
    printf("\n");
}
/*End of function*/
/*Function to implement insert function of string of std:: C++ library*/
void insertC(char *original, const char *toInsert, int pos)
{
    int origLen = strlen(original);
    int insertLen = strlen(toInsert);
    memmove(original+pos+insertLen, original+pos,origLen - pos + 1);
    memcpy(original + pos, toInsert, insertLen);
}
/*End of function insert */
int validop(char* s)
/*Function to determine whether the string is decimal, octal,hexadecimal number or an opcode*/
{
    size_t i;
    int f=3;
    if(validLabel(s)&&findM(&labels,s))
    {
        return 5;
    }
    else if(validLabel(s)&&!findM(&labels,s))
    {
        return 1;
    }
    for(i=0; i<strlen(s);i++)
    {
        if(!(s[i]>='0' && s[i]<='7'))
        {
            f=2;
        }
    }
    if(f==3)
    {
        return 3;
    }
    for(i=0;i<strlen(s);i++)
    {
        if(!(s[i]>='0' && s[i]<= '9'))
        {
            f=4;
        }
    }
    if(f==2)
    {
        return 2;
    }
    for(i=0; i<strlen(s); i++)
    {
        if(!((s[i]>='0' && s[i]<= '9') || (s[i]<= 'f' && s[i] >= 'a')||(s[i]<='F' && s[i] >= 'A')))
        {
            f=0;
        }
    }
    if(f==4)
    {
        return 4;
    }
    return f;
}
/*Function to add token to specific line */
void add_token(char *lines[MAX_LINES][MAX_TOKENS_PER_LINE],int line_index,const char *token,int *token_count)
{

    /*Allocating memory for the line if not already done*/
    if(line_index >= MAX_LINES || token_count[line_index] >= MAX_TOKENS_PER_LINE)
    {
        fprintf(stderr,"Error: Index out of bounds, Line index: %d Token Count: %d\n",line_index, token_count[line_index]);
        exit(1);
    }
    /*Allocating memory for the token*/
    lines[line_index][token_count[line_index]] =(char*) malloc(strlen(token)+1);
    if(lines[line_index][token_count[line_index]]==NULL)
    {
        perror("Failed to allocate memory for token");
        exit(0);
    }
    /*Copying the token to allocated space*/
    strcpy(lines[line_index][token_count[line_index]],token);
    token_count[line_index]++;
}
/*End of function */
/*Function find of std:: vector class in C*/
int find(const char *key)
{
    int i;
    for (i=0;i<data_size;i++){/*compare Keys*/
        if(strcmp(data[i].key,key)==0){ /*Compare Keys*/
            return i; /*Return the index if found*/
        }
    }
    return -1;/*Return -1 if not found*/
}
/*End of function*/
/*Function to mimick substring function of c++*/
char *substring(const char *source,int start,int end)
{
    int i;
    int length=0;
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
/*Function to mimick set<pair<int,string>> function.*/
/*Function to compare two errors for sorting by line number(ascending)*/
int compareErrors(const void *a, const void *b)
{
    Error *errorA = (Error *)a;
    Error *errorB = (Error *)b;
    /*First, compare by line number*/
    if(errorA -> line_num != errorB -> line_num)
    return errorA->line_num - errorB->line_num;
    /*If line numbers are same ,compare by error message alphabetically*/
    return strcmp(errorA->message, errorB->message);
}
/*Function to add an error to the array,checking for duplicates*/
int addError(Error **errorArray, int *size,int line_num, const char *message)
{
    /*Variable declaration start*/
    int i;
    Error *temp;
    /*Variable declaration end*/
    for (i=0 ;i<*size;i++)
    {
        if((*errorArray)[i].line_num == line_num && strcmp((*errorArray)[i].message, message) == 0)
        return 0; /*Duplicate found, do not insert*/
    }
    /*Allocate memory for a new error*/
    temp = realloc(*errorArray,(*size+1)*sizeof(Error));
    if(temp == NULL)
    {
        return -1; /*Memory Allocation failure*/
    }
    *errorArray = temp;
    /*Adding new error to the array*/
    /*(*errorArray)[*size]=malloc(sizeof(Error));*/
    if(*errorArray == NULL)
    {
        return -1;
    }
    (*errorArray)[*size].line_num = line_num ;
    (*errorArray)[*size].message = strdup(message); /*strdup(duplicate string) used to copy the error message*/
    if((*errorArray)[*size].message == NULL)
    {
        return -1;
    }
    (*size)++;
    /*sort the array by line number and message*/
    qsort(*errorArray, *size, sizeof(Error), compareErrors);
    return 1;
}
/*End of function*/
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
/*Function to free the memory allocated for errors*/
void freeErrors(Error **errorArray,int size){
    /*Variable declaration start*/
    int i;
    /*Variable declaration end*/
    for ( i=0 ; i < size; i++)
    {
        free(errorArray[i]->message); /*First free the messages string*/
        free(errorArray[i]); /* Free the error struct */
    }
    free(errorArray);  /*Free the error array itself*/
}
/*End of the function freeErrors*/
void initial()
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
    return;
}
bool validLabel(const char *s)
{
    /*variables declaration start*/
    int i;
    /*Variables declaration End*/
    if(!((s[0] >= 'a' && s[0] <= 'z')||(s[0] >= 'A' && s[0] <= 'Z') || (s[0] == '_')))
    {
        return false;
    }
    for(i = 1;i<(int)strlen(s);i++)
    {
        if(!((s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') || (s[i] == '_')))
        {
            return false;
        }
    }
    return true;
}
/*Code to implement map function */
unsigned int hash(const char* key)
{
    unsigned int hash=0 ;
    while(*key){
        hash=(hash << 5) + *key++;
    }
    return hash%TABLE_SIZE;
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
void insert(Map *map, const char* key,int value)
{
    unsigned int index = hash(key);
    Node *current = NULL;
    Node *newNode = malloc(sizeof(Node));
    if(!newNode)
    {
        perror("Failed to allocate memory for node");
        exit(0);
    }
        newNode->key = strdup(key);
        newNode->value = value;
        newNode->next = NULL;
    /*Checking for existing value of keys and updating if key exists*/
    current = map->table[index]; /*Node *current dec*/
    while(current)
    {
        if(strcmp(current->key,key)==0)
        {
            current->value = value;
            free(newNode->key);
            free(newNode);
            return ;
        }
        current= current->next;
    }
    /*Insert at the beginning of the list*/
    newNode -> next = map->table[index];
    map->table[index]=newNode;
}
/*Lookup a value by key*/
int findM(Map *map,const char *key)
{
    unsigned int index=hash(key);
    Node *current = map->table[index];
    while(current)
    {
        if(strcmp(current->key,key)==0)
        {
            return current->value;
        }
        current= current->next;
    }
    return -1;
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
            free(temp->key);
            free(temp);
        }
    }
}
/*End of the code to implement map function */
/*---------------------------------------------------------*/
/*Vector implementation function*/
void initStringVector(StringVector *vector)
{
    vector->size = 0;
    vector->capacity = 10;
    vector -> data = (char** ) malloc(vector->capacity *sizeof(char*));
    if(vector -> data == NULL)
    {
        perror("Failed to allocate memory");
        exit(1);
    }
}
void addString(StringVector *vector, const char *str)
{
    if(vector-> size == vector->capacity)
    {
        vector->capacity*=2 ;
        vector->data =(char **)realloc(vector->data,vector->capacity*sizeof(char*));
        if(vector->data == NULL)
        {
            perror("Failed to reallocate memory");
            exit(1);
        }
    }
    /*Allocating memory for new string and copying it*/
    vector->data[vector->size] = strdup(str);
    if(vector->data[vector->size] == NULL)
    {
        perror("Failed to duplicate string");
        exit(1);
    }
    vector->size ++ ;
}
void freeStringVector(StringVector *vector)
{
    size_t i;
    for(i=0;i<vector->size;i++)
    {
        free(vector->data[i]);
    }
    free(vector->data);
}
/*end of vector implementation*/
/*StringVector used_labels;
INSERT: addString(&used_labels, " label1");
to access any element used_labels.data[i];*/
/*------------------------------------------------*/
/*Function to check if a key exists in a string list*/
int find_in_vector(StringVector *vector,const char* key)
{
    size_t i = 0;
    for(i=0 ; i<vector->size; i++)
    {
        if(strcmp(vector->data[i],key)==0)
        {
            return 1;
        }
    }
    return 0;
}
/*End of function */
/*Initialize warning lists*/
void initWarnings(WarningList *list)
{
    list->size=0;
    list->capacity=10;
    list->warnings = malloc(list->capacity * sizeof(Warning));
}
/*Adding a warning to warning list*/
void addWarning(WarningList *list,int line ,const char* message)
{
    if(list->size == list->capacity)
    {
        list->capacity *= 2;
        list-> warnings = realloc(list->warnings , list->capacity * sizeof(Warning));
    }
    list->warnings[list->size].line=line;
    list->warnings[list->size].message = strdup(message); /*copying the message*/
    list->size ++ ;
}
/*Free the memory after use*/
void freeWarnings(WarningList *list)
{
    int i;
    for(i=0 ;i<list->size;i++)
    {
        free(list->warnings[i].message);
    }
    free(list->warnings);
}
/*End of function for warning implementation*/
/*TO Find the array length of a string */
size_t get_array_length(char **array)
{
    size_t count = 0;
    while(array != NULL && array[count]!=NULL)
    {
        count++;
    }
    return count;
}
/*End of function*/
void trim(char *str)
{
    char *start= str;
    char *end;
    /*Remove leading whitespace*/
    while(*start && (isspace((unsigned char)* start) || !isprint((unsigned char)*start)))
    {
        start++;
    }
    if(*start == '\0'){
        str[0]= '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while ( end > start && (isspace((unsigned char)* end)||!isprint((unsigned char)*end)))
    {
        end--;
    }
    *(end + 1) = '\0';
    if(start != str)
    {
        memmove(str, start, end - start + 2);
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