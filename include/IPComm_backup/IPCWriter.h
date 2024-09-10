#ifndef _IPC_WRITER_H_
#define _IPC_WRITER_H_
#include "SharedData.h"
class IPCWriter
{
public:
    IPCWriter(int key)
    {
        param.insertParam("key", key);
        param.insertParam("sem_ticket_mtx", "/ipc_ticket_mtx");
        param.insertParam("sem_write_mtx", "/ipc_write_mtx");
        param.insertParam("sem_update_mtx", "/ipc_update_mtx");
        param.insertParam("sem_cnt_mtx", "/ipc_cnt_mtx");

        sh_data = new shData_t();
        init_sem(this);
        Init();
    }
    ~IPCWriter() { Free(); };
    int Init()
    {
        if ((shmid = shmget((key_t)param.getParamInt("key"), sizeof(SharedData::Header) + sizeof(int) + MAX_IPC_BUF, IPC_CREAT | IPC_EXCL | 0666)) == -1)
        {
            char bufferKey[128];
            sprintf(bufferKey, "0x%08x", param.getParamInt("key"));
            std::string key0 = bufferKey;
            std::array<char, 128> buffer;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("ipcs -m", "r"), pclose);
            if (!pipe)
                perror("popen failed");
            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            {
                std::string str = buffer.data();
                int pos = str.find(key0);
                int pos1 = -1;

                if (pos >= 0)
                {
                    pos1 = str.find("  ", pos + key0.size());
                    shmid = std::stol(str.substr(pos + key0.size(), pos1));
                    break;
                }
            }
            if (shmid)
            {
                if (shmctl(shmid, IPC_RMID, 0) == -1)
                    perror("Shmctl failed");
            }

            shmid = shmget((key_t)param.getParamInt("key"), sizeof(SharedData::Header) + sizeof(int) + MAX_IPC_BUF, IPC_CREAT | 0666);
            if (-1 == shmid)
            {
                printf("IpcData Create error\n");
                return -1;
            }
        }
        data_addr = (char *)shmat(shmid, NULL, 0);
        if (data_addr == (char *)-1)
        {
            printf("Shared Memory Buffer Create Failed...\n");
            return -1;
        }
        writeHeader();
        return 0;
    }
    shData_t *getSharedData()
    {
        return sh_data;
    }
    int readHeader()
    {
        sem_wait(ticket_mtx);
        memcpy((char *)sh_data->header, (char *)data_addr, sizeof(SharedData::Header));
        sem_post(ticket_mtx);

        return 0;
    }
    int writeHeader()
    {
        sem_wait(ticket_mtx);
        memcpy((char *)data_addr, (char *)sh_data->header, sizeof(SharedData::Header));
        sem_post(ticket_mtx);

        return 0;
    }
    int writeBody(void *data, int size)
    {
        // size_t data_len = strlen((char *)data) + 1;
        
        // int msg_size = size;
        // std::cout << "size : " << size << std::endl;
        // std::cout << "data : " << (char*)data << std::endl;
        int len = size;
        updateClients();

        start_write_sem();
        memcpy(data_addr+ sizeof(SharedData::Header), &len, sizeof(int));
        memcpy(data_addr+ sizeof(SharedData::Header) + sizeof(int), (void *)data, size);
        end_write_sem();
        return 0;
    }   
    // int writeBody()
    // {
    //     size_t data_len = strlen((char *)sh_data->body) + 1;
    //     updateClients();

    //     start_write_sem();
    //     memcpy(data_addr+ sizeof(SharedData::Header), (void *)sh_data->body, data_len);
    //     end_write_sem();

    //     return 0;
    // }
    void updateClients()
    {
        static int prev_clients = 0;
        readHeader();
        int cur_clients = sh_data->header->clients;

        std::vector<int> erase_idx;
        for (int i = 0; i < vec_client_idx.size(); i++)
        {
            std::string sem_name = param.getParamStr("sem_update_mtx") + std::to_string(vec_client_idx[i]);
            vec_update_mtx[i] = sem_open(sem_name.c_str(), 0);
            if (vec_update_mtx[i] == SEM_FAILED)
            {
                if (errno == ENOENT)
                {
                    coconut::coconut.cout("Client Leaved!\n", coconut::Color::YELLOW, coconut::Style::BOLD);
                    erase_idx.push_back(i);
                }
            }
        }
        for (int i = 0; i < erase_idx.size(); i++)
        {
            vec_update_mtx.erase(vec_update_mtx.begin() + erase_idx[i]);
            vec_client_idx.erase(vec_client_idx.begin() + erase_idx[i]);
        }

        for (int i = prev_clients; i < cur_clients; i++)
        {
            static sem_t *sem = nullptr;
            std::string sem_name = param.getParamStr("sem_update_mtx") + std::to_string(i);
            sem = sem_open(sem_name.c_str(), O_CREAT, 0644, 1);
            if (sem == SEM_FAILED)
            {
                perror("sem_open failed");
                exit(-1);
            }
            coconut::coconut.cout("Client Entered!\n", coconut::Color::GREEN, coconut::Style::BOLD);
            vec_update_mtx.push_back(sem);
            vec_client_idx.push_back(i);
        }
        prev_clients = cur_clients;

        return;
    }
    int Free()
    {
        free_sem(this);
        if (shmctl(shmid, IPC_RMID, 0) == -1)
            perror("Shmctl failed");

        printf("Shared memory end\n");

        return 0;
    }

    void init_sem(void *pthis)
    {
        static sem_t *st_write_mtx = nullptr;
        static sem_t *st_cnt_mtx = nullptr;
        static sem_t *st_ticket_mtx = nullptr;

        IPCWriter *user = (IPCWriter *)pthis;

        st_ticket_mtx = sem_open(user->param.getParamStr("sem_ticket_mtx").c_str(), O_CREAT, 0644, 1);
        st_write_mtx = sem_open(user->param.getParamStr("sem_write_mtx").c_str(), O_CREAT, 0644, 1);
        st_cnt_mtx = sem_open(user->param.getParamStr("sem_cnt_mtx").c_str(), O_CREAT, 0644, 1);
        if (st_ticket_mtx == SEM_FAILED || st_write_mtx == SEM_FAILED || st_cnt_mtx == SEM_FAILED)
        {
            perror("init_sem sem_open failed");
            exit(-1);
        }
        user->ticket_mtx = st_ticket_mtx;
        user->write_mtx = st_write_mtx;
        user->cnt_mtx = st_cnt_mtx;
        return;
    }

    void start_write_sem()
    {
        if (sem_wait(write_mtx) == -1)
            perror("sem_wait() error");

        return;
    }
    void end_write_sem()
    {
        sem_post(write_mtx);
        for (int i = 0; i < vec_update_mtx.size(); i++)
        {
            sem_post(vec_update_mtx[i]);
        }
        return;
    }
    void free_sem(void *pthis)
    {
        sem_close(write_mtx);
        sem_unlink(param.getParamStr("sem_write_mtx").c_str());

        for (int i = 0; i < vec_update_mtx.size(); i++)
        {
            std::string id_str = param.getParamStr("sem_update_mtx") + std::to_string(i);
            sem_close(vec_update_mtx[i]);
            sem_unlink(id_str.c_str());
        }

        sem_close(cnt_mtx);
        sem_unlink(param.getParamStr("sem_cnt_mtx").c_str());

        return;
    }

public:
    sem_t *ticket_mtx, *write_mtx, *cnt_mtx;
    std::vector<sem_t *> vec_update_mtx;
    std::vector<int> vec_client_idx;
    Parameter param;

private:
    char *data_addr;
    int shmid;
    shData_t *sh_data;
};
#endif