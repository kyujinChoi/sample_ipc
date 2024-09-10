//////////////////////////////////////
//    Second Process Shared Memory    
////////////////////////////////////////
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#define  KEY_NUM   1234
#define  MEM_SIZE  4096
using namespace std;
int shmid;
static int SharedMemoryInit();
static int SharedMemoryWrite(char *sMemory, int size);
static int SharedMemoryRead(std::string& sMemory);
void *m_pIpcPosition;
static int SharedMemoryFree(void)
{
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        perror("Shmctl failed");
        return 1;
    }

    printf("Shared memory end\n");
    return 0;
}
int main(int argc, char *argv[])
{
    std::string buffer="abc";
    SharedMemoryInit();
    
    while(1)
    {
        SharedMemoryRead(buffer);
        if(buffer == "test data\n")
        {
            cout << "Receive data from shared memory!" << endl;
            std::cout << buffer << endl;
            break;
        }    
    }
    SharedMemoryFree();
    return 0;
}
static int SharedMemoryInit()
{
    void *shmaddr;
    
    if((shmid = shmget((key_t)KEY_NUM, 0, 0)) == -1)
    {
        perror("Shmid failed");
    }
    m_pIpcPosition = shmat(shmid, NULL, 0);
    if (m_pIpcPosition == (void *)-1)
    {
        printf("Shared Memory Buffer Create Failed...\n");
        return -1;
    }
    return 0;
}
static int SharedMemoryWrite(char *sMemory, int size)
{
    void *shmaddr;
    
    // if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    // {
    //     perror("Shmat failed");
    // }
    
    memcpy((char *)shmaddr, sMemory, size);
    
    if(shmdt(shmaddr) == -1)
    {
        perror("Shmdt failed");
        exit(1);
    }
    return 0;
}
static int SharedMemoryRead(std::string& sMemory)
{
    void *shmaddr;
    char msg[MEM_SIZE] = {0};
    // if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    // {
    //     perror("Shmat failed");
    // }
    
    // memcpy((void*)sMemory.c_str(), (char *)shmaddr, sizeof(sMemory));
    memcpy(msg, (char *)m_pIpcPosition, MEM_SIZE);
    // if(shmdt(shmaddr) == -1)
    // {
    //     perror("Shmdt failed");
    // }
    sMemory.assign(msg);
    return 0;
}