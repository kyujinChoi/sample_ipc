#include "IPCReader.h"
IPCReader::IPCReader(int key)
{
    param.insertParam("key", key);
    param.insertParam("sem_ticket_mtx", "/ipc_ticket_mtx");
    param.insertParam("sem_write_mtx", "/ipc_write_mtx");
    param.insertParam("sem_update_mtx", "/ipc_update_mtx");
    param.insertParam("sem_cnt_mtx", "/ipc_cnt_mtx");

    siz = 14;
    pkt = new char[siz];

    init_sem(this);
    Init();
}
IPCReader::~IPCReader() { Free(); }
int IPCReader::Init()
{
    void *shmaddr;

    if ((shmid = shmget((key_t)param.getParamInt("key"), 0, 0)) == -1)
        perror("Shmid failed");

    data_addr = (char *)shmat(shmid, NULL, 0);
    if (data_addr == (char *)-1)
    {
        printf("Shared Memory Buffer Create Failed...\n");
        return -1;
    }
    // readerEnter();

    // sh_data->resizeBody(sh_data->header->body_size);

    return 0;
}
int IPCReader::readHeader()
{
    // sem_wait(ticket_mtx);
    // memcpy((char *)sh_data->header, (char *)data_addr, sizeof(SharedData::Header));
    // sem_post(ticket_mtx);

    return 0;
}
int IPCReader::writeHeader()
{
    // sem_wait(ticket_mtx);
    // memcpy((char *)data_addr, (void *)sh_data->header, sizeof(SharedData::Header));
    // sem_post(ticket_mtx);

    return 0;
}
void IPCReader::readerEnter()
{
    readHeader();
    // ++sh_data->header->clients;
    writeHeader();
    // param.insertParam("client_id", sh_data->header->clients - 1);
    static sem_t *st_update_mtx = SEM_FAILED;
    while (st_update_mtx == SEM_FAILED)
    {
        usleep(10);
        std::string id_str = param.getParamStr("sem_update_mtx") + param.getParamStr("client_id");
        st_update_mtx = sem_open(id_str.c_str(), 0);
    }

    update_mtx = st_update_mtx;
    return;
}

void IPCReader::readerExit()
{
    std::string id_str = param.getParamStr("sem_update_mtx") + param.getParamStr("client_id");
    sem_close(update_mtx);
    sem_unlink(id_str.c_str());

    return;
}

int IPCReader::Free()
{
    readerExit();
    printf("Shared memory end\n");

    return 0;
}
void IPCReader::ReadBody()
{
    // wait_for_update();
    // start_cnt_sem();
    memcpy(pkt, data_addr, siz);
    google::protobuf::io::ArrayInputStream ais(pkt, 14);
    CodedInputStream coded_input(&ais);
    google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(14);
    msg.ParseFromCodedStream(&coded_input);
    coded_input.PopLimit(msgLimit);
    // std::cout << msg._clients() << std::endl;
    // std::cout << msg._reader_cnt() << std::endl;
    // std::cout << msg._msg() << std::endl;
    // Print the size stored in sh_data->body
    // memcpy(sh_data->body->msg, data_addr + sizeof(SharedData::Header) + sizeof(int), sh_data->body->size);
    // memcpy(sh_data->body->msg, data_addr + sizeof(SharedData::Header)+ sizeof(int), sh_data->body->size);
    // std::cout << "data_addr + sizeof(SharedData::Header) : " << data_addr + sizeof(SharedData::Header) << std::endl;

    // end_cnt_sem();

    return;
}
void IPCReader::init_sem(void *pthis)
{
    static sem_t *st_write_mtx = nullptr;
    static sem_t *st_cnt_mtx = nullptr;
    static sem_t *st_ticket_mtx = nullptr;

    IPCReader *user = (IPCReader *)pthis;

    st_ticket_mtx = sem_open(user->param.getParamStr("sem_ticket_mtx").c_str(), 0);
    st_write_mtx = sem_open(user->param.getParamStr("sem_write_mtx").c_str(), 0);
    st_cnt_mtx = sem_open(user->param.getParamStr("sem_cnt_mtx").c_str(), 0);
    if (st_ticket_mtx == SEM_FAILED || st_write_mtx == SEM_FAILED || st_cnt_mtx == SEM_FAILED)
    {
        perror("init_sem sem_open failed");
        std::cout << "maybe there is no writer\n";
        exit(-1);
    }
    user->ticket_mtx = st_ticket_mtx;
    user->write_mtx = st_write_mtx;
    user->cnt_mtx = st_cnt_mtx;
    return;
}
void IPCReader::wait_for_update()
{
    sem_wait(update_mtx);
}
void IPCReader::start_cnt_sem()
{
    int cnt;
    sem_wait(cnt_mtx);
    readHeader();
    // cnt = ++sh_data->header->reader_cnt;
    writeHeader();
    sem_post(cnt_mtx);
    if (cnt == 1)
        sem_wait(write_mtx);

    return;
}
void IPCReader::end_cnt_sem()
{
    int cnt;
    sem_wait(cnt_mtx);
    readHeader();
    // cnt = --sh_data->header->reader_cnt;
    writeHeader();
    sem_post(cnt_mtx);
    if (cnt == 0)
        sem_post(write_mtx);

    return;
}
void IPCReader::start_write_sem()
{
    if (sem_wait(write_mtx) == -1)
        perror("sem_wait() error");

    return;
}
void IPCReader::end_write_sem()
{
    sem_post(write_mtx);
    return;
}