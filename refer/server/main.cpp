#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>

#define KEY_NUM 1234
#define MEM_SIZE 4096

using namespace std;

int shmid;
static int SharedMemoryCreate();
static int SharedMemoryWrite(std::string shareddata, int size);
static int SharedMemoryRead(char *sMemory);
static int SharedMemoryFree(void);

int main(int argc, char *argv[])
{
    std::string buffer = "test data\n";

    SharedMemoryCreate();
    sleep(5);

    SharedMemoryWrite(buffer, MEM_SIZE);
    SharedMemoryFree();
    return 0;
}

static int SharedMemoryCreate()
{
    if ((shmid = shmget((key_t)KEY_NUM, MEM_SIZE, IPC_CREAT | IPC_EXCL | 0666)) == -1)
    {
        printf("There was shared memory.");

        shmid = shmget((key_t)KEY_NUM, MEM_SIZE, IPC_CREAT | 0666);

        if (shmid == -1)
        {
            perror("Shared memory create fail");
            return 1;
        }
        else
        {
            SharedMemoryFree();
            shmid = shmget((key_t)KEY_NUM, MEM_SIZE, IPC_CREAT | 0666);

            if (shmid == -1)
            {
                perror("Shared memory create fail");
                return 1;
            }
        }
    }

    return 0;
}

static int SharedMemoryWrite(std::string shareddata, int size)
{
    void *shmaddr;
    if (size > MEM_SIZE)
    {
        printf("Shared memory size over");
        return 1;
    }

    if ((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    {
        perror("Shmat failed");
        return 1;
    }

    memcpy((char *)shmaddr, shareddata.c_str(), size);
    std::cout << "write : " << shareddata << std::endl;
    if (shmdt(shmaddr) == -1)
    {
        perror("Shmdt failed");
        return 1;
    }
    return 0;
}

static int SharedMemoryRead(char *sMemory)
{
    void *shmaddr;
    char mess[MEM_SIZE] = {0};

    if ((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    {
        perror("Shmat failed");
        return 1;
    }

    memcpy(sMemory, (char *)shmaddr, sizeof(mess));

    if (shmdt(shmaddr) == -1)
    {
        perror("Shmdt failed");
        return 1;
    }
    return 0;
}

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