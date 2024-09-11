
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
typedef struct SharedData
{
    typedef struct Header
    {
        int clients;
        int reader_cnt;
    }header_t;
    typedef struct Body
    {
        int type;
        int size;
    } body_t;
    header_t *header;
    body_t *body;
    umsg::sample msg;
    SharedData()
    {
        void *block = malloc(sizeof(header_t) + 2 * sizeof(int) + MAX_IPC_BUF);
        if (block == nullptr) 
        {
            std::cerr << "Failed to allocate memory for SharedData!" << std::endl;
            exit(EXIT_FAILURE);
        }
        memset(block, 0, sizeof(header_t) + 2 * sizeof(int) + MAX_IPC_BUF);
        header = (header_t *)block;
        body = (body_t *)((char*)block + sizeof(header_t));
        // static umsg::sample *tmp = new umsg::sample();
        // body->msg = tmp;
        // std::memcpy(body->msg, tmp, sizeof(umsg::sample));
    }
    
    ~SharedData()
    {
        if (header != nullptr) 
            free(header);
        if (body != nullptr) 
            free(body);
    }
} shData_t;
#pragma pack(pop)

#endif