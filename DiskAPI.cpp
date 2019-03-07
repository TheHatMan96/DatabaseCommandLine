    #include "DiskAPI.h"
    #include <stdlib.h>
    #include <cstring>
    #include <math.h>

#define BLOCK_SIZE 4096
#define SIZE_DIR 0
#define TABLES_TABLE_DIR 4
#define NUMBER_OF_TABLES 8
#define NUMBER_OF_BLOCKS 12

using namespace std;

char disk_buffer[BLOCK_SIZE];

char* get_database_to_use(){
    std::fstream fs("database_to_use", std::ios::in);
    string file_name;
    getline(fs, file_name);
    char* database_name = new char[file_name.size() + 1];
    copy(file_name.begin(), file_name.end(), database_name);
    return database_name;
    
}

char* get_values(char* command){
    if(command == NULL){
        return NULL;
    }
    char* comm;
    comm = strtok(command, "=");
    comm = strtok(NULL, "=");
    return comm;
}

void clean_buffer(char* buff){
    for(int c = 0; c < BLOCK_SIZE; c++){
        buff[c] = 0;
    }
}

void toggle_bit(int pos, char* name){
    read_block(name, 1, disk_buffer);
    int pos_block = pos / 8;
    int bit_in_char = pos % 8;
    char byte_to_edit = disk_buffer[pos_block];
    byte_to_edit ^= 1UL << bit_in_char;
    memcpy(&disk_buffer[pos_block], &byte_to_edit, 1);
    write_block(name, 1, disk_buffer);
}

int find_next_empty_block(char* name){
    read_block(name, 0, disk_buffer);
    int number_of_blocks;
    memcpy(&number_of_blocks, &disk_buffer[NUMBER_OF_BLOCKS], 4);
    read_block(name, 1, disk_buffer);
    for(int c = 0; c < number_of_blocks / 8; c++){
        char byte_to_check = disk_buffer[c];
        for(int a = 0; a < 8; a++){
            int check = 1 << a;
            if((byte_to_check & check) == 0){
                return c * 8 + a;
            }
        }
    }
    return 0;
}

int get_size_in_bytes(int initial_size, char* multiplier){
    if(!strcmp(multiplier, "KB")){
        return initial_size * 1024;
    }else if(!strcmp(multiplier, "MB")){
        return initial_size * 1024 * 1024;
    }else if(!strcmp(multiplier, "GB")){
        return initial_size * 1024 * 1024 * 1024;
    }
    return initial_size;
}

int create_database(int size, char* name){
    std::ofstream ofs(name, std::ios::binary | std::ios::out);
    ofs.seekp(size - BLOCK_SIZE, ios_base::beg);
    ofs.write("", 1);
    ofs.close();
}

void format_database(int size, char* name){
    int number_of_blocks = size / BLOCK_SIZE;
    float blocks_for_bitmap = ceil(float(number_of_blocks) / float(BLOCK_SIZE));
    blocks_for_bitmap += 1;
    int number_of_tables = 0;
    clean_buffer(disk_buffer);
    memcpy(&disk_buffer[SIZE_DIR], &size, 4);
    memcpy(&disk_buffer[TABLES_TABLE_DIR], &blocks_for_bitmap, 4);
    memcpy(&disk_buffer[NUMBER_OF_TABLES], &number_of_tables, 4);
    memcpy(&disk_buffer[NUMBER_OF_BLOCKS], &number_of_blocks, 4);
    write_block(name, 0, disk_buffer);
    blocks_for_bitmap -= 1;
    toggle_bit(0, name);
    toggle_bit(1, name);
    toggle_bit(2, name);
}

void delete_database(char* name){
    remove(name);
    cout << "The Datbase " << name <<  " has been eliminated" << endl;
}

void read_block(char* name, int block_number, char* buffer){
    clean_buffer(buffer);
    std::fstream fs (name, std::ios::in | std::ios::out);
    fs.seekp(BLOCK_SIZE * block_number, ios_base::beg);
    fs.read(buffer, BLOCK_SIZE);
    fs.close();
}

void write_block(char* name, int block_number, char* buffer){
    std::fstream fs (name, std::ios::in | std::ios::out);
    fs.seekp(BLOCK_SIZE * block_number, ios_base::beg);
    fs.write(buffer, BLOCK_SIZE);
    fs.close();
}