#include "IPComm/IPCWriter.h"
#include <signal.h>

IPCWriter* ipc;

void sig_handler(int signo)
{
    ipc->Free();
    std::cout <<"signal: " << signo << std::endl;
    exit(0);
}
int main()
{
    signal(SIGINT, sig_handler); // Catch interrupt signal
    std::string buffer = "class test data";
    int cnt = 0;
    ipc = new IPCWriter(1234);
    shData_t *sh_data;

    while(1)
    {
        usleep(100*1000);
        sh_data = ipc->getSharedData();
        buffer = "class test data" + std::to_string(cnt++);
        // ipc->ReadHeader(sh_data->header);
        
        ipc->writeBody((void*)buffer.c_str(), sizeof(buffer));
        std::cout << sh_data->header->clients << std::endl;
        std::cout << sh_data->header->reader_cnt << std::endl;
        std::cout << sizeof(buffer) << std::endl;
        std::cout << buffer << std::endl << std::endl;
        if(cnt > 100000)
            break;
    }
    ipc->Free();
    return 0;
}