#ifndef _IPC_WRITER_H_
#define _IPC_WRITER_H_
#include "SharedData.h"

class IPCWriter
{
public:
    IPCWriter(int key);
    ~IPCWriter();
    int Init();
    int readHeader();
    int writeHeader();
    int writeBody(unsigned int type, shData_t *send_msg);
    void updateClients();
    int Free();
    void init_sem(void *pthis);
    void start_write_sem();
    void end_write_sem();
    void free_sem(void *pthis);
    void serialize(unsigned int type, shData_t *send_msg);

public:
    sem_t *ticket_mtx, *write_mtx, *cnt_mtx;
    std::vector<sem_t *> vec_update_mtx;
    std::vector<int> vec_client_idx;
    Parameter param;
    
private:
    char *data_addr;
    int shmid;

    shData_t *sh_header;
    char* pkt;
    size_t pkt_size;
};
#endif