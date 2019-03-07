#include <stdio.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include "DBAPI.h"

using namespace std;

char database_to_use[50];

int main(int argc, char **argv){
    char *option = new char[500];
    string opt;
    do{
        system("clear");
        cout <<"\t-----------DATABASE COMMAND LINE------------\n";
        cout <<"1.Create Database: create database dbname.db size type\n";
        cout <<"-------- type=kb,mb,gb\n";
        cout <<"2.Drop Database: drop database dbname.db\n";
        cout <<"3.Enter to database: use database dbname.db\n";
        cout <<"4.Create Table: create table table_name -columns=col1,col2,col3 -type=int,double,char(50) -key=col1\n";
        cout <<"5.Drop Table: drop table table_name\n";
        cout <<"6.Show Tables: show tables //only if it is entered in the database\n";
        cout <<"7.INSERT: insert table_name -columns=col1,col2,col3 -values=1,2.4,Daniel\n";
        cout <<"8.UPDATE: update table_name -columns=col1,col2,col3 -values=1,2.4,Daniel -where=col1<>1\n";
        cout <<"9.DELETE: delete table_name -where=col1<>1\n";
        cout <<"10.SELECT: select -table=table_name -columns=col1,col2,col3\n";
        cout <<"\nEnter > ";
        cin.getline(option, 500, '\n');
        char* op1 = strtok(option, " ");
        char* op2 = strtok(NULL, " ");
        char* op3 = strtok(NULL, " ");
        char* op4 = strtok(NULL, " ");
        char* op5 = strtok(NULL, " ");
        char* op6 = strtok(NULL, " ");
        if(!strcmp(op1, "create") && !strcmp(op2, "database")){
            if(atoi(op4) > 512){
                cout << "Tamano maximo de DB es de 512 MB" << endl;
            }else{
                int size_in_bytes = get_size_in_bytes(atoi(op4), op5);
                create_database(size_in_bytes, op3);
                format_database(size_in_bytes, op3);
                cout << "The database has been created" <<endl;
            }
        }
        else if(!strcmp(op1, "drop") && !strcmp(op2, "database"))
        {
            delete_database(op3);
            
        }
        else if(!strcmp(op1, "use") && !strcmp(op2, "database"))
        {
            strcpy(database_to_use, op3);
            cout<< "You have enter to the database"<<endl;
        }
        else if(!strcmp(op1, "create") && !strcmp(op2, "table"))
        {
            create_table(database_to_use, op3, get_values(op4), get_values(op5), get_values(op6));
            cout << "You have created a new table"<<endl;
        }
        else if(!strcmp(op1, "drop") && !strcmp(op2, "table"))
        {
            drop_table(database_to_use, op3);
            cout<< "You have deleted a table"<<endl;
        }
        else if(!strcmp(op1, "show") && !strcmp(op2, "tables"))
        {
            show_tables(database_to_use);
        }
        else if(!strcmp(op1, "insert"))
        {
            insert_register(database_to_use, op2, get_values(op3), get_values(op4));
            cout<< "Register has been inserted"<<endl;
        }
        else if(!strcmp(op1, "select"))
        {
            select_show(database_to_use, get_values(op2), get_values(op3), get_values(op4));
        }
        else if(!strcmp(op1, "update"))
        {
            update_register(database_to_use, op2, get_values(op3), get_values(op4), get_values(op5));
            cout<< "The values has been updated"<<endl;
        }
        else if(!strcmp(op1, "close"))
        {
            delete_register(database_to_use, op2, get_values(op3));
            cout<<"The values has been deleted"<<endl;
        }
        else if(!strcmp(op1, "exit"))
        {
            break;
        }
        getchar();
        strcpy(option, " ");
    }while(true);
}