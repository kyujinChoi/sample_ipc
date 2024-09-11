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
    // umsg::sample send_msg;
    shData_t *sh_data;
    SharedData send_msg;
    while(1)
    {
        usleep(100*1000);
        // ipc->ReadHeader(sh_data->header);
        sh_data = ipc->getSharedData();
        std::string buf = buffer + std::to_string(cnt++);
        // send_msg.body->type = cnt * 10;
        std::cout << "-----------------------\n";
        send_msg.body->set__clients(cnt);
        send_msg.body->set__reader_cnt(cnt + 1);
        send_msg.body->set__msg(buf);
        // send_msg.body->size = send_msg.body->msg->ByteSizeLong();

        ipc->writeBody(send_msg.body);
        std::cout << "clients : " << sh_data->body->_clients() << std::endl;
        std::cout << "reader_cnt : " << sh_data->body->_reader_cnt() << std::endl;
        std::cout << "msg : " << sh_data->body->_msg() << std::endl;
        
        if(cnt > 100000)
            break;
    }
    ipc->Free();
    return 0;
}