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
    void ReadBody();
    void init_sem(void *pthis);
    void wait_for_update();
    void start_cnt_sem();
    void end_cnt_sem();
    void start_write_sem();
    void end_write_sem();

public:
    sem_t *ticket_mtx, *write_mtx, *cnt_mtx, *update_mtx;
    Parameter param;    

    umsg::sample msg;
private:
    char *data_addr;
    int shmid;

    // CodedInputStream *coded_input;
    char *pkt;
    int siz;
    // google::protobuf::io::CodedInputStream::Limit msgLimit;
    // shData_t *sh_data;
};
#endif