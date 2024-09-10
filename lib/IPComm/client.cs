#include "IPComm/IPCReader.h"
#include <signal.h>
IPCReader* ipc;

void sig_handler(int signo)
{
    ipc->Free();
    std::cout <<"signal: " << signo << std::endl;
    exit(0);
}

int main()
{
    signal(SIGINT, sig_handler); // Catch interrupt signal
    std::string buffer="abc";
    ipc = new IPCReader(1234);
    shData_t *sh_data;
    // ipc->Init();
    
    while(1)
    {
        // usleep(100*1000);
        sh_data = ipc->ReadBody();

        // if(buffer == "class test data")
        {
            std::cout << "Receive data from shared memory!" << std::endl;
            std::cout << sh_data->header->clients << std::endl;
            std::cout << sh_data->body->size << std::endl;
            std::cout << (char *)sh_data->body->msg << std::endl << std::endl;
        }   
        
    }
    
    ipc->Free();
    return 0;
}