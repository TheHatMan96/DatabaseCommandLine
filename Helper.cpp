#include <stdlib.h>
#include <cstring>
#include <math.h>
#include "Helper.h"

#define BLOCK_SIZE 4096
#define SIZE_DIR 0
#define TABLES_TABLE_DIR 4
#define NUMBER_OF_TABLES 8
#define NUMBER_OF_BLOCKS 12

using namespace std;

char helper_buffer[BLOCK_SIZE];

void get_register(char* name, int register_size, int block_count, int next_block, char* register_temp, int a){
    read_block(name, next_block, helper_buffer);
    if(block_count + register_size > 4092){
        int this_block = 4092 - block_count;
        memcpy(register_temp, &helper_buffer[a * register_size], this_block);
        memcpy(&next_block, &helper_buffer[4092], 4);
        read_block(name, next_block, helper_buffer);
        memcpy(&register_temp[block_count], helper_buffer, register_size - this_block);
    }else{
        if(block_count == 4092){
            memcpy(&next_block, &helper_buffer[4092], 4);
            read_block(name, next_block, helper_buffer);
        }
        memcpy(register_temp, &helper_buffer[a * register_size], register_size);
    } 
}

bool check_cond(char* reg, struct Column column, char* cond){
    if((int)reg[0] == 0){
        return false;
    }

    if(cond == NULL){
        return true;
    }
    if(column.type == 'i'){
        int temp;
        memcpy(&temp, &reg[column.pos_in_register], 4);
        if(temp == atoi(cond)){
            return true;
        }
    }else if(column.type == 'd'){
        double temp;
        memcpy(&temp, &reg[column.pos_in_register], 8);
        if(temp == atof(cond)){
            return true;
        }
    }else if(column.type == 'c'){
        return !strcmp(&reg[column.pos_in_register], cond);
    }
    return false;
}

int find_table(char* name, char* table_name){
    read_block(name, 2, helper_buffer);
    struct TableInfo *tables = (struct TableInfo*)malloc(sizeof(struct TableInfo));
    int c = 0;
    memcpy(tables, &helper_buffer[c * sizeof(struct TableInfo)], sizeof(struct TableInfo));
    while(tables->used){
        if(!strcmp(tables->table_name, table_name)){
            return tables->metadata_block;
        }
        c++;
        memcpy(tables, &helper_buffer[c * sizeof(struct TableInfo)], sizeof(struct TableInfo));
    }

    return -1;
}