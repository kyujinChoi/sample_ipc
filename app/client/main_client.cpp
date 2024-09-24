#include "IPComm/IPCReader.h"
#include <signal.h>
#include "Util/time.h"
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
    int timer_fd = init_timerfd(100);
    while(1)
    {
        // ipc->ReadHeader(sh_data->header);
        recv_msg = ipc->ReadBody();
        std::cout << "-------------------------?\n";
        if(recv_msg->type == SharedData::POINTCLOUD)
        {
            static unsigned int msg_cnt = 0;
            if(msg_cnt == recv_msg->cnt)
                continue;
            std::cout << ((umsg::PointCloud *)recv_msg->body)->points_size() << std::endl;
            for(int i = 0; i < ((umsg::PointCloud *)recv_msg->body)->points_size();i++)
            {
                const umsg::PointXYZIRL &p = ((umsg::PointCloud *)recv_msg->body)->points(i); // points의 첫 번째 요소 가져오기
                float x = p.x();                                  // x 값 가져오기
                float y = p.y();                                  // y 값 가져오기
                float z = p.z();                                  // z 값 가져오기
                std::cout << "x : " << x << std::endl;
                std::cout << "y : " << y << std::endl;
                std::cout << "z : " << z << std::endl;
            }
            msg_cnt = recv_msg->cnt;
        }
        else if(recv_msg->type == SharedData::LOG_EVENT)
        {
            static unsigned int msg_cnt = 0;
            if(msg_cnt == recv_msg->cnt)
                continue;
            std::cout << recv_msg->cnt << "'s message = " << ((umsg::LogEvent *)recv_msg->body)->obsol() << std::endl;
            msg_cnt = recv_msg->cnt;
        }

        // std::cout << "clients : " << recv_msg->body->x() << std::endl;
        // std::cout << "reader_cnt : " << recv_msg->body->y() << std::endl;
        // std::cout << "msg : " << recv_msg->body->z() << std::endl;
        wait_timerfd(timer_fd);
    }
    ipc->Free();
    return 0;
}