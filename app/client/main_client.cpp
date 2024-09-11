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
    std::string buffer = "class test data";
    int cnt = 0;
    ipc = new IPCReader(1234);
    shData_t *recv_msg;
    // umsg::sample send_msg;
    while(1)
    {
        // ipc->ReadHeader(sh_data->header);
        usleep(100*1000);
        recv_msg = ipc->ReadBody();
        std::cout << "-------------------------\n";
        std::cout << "clients : " << recv_msg->body->_clients() << std::endl;
        std::cout << "reader_cnt : " << recv_msg->body->_reader_cnt() << std::endl;
        std::cout << "msg : " << recv_msg->body->_msg() << std::endl;

        if(cnt > 100000)
            break;
    }
    ipc->Free();
    return 0;
}