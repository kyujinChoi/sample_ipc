
#ifndef _IPC_DATA_H_
#define _IPC_DATA_H_


#include <sys/ipc.h>
#include <sys/shm.h>
#include "Parameter/Parameter.h"
#include <semaphore.h>
#include <fcntl.h>
#include <memory>
#include "umsg.pb.h"

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#define MAX_IPC_BUF 320000

using namespace google::protobuf::io;
// writer cares semaphores. create them when initializing
#pragma pack(push, 1)
typedef struct SharedStatus
{
    int* clients;
    int* reader_cnt;
    SharedStatus()
    {
        void *block = malloc(sizeof(int) * 2);
        if (block == nullptr) 
        {
            std::cerr << "Failed to allocate memory for SharedData!" << std::endl;
            exit(EXIT_FAILURE);
        }
        memset(block, 0, sizeof(int) * 2);
        clients = (int *)block;
        reader_cnt = (int *)((char*)block + sizeof(int));
    }
    ~SharedStatus()
    {
        if (clients != nullptr) 
            free(clients);
        if (reader_cnt != nullptr) 
            free(reader_cnt);
    }
} sh_status_t;
#pragma pack(pop)

#endif