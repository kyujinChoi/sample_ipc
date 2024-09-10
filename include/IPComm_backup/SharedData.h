
#ifndef _IPC_DATA_H_
#define _IPC_DATA_H_


#include <sys/ipc.h>
#include <sys/shm.h>
#include "Parameter/Parameter.h"
#include <semaphore.h>
#include <fcntl.h>
#include <memory>
#define MAX_IPC_BUF 320000
// writer cares semaphores. create them when initializing
#pragma pack(push, 1)
typedef struct SharedData
{
    typedef struct Header
    {
        int clients = 0;
        int reader_cnt = 0;
    } header_t;
    typedef struct Body
    {
        int size = 0;
        void *msg;
    } body_t;
    header_t *header;
    body_t *body;
    SharedData()
    {
        void *block = malloc(sizeof(header_t) + sizeof(int) + MAX_IPC_BUF);
        if (block == nullptr) 
        {
            std::cerr << "Failed to allocate memory for SharedData!" << std::endl;
            exit(EXIT_FAILURE);
        }
        memset(block, 0, sizeof(header_t) + sizeof(int) + MAX_IPC_BUF);
        header = (header_t *)block;
        body = (body_t *)((char*)block + sizeof(header_t));
        body->msg = (char*)(body + sizeof(int));
    }
    // SharedData(int data_size_)
    // {
    //     header = (header_t *)malloc(sizeof(header_t));
    //     if (header == nullptr) 
    //     {
    //         std::cerr << "Failed to allocate memory for header!" << std::endl;
    //         exit(EXIT_FAILURE);
    //     }
    //     memset(header, 0, sizeof(header_t));  // 초기화
    //     body = nullptr;  // body는 필요시 초기화
    //     resizeBody(data_size_);
    // }
    ~SharedData()
    {
        if (header != nullptr) {
            free(header);
        }
        if (body != nullptr) {
            free(body);
        }
    }
    // void resizeBody(int data_size_)
    // {
    //     header->body_size = data_size_;
    //     if (body != nullptr) 
    //     {
    //         free(body);  // 기존에 할당된 메모리 해제
    //     }
    //     body = malloc(header->body_size);
    //     if (body == nullptr) 
    //     {
    //         std::cerr << "Failed to allocate memory for body!" << std::endl;
    //         exit(EXIT_FAILURE);
    //     }
    //     memset(body, 0, header->body_size);

    // }
} shData_t;
#pragma pack(pop)
#endif