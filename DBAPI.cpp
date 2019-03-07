#include "DBAPI.h"
#include <stdlib.h>
#include <cstring>
#include <math.h>

#define BLOCK_SIZE 4096
#define SIZE_DIR 0
#define TABLES_TABLE_DIR 4
#define NUMBER_OF_TABLES 8
#define NUMBER_OF_BLOCKS 12

using namespace std;

char db_buffer[BLOCK_SIZE];

void show_tables(char* name){
    read_block(name, 2, db_buffer);

    struct TableInfo *tables = (struct TableInfo*)malloc(sizeof(struct TableInfo));
    int pos = 0;
    while(pos * sizeof(struct TableInfo) < BLOCK_SIZE){
        memcpy(tables, &db_buffer[pos*sizeof(struct TableInfo)], sizeof(struct TableInfo));
        if(tables->used){
            cout << tables->table_name << endl;
        }
        pos++;
    }
}

void create_table(char* name, char* table_name, char* columns, char* types, char* key){
    if(find_table(name, table_name) != -1){
        cout << "Table already exist" << endl;
        return;
    }

    struct Column *columns_info[15];
    char *column, *type;
    column = strtok(columns, ",");

    int block_for_table_metadata = find_next_empty_block(name);
    toggle_bit(block_for_table_metadata, name);

    int start_of_data = find_next_empty_block(name);
    toggle_bit(start_of_data, name);
    
    int c = 0;
    while(column != NULL){
        columns_info[c] = (struct Column*)malloc(sizeof(struct Column));
        strcpy(&columns_info[c]->column_name[0], column);
        column = strtok(NULL, ",");
        c++;
    }

    clean_buffer(db_buffer);
    int size_of_register = 0;
    type = strtok(types, ",");
    for(int a=0; a < c; a++){
        if(!strcmp(type, "int")){
            columns_info[a]->type = 'i';
            int column_size = 4;
            memcpy(&columns_info[a]->pos_in_register, &size_of_register, 4);
            size_of_register += 4;
            memcpy(&columns_info[a]->size, &column_size, 4);
        }else if(!strcmp(type, "double")){
            columns_info[a]->type = 'd';
            int column_size = 8;
            memcpy(&columns_info[a]->pos_in_register, &size_of_register, 4);
            size_of_register += 8;
            memcpy(&columns_info[a]->size, &column_size, 4);
        }else{
            columns_info[a]->type = 'c';
            char* temp;
            temp = strtok(type, "(");
            temp = strtok(NULL, ")");
            int column_size = atoi(temp);
            memcpy(&columns_info[a]->pos_in_register, &size_of_register, 4);
            size_of_register += atoi(temp);
            memcpy(&columns_info[a]->size, &column_size, 4);
        }
        if(!strcmp(columns_info[a]->column_name, key)){
            columns_info[a]->primary_key = 1;
        }else{
            columns_info[a]->primary_key = 0;
        }
        type = strtok(NULL, ",");
        columns_info[a]->used = 1;
        memcpy(&db_buffer[8 + sizeof(struct Column) * a], columns_info[a], sizeof(struct Column));
    }

    memcpy(&db_buffer[0], &size_of_register, 4);
    size_of_register = 0;
    memcpy(&db_buffer[4], &size_of_register, 4);

    memcpy(&db_buffer[4092], &start_of_data, 4);

    write_block(name, block_for_table_metadata, db_buffer);
    read_block(name, 2, db_buffer);

    struct TableInfo *tables = (struct TableInfo*)malloc(sizeof(struct TableInfo));
    int pos = 0;
    memcpy(tables, &db_buffer, sizeof(struct TableInfo));
    pos++;
    while(tables->used){
        memcpy(tables, &db_buffer[pos*sizeof(struct TableInfo)], sizeof(struct TableInfo));
        pos++;
    }
    pos--;
    tables = (struct TableInfo*)malloc(sizeof(struct TableInfo));
    tables->used = 1;
    strcpy(tables->table_name, table_name);
    tables->metadata_block = block_for_table_metadata;
    memcpy(&db_buffer[pos*sizeof(struct TableInfo)], tables, sizeof(struct TableInfo));

    write_block(name, 2, db_buffer);
}

void drop_table(char* name, char* table){
    int metadata_block = find_table(name, table);
    
    if(metadata_block == -1){
        cout << "Table does not exist" << endl;
        return;
    }

    read_block(name, 2, db_buffer);
    struct TableInfo *tables = (struct TableInfo*)malloc(sizeof(struct TableInfo));
    int c = 0;
    memcpy(tables, &db_buffer[c * sizeof(struct TableInfo)], sizeof(struct TableInfo));
    while(tables->used){
        if(!strcmp(tables->table_name, table)){
            break;
        }
        c++;
        memcpy(tables, &db_buffer[c * sizeof(struct TableInfo)], sizeof(struct TableInfo));
    }

    memset(tables, 0, sizeof(struct TableInfo));
    delete_register(name, table, NULL);
    read_block(name, 2, db_buffer);
    memcpy(&db_buffer[c * sizeof(struct TableInfo)], tables, sizeof(struct TableInfo));
    write_block(name, 2, db_buffer);
    
    int next_block = tables->metadata_block;
    while(next_block != 0){
        toggle_bit(next_block, name);
        char temp_buff[4096];
        read_block(name, next_block, temp_buff);
        memcpy(&next_block, &temp_buff[4092], 4);
    }
}

void insert_register(char* name, char* table_name, char* columns, char* values){
    int metadata_block = find_table(name, table_name);

    if(metadata_block == -1){
        cout << "Table does not exist" << endl;
        return;
    }

    read_block(name, metadata_block, db_buffer);

    int register_size;
    int number_of_registers;

    memcpy(&register_size, &db_buffer[0], 4);
    memcpy(&number_of_registers, &db_buffer[4], 4);

    char register_to_add[register_size];

    for(int a = 0; a < register_size; a++){
        register_to_add[a] = 0;
    }

    char* column;
    column = strtok(columns, ",");
    int c = 0;
    struct Column columns1[15];
    while(column != NULL){
        memcpy(&columns1[c], &db_buffer[8 + c * sizeof(struct Column)], sizeof(struct Column));
        bool found = false;
        while(columns1[c].used){
            if(!strcmp(columns1[c].column_name, column)){
                found = true;
                break;
            }
            c++;
            memcpy(&columns1[c], &db_buffer[8 + c * sizeof(struct Column)], sizeof(struct Column));
        }
        if(!found){
            cout << "Column " << column << " does not exist in this table." << endl;
            return;
        }
        column = strtok(NULL, ",");
    }

    char* value;
    value = strtok(values, ",");
    for(int a = 0; a <= c; a++){
        if(columns1[a].type == 'i'){
            int val = atoi(value);
            memcpy(&register_to_add[columns1[a].pos_in_register], &val, 4);
        }else if(columns1[a].type == 'c'){
            string temp_str = value;
            if(temp_str.size() <= columns1[a].size){
                strcpy(&register_to_add[columns1[a].pos_in_register], value);
            }else{
                cout << "Char size is too long" << endl;
                return;
            }
        }else if(columns1[a].type == 'd'){
            double val = atof(value);
            memcpy(&register_to_add[columns1[a].pos_in_register], &val, 8);
        }else{
            cout << "You cant insert on the table" << endl;
            return;
        }
        value = strtok(NULL, ",");
    }

    int pos = register_size * number_of_registers;
    int relative_block = pos / 4092;
    int pos_in_block = pos % 4092;
    int next_block;
    memcpy(&next_block, &db_buffer[4092], 4);
    read_block(name, next_block, db_buffer);
    int real_block;
    for(int x = 0; x < relative_block; x++){
        memcpy(&next_block, &db_buffer[4092], 4);
        read_block(name, next_block, db_buffer);
        real_block = x;
    }

    if((pos_in_block + register_size) > 4092){
        int size_in_this_block = 4092 - pos_in_block;
        memcpy(&db_buffer[pos_in_block], register_to_add, size_in_this_block);
        char* buff_temp[BLOCK_SIZE];
        memcpy(buff_temp, db_buffer, BLOCK_SIZE);
        int next_block = find_next_empty_block(name);
        toggle_bit(next_block, name);
        clean_buffer(db_buffer);
        memcpy(db_buffer, buff_temp, BLOCK_SIZE);
        memcpy(&db_buffer[4092], &next_block, next_block);
        memcpy(&db_buffer[pos_in_block], register_to_add, size_in_this_block);
        write_block(name, real_block, db_buffer);
        clean_buffer(db_buffer);
        memcpy(db_buffer, &register_to_add[register_size - size_in_this_block], register_size - size_in_this_block);
        write_block(name, next_block, db_buffer);
    }else{
        memcpy(&db_buffer[pos_in_block], register_to_add, register_size);
        write_block(name, next_block, db_buffer);
    }

    number_of_registers++;
    read_block(name, metadata_block, db_buffer);
    memcpy(&db_buffer[4], &number_of_registers, 4);
    write_block(name, metadata_block, db_buffer);
}

void select_show(char* name, char* table, char* columns, char* where){
    int metadata_block = find_table(name, table);

    if(metadata_block == -1){
        cout << "Table does not exist" << endl;
        return;
    }

    char* column_with_cond;
    char* cond;

    if(where != NULL){
        column_with_cond = strtok(where, "<>");
        //cout << column_with_cond << endl;
        cond = strtok(NULL, "<>");
    }else{
        column_with_cond = NULL;
        cond = NULL;
    }

    read_block(name, metadata_block, db_buffer);
    
    int register_size;
    int number_of_registers;
    int next_block;

    memcpy(&register_size, &db_buffer[0], 4);
    memcpy(&number_of_registers, &db_buffer[4], 4);
    memcpy(&next_block, &db_buffer[4092], 4);

    char register_to_add[register_size];

    char* column;
    column = strtok(columns, ",");
    int c = 0;
    int y = 0;
    struct Column columns1[15];
    struct Column col_with_cond;

    string column_names;

    while(column != NULL){
        struct Column temp;
        memcpy(&temp, &db_buffer[8 + y * sizeof(struct Column)], sizeof(struct Column));
        bool found = false;
        while(temp.used){
            if(!strcmp(temp.column_name, column)){
                memcpy(&columns1[c], &temp, sizeof(struct Column));
                column_names += column;
                found = true;
                c++;
                if(where != NULL && !strcmp(temp.column_name, column_with_cond)){
                    memcpy(&col_with_cond, &temp, sizeof(struct Column));
                }
                break;
            }
            if(where != NULL && !strcmp(temp.column_name, column_with_cond)){
                memcpy(&col_with_cond, &temp, sizeof(struct Column));
                found = true;
            }
            y++;
            memcpy(&temp, &db_buffer[8 + y * sizeof(struct Column)], sizeof(struct Column));
        }
        if(!found && column != NULL){
            cout << "Column " << column << " does not exist in this table." << endl;
            return;
        }
        column = strtok(NULL, ",");
        if(column != NULL){
            column_names += " - ";
        }
    }

    cout << column_names << endl;

    read_block(name, next_block, db_buffer);
    int block_count = 0;

    string reg;

    for(int a = 0; a < number_of_registers; a++){
        char register_temp[register_size];
        get_register(name, register_size, block_count, next_block, register_temp, a);

        if(check_cond(register_temp, col_with_cond, cond)){
            for(int x = 0; x < c; x++){
                if(columns1[x].type == 'i'){
                    int temp;
                    memcpy(&temp, &register_temp[columns1[x].pos_in_register], 4);
                    reg += to_string(temp);
                }else if(columns1[x].type == 'c'){
                    char temp[50];
                    strcpy(temp, &register_temp[columns1[x].pos_in_register]);
                    reg += temp;
                }else if(columns1[x].type == 'd'){
                    double temp;
                    memcpy(&temp, &register_temp[columns1[x].pos_in_register], 8);
                    reg += to_string(temp);
                }
                if(x != c - 1){
                    reg += " - ";
                }
            }
            cout << reg << endl;
            reg = "";
        }
        block_count += register_size;
    }
}

void update_register(char* name, char* table, char* columns, char* values, char* where){
    int metadata_block = find_table(name, table);
    
    if(metadata_block == -1){
        cout << "Table does not exist" << endl;
        return;
    }

    char* column_with_cond;
    char* cond;

    if(where != NULL){
        column_with_cond = strtok(where, "<>");
        cond = strtok(NULL, "<>");
    }
    else{
        column_with_cond = NULL;
        cond = NULL;
    }

    read_block(name, metadata_block, db_buffer);
    
    int register_size;
    int number_of_registers;
    int next_block;

    memcpy(&register_size, &db_buffer[0], 4);
    memcpy(&number_of_registers, &db_buffer[4], 4);
    memcpy(&next_block, &db_buffer[4092], 4);

    char register_to_add[register_size];

    char* column;
    column = strtok(columns, ",");
    int c = 0;
    int y = 0;
    struct Column columns1[15];
    struct Column col_with_cond;
    
    while(column != NULL){
        struct Column temp;
        memcpy(&temp, &db_buffer[8 + y * sizeof(struct Column)], sizeof(struct Column));
        bool found = false;
        while(temp.used){
            if(!strcmp(temp.column_name, column)){
                memcpy(&columns1[c], &temp, sizeof(struct Column));
                found = true;
                c++;
                if(where != NULL && !strcmp(temp.column_name, column_with_cond)){
                    memcpy(&col_with_cond, &temp, sizeof(struct Column));
                }
                break;
            }
            if(where != NULL && !strcmp(temp.column_name, column_with_cond)){
                memcpy(&col_with_cond, &temp, sizeof(struct Column));
                found = true;
            }
            y++;
            memcpy(&temp, &db_buffer[8 + y * sizeof(struct Column)], sizeof(struct Column));
        }
        if(!found && column != NULL){
            cout << "Column " << column << " does not exist on the table." << endl;
            return;
        }
        column = strtok(NULL, ",");
    }

    read_block(name, next_block, db_buffer);
    int block_count = 0;


    for(int a = 0; a < number_of_registers; a++){
        char register_temp[register_size];
        int old_block = next_block;
        int this_block = 4092 - block_count;
        get_register(name, register_size, block_count, next_block, register_temp, a);

        if(check_cond(register_temp, col_with_cond, cond)){
            char* value;
            value = strtok(values, ",");
            for(int x = 0; x < c; x++){
                if(columns1[x].type == 'i'){
                    int temp = atoi(value);
                    memcpy(&register_temp[columns1[x].pos_in_register], &temp, 4);
                }else if(columns1[x].type == 'c'){
                    char temp[50];
                    strcpy(temp, value);
                    strcpy(&register_temp[columns1[x].pos_in_register], temp);
                }else if(columns1[x].type == 'd'){
                    double temp = atof(value);
                    memcpy(&register_temp[columns1[x].pos_in_register], &temp, 8);
                }
                value = strtok(NULL, ",");
            }
            
            if(this_block < register_size){
                memcpy(&db_buffer[a * register_size], register_temp, this_block);
                write_block(name, this_block, db_buffer);
                read_block(name, next_block, db_buffer);
                memcpy(db_buffer, &register_temp[block_count], register_size - this_block);
                write_block(name, next_block, db_buffer);
            }else{
                memcpy(&db_buffer[a * register_size], register_temp, register_size);
                write_block(name, next_block, db_buffer);
            }
        }
        block_count += register_size;
    }
}

void delete_register(char* name, char* table, char* where){
    int metadata_block = find_table(name, table);
    
    if(metadata_block == -1){
        cout << "Table does not exist" << endl;
        return;
    }

    char* column_with_cond;
    char* cond;

    if(where != NULL){
        column_with_cond = strtok(where, "<>");
        cond = strtok(NULL, "<>");
    }else{
        column_with_cond = NULL;
        cond = NULL;
    }

    read_block(name, metadata_block, db_buffer);
    
    int register_size;
    int number_of_registers;
    int next_block;

    memcpy(&register_size, &db_buffer[0], 4);
    memcpy(&number_of_registers, &db_buffer[4], 4);
    memcpy(&next_block, &db_buffer[4092], 4);

    char register_to_del[register_size];

    int y = 0;
    struct Column col_with_cond;

    struct Column temp;
    memcpy(&temp, &db_buffer[8 + y * sizeof(struct Column)], sizeof(struct Column));
    bool found = false;
    while(temp.used){
        if(where != NULL && !strcmp(temp.column_name, column_with_cond)){
            memcpy(&col_with_cond, &temp, sizeof(struct Column));
            found = true;
            break;
        }
        y++;
        memcpy(&temp, &db_buffer[8 + y * sizeof(struct Column)], sizeof(struct Column));
    }
    if(!found && column_with_cond != NULL){
        cout << "Column " << column_with_cond << " does not exist on the table." << endl;
        return;
    }

    read_block(name, next_block, db_buffer);
    int block_count = 0;

    for(int a = 0; a < number_of_registers; a++){
        char register_temp[register_size];
        int old_block = next_block;
        int this_block = 4092 - block_count;
        get_register(name, register_size, block_count, next_block, register_temp, a);

        if(check_cond(register_temp, col_with_cond, cond)){
            memset(register_temp, 0, register_size);
            
            if(this_block < register_size){
                memcpy(&db_buffer[a * register_size], register_temp, this_block);
                write_block(name, this_block, db_buffer);
                read_block(name, next_block, db_buffer);
                memcpy(db_buffer, &register_temp[block_count], register_size - this_block);
                write_block(name, next_block, db_buffer);
            }else{
                memcpy(&db_buffer[a * register_size], register_temp, register_size);
                write_block(name, next_block, db_buffer);
            }
        }
        block_count += register_size;
    }

}