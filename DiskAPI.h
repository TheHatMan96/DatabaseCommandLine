#include <stdio.h>
#include <iostream>
#include <fstream>

#pragma once

using namespace std;

char* get_database_to_use();

char* get_values(char* command);

int get_size_in_bytes(int initial_size, char* multiplier);

int create_database(int size, char* name);

void format_database(int size, char* name);

void delete_database(char* name);

int find_next_empty_block(char* name);

void clean_buffer(char* buff);

void toggle_bit(int pos, char* name);

void read_block(char* name, int block_number, char* buffer);

void write_block(char* name, int block_number, char* buffer);

struct Column{
    bool used;
    char column_name[50];
    char pos_in_register;
    int size;
    char type;
    char primary_key;
};

struct TableInfo{
    bool used;
    char table_name[50];
    int metadata_block;
};
