#ifndef _IPC_WRITER_H_
#define _IPC_WRITER_H_
// #include "umsg.pb.h"

#include "SharedData.h"
class IPCWriter
{
public:
    IPCWriter(int key);
    ~IPCWriter();
    int Init();
    shData_t *getSharedData();
    int readHeader();
    int writeHeader();
    int writeBody(SharedData::Body *body, umsg::sample msg);
    // int writeBody();
    void updateClients();
    int Free();
    void init_sem(void *pthis);
    void start_write_sem();
    void end_write_sem();
    void free_sem(void *pthis);
    void serialize();

public:
    sem_t *ticket_mtx, *write_mtx, *cnt_mtx;
    std::vector<sem_t *> vec_update_mtx;
    std::vector<int> vec_client_idx;
    Parameter param;
    
private:
    char *data_addr;
    int shmid;

    shData_t *sh_data;
    char* pkt;
    size_t pkt_size;
    // google::protobuf::io::ArrayOutputStream* aos;
    // google::protobuf::io::CodedOutputStream* coded_output;
};
#endif