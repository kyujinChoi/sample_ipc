#include "IPCReader.h"
IPCReader::IPCReader(int key)
{
    param.insertParam("key", key);
    param.insertParam("sem_ticket_mtx", "/ipc_ticket_mtx");
    param.insertParam("sem_write_mtx", "/ipc_write_mtx");
    param.insertParam("sem_update_mtx", "/ipc_update_mtx");
    param.insertParam("sem_cnt_mtx", "/ipc_cnt_mtx");

    sh_data = new shData_t();
    pkt_size = 2 * sizeof(int);
    pkt = new char[pkt_size];
    // ais = new google::protobuf::io::ArrayInputStream(pkt, 2 * sizeof(int));
    // coded_input = new google::protobuf::io::CodedInputStream(ais);
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
    readerEnter();

    // sh_data->resizeBody(sh_data->header->body_size);

    return 0;
}
int IPCReader::readHeader()
{
    sem_wait(ticket_mtx);
    memcpy((char *)sh_data->header, (char *)data_addr, sizeof(SharedData::Header));
    sem_post(ticket_mtx);

    return 0;
}
int IPCReader::writeHeader()
{
    sem_wait(ticket_mtx);
    memcpy((char *)data_addr, (void *)sh_data->header, sizeof(SharedData::Header));
    sem_post(ticket_mtx);

    return 0;
}
void IPCReader::readerEnter()
{
    readHeader();
    ++sh_data->header->clients;
    writeHeader();
    param.insertParam("client_id", sh_data->header->clients - 1);
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
shData_t *IPCReader::ReadBody()
{
    wait_for_update();
    start_cnt_sem();
    // google::protobuf::uint32 hdr[2];
    memcpy(sh_data->body, data_addr + sizeof(SharedData::Header), 2 * sizeof(int));
    if (sh_data->body->size > pkt_size) 
    {
        // 만약 메시지가 최대 크기를 초과하면 새로운 크기만큼 다시 할당
        delete[] pkt;
        pkt = new char[sh_data->body->size];
        pkt_size = sh_data->body->size;
    }
    
    // Print the size stored in sh_data->body
    
    memcpy(pkt, data_addr + sizeof(SharedData::Header) + 2 * sizeof(int), sh_data->body->size);
    deserialize();
    // memcpy(sh_data->body->msg, data_addr + sizeof(SharedData::Header)+ sizeof(int), sh_data->body->size);
    // std::cout << "data_addr + sizeof(SharedData::Header) : " << data_addr + sizeof(SharedData::Header) << std::endl;
    
    end_cnt_sem();
    
    return sh_data;
}
void IPCReader::deserialize()
{
    umsg::sample *sample_msg = new umsg::sample;
    google::protobuf::io::ArrayInputStream ais(pkt, sh_data->body->size);
    CodedInputStream coded_input(&ais);
    google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(sh_data->body->size);
    sample_msg->ParseFromCodedStream(&coded_input);
    coded_input.PopLimit(msgLimit);
    sh_data->msg = *sample_msg;
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
    cnt = ++sh_data->header->reader_cnt;
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
    cnt = --sh_data->header->reader_cnt;
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