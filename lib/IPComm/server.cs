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
        // std::string buf = buffer + std::to_string(cnt++);
        // send_msg.body->type = cnt * 10;
        std::cout << "-----------------------\n";
        sh_data->body->clear_points();
        for(int i = 0 ; i < 9; i++)
        {
            cnt++;
            umsg::Point* point = sh_data->body->add_points(); 
            point->set_x(cnt + 0.123);
            point->set_y(cnt + 0.456);
            point->set_z(cnt + 0.789);
            std::cout << "i = " << i << std::endl;
            std::cout << "x : " << sh_data->body->points(i).x() << std::endl;
            std::cout << "y : " << sh_data->body->points(i).y() << std::endl;
            std::cout << "z : " << sh_data->body->points(i).z() << std::endl;
        }
        // send_msg.body->size = send_msg.body->msg->ByteSizeLong();

        ipc->writeBody();
        const umsg::Point& p = sh_data->body->points(0);  // points의 첫 번째 요소 가져오기
        float x = p.x();  // x 값 가져오기
        float y = p.y();  // y 값 가져오기
        float z = p.z();  // z 값 가져오기
        
        if(cnt > 100000)
            break;
    }
    ipc->Free();
    return 0;
}