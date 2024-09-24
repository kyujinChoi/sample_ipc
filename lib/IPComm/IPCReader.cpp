#include "IPCReader.h"
IPCReader::IPCReader(int key)
{
    param.insertParam("key", key);
    param.insertParam("sem_ticket_mtx", "/ipc_ticket_mtx");
    param.insertParam("sem_write_mtx", "/ipc_write_mtx");
    param.insertParam("sem_update_mtx", "/ipc_update_mtx");
    param.insertParam("sem_cnt_mtx", "/ipc_cnt_mtx");

    for(int i = 0 ; i < SharedData::MAX_NUM; i++)
    {
        shData_t *sh_data = new shData_t(i);
        vec_sh_data.push_back(sh_data);
    }
    sh_header = new shData_t();
    pkt_size = 3 * sizeof(int);
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
    memcpy((char *)sh_header->header, (char *)data_addr, sizeof(SharedData::Header));
    sem_post(ticket_mtx);

    return 0;
}
int IPCReader::writeHeader()
{
    sem_wait(ticket_mtx);
    memcpy((char *)data_addr, (void *)sh_header->header, sizeof(SharedData::Header));
    sem_post(ticket_mtx);

    return 0;
}
void IPCReader::readerEnter()
{
    readHeader();
    ++sh_header->header->clients;
    writeHeader();
    param.insertParam("client_id", sh_header->header->clients - 1);
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
    char buf[20];
    google::protobuf::uint32 hdr[3];

    wait_for_update();
    start_cnt_sem();
    
    memcpy(buf, data_addr + sizeof(SharedData::Header), sizeof(buf));
    google::protobuf::io::ArrayInputStream ais(buf, sizeof(buf));
    CodedInputStream coded_input(&ais);
    coded_input.ReadVarint32(&hdr[0]);
    coded_input.ReadVarint32(&hdr[1]);
    coded_input.ReadVarint32(&hdr[2]);
    // std::cout << "hdr[0]: " << hdr[0] << std::endl;
    // std::cout << "hdr[1]: " << hdr[1] << std::endl;
    // std::cout << "hdr[2]: " << hdr[2] << std::endl;
    // std::cout << "coded_input.CurrentPosition(): " << coded_input.CurrentPosition() << std::endl;
    // Print the size stored in sh_data->body
    if (hdr[1] > pkt_size) 
    {
        delete[] pkt;  // 기존 버퍼 삭제
        pkt_size = hdr[1];  // 새로운 크기로 설정
        pkt = new char[pkt_size];  // 새로운 버퍼 할당
    }
    memcpy(pkt, data_addr + sizeof(SharedData::Header) + coded_input.CurrentPosition() , hdr[1]);
    deserialize(hdr);
    // memcpy(sh_data->body->msg, data_addr + sizeof(SharedData::Header)+ sizeof(int), sh_data->body->size);
    // std::cout << "data_addr + sizeof(SharedData::Header) : " << data_addr + sizeof(SharedData::Header) << std::endl;
    
    end_cnt_sem();
    
    return vec_sh_data[hdr[0]];
}
void IPCReader::deserialize(google::protobuf::uint32 *hdr)
{
    google::protobuf::io::ArrayInputStream ais(pkt, hdr[1]);
    CodedInputStream coded_input(&ais);
    google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(hdr[1]);
    if (hdr[0] == SharedData::POINTCLOUD)
        ((umsg::PointCloud *)vec_sh_data[hdr[0]]->body)->ParseFromCodedStream(&coded_input);
    else if (hdr[0] == SharedData::LOG_EVENT)
        ((umsg::LogEvent *)vec_sh_data[hdr[0]]->body)->ParseFromCodedStream(&coded_input);
    
    vec_sh_data[hdr[0]]->cnt = hdr[2];
    coded_input.PopLimit(msgLimit);
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
    cnt = ++sh_header->header->reader_cnt;
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
    cnt = --sh_header->header->reader_cnt;
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