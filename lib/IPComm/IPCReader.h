#ifndef _IPC_READER_H_
#define _IPC_READER_H_
#include "SharedData.h"
class IPCReader
{
public:
    IPCReader(int key);

    ~IPCReader();
    int Init();
    int readHeader();
    int writeHeader();
    void readerEnter();
    void readerExit();
    int Free();
    shData_t *ReadBody();
    void init_sem(void *pthis);
    void wait_for_update();
    void start_cnt_sem();
    void end_cnt_sem();
    void start_write_sem();
    void end_write_sem();
    void deserialize();
public:
    sem_t *ticket_mtx, *write_mtx, *cnt_mtx, *update_mtx;
    Parameter param;    
private:
    char *data_addr;
    int shmid;
    shData_t *sh_data;
    // CodedInputStream *coded_input;
    char *pkt;
    int pkt_size;
    // google::protobuf::io::ArrayInputStream* ais;
    // google::protobuf::io::CodedInputStream* coded_input;
    // google::protobuf::io::CodedInputStream::Limit msgLimit;
    // shData_t *sh_data;
};
#endif