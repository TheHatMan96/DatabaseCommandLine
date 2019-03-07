#include <stdio.h>
#include <iostream>
#include <fstream>
#include "DiskAPI.h"

using namespace std;

void get_register(char* name, int register_size, int block_count, int next_block, char* register_temp, int a);

bool check_cond(char* reg, struct Column column, char* cond);

int find_table(char* name, char* table_name);