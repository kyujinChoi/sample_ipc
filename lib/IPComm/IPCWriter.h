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
    // shData_t *getSharedData()
    // {
    //     return sh_data;
    // }
    int readHeader();
    int writeHeader();
    int writeBody(void *data, int size);
    int writeBody();
    void updateClients();
    int Free();
    void init_sem(void *pthis);
    void start_write_sem();
    void end_write_sem();
    void free_sem(void *pthis);

public:
    sem_t *ticket_mtx, *write_mtx, *cnt_mtx;
    std::vector<sem_t *> vec_update_mtx;
    std::vector<int> vec_client_idx;
    Parameter param;
    umsg::sample msg;
private:
    char *data_addr;
    int shmid;

    char* pkt;
    size_t pkt_size;
    google::protobuf::io::ArrayOutputStream* aos;
    google::protobuf::io::CodedOutputStream* coded_output;
};
#endif