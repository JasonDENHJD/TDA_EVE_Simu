//
// Created by 刘天骁 on 2022/7/13.
//

#ifndef TDA2_EVE_SIMULATION_REMOTE_FILE_IO_H
#define TDA2_EVE_SIMULATION_REMOTE_FILE_IO_H

#include "stdint.h"

void Read_File_Exist_Buffer(char *file_name, void *buffer, uint64_t length);
void *Read_File_New_Buffer(char *file_name, uint64_t length);

#endif //TDA2_EVE_SIMULATION_REMOTE_FILE_IO_H
