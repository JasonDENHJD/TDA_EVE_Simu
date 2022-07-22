//
// Created by 刘天骁 on 2022/7/13.
//

#include "file_io.h"
#include <iostream>
#include <algorithm>

using namespace std;

void Read_File_Exist_Buffer(char *file_name, void *buffer, uint64_t length)
{
    FILE *fp;
    if ((fp = fopen(file_name, "rb")) == NULL) {
        cout << "Fail to open file!" << endl;
        exit(0);
    }
    if (length <= 0)
    {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        rewind(fp);
    }

//    auto *raw_data = (int16_t *) malloc(sizeof(int16_t) * length);
    fread(buffer, length, 1, fp);
    fclose(fp);

}

void *Read_File_New_Buffer(char *file_name, uint64_t length)
{
    FILE *fp;
    if ((fp = fopen(file_name, "rb")) == NULL) {
        cout << "Fail to open file!" << endl;
        exit(0);
    }
    if (length <= 0)
    {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        rewind(fp);
    }

    void *data = malloc(sizeof(int16_t) * length);
    fread(data, length, 1, fp);
    fclose(fp);

    return data;
}