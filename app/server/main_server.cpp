#include "IPComm/IPCWriter.h"
#include <signal.h>
#include "Util/time.h"
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
    SharedData send_msg(SharedData::POINTCLOUD);
    int timer_fd = init_timerfd(100);
    while(1)
    {
        // ipc->ReadHeader(sh_data->header);
        sh_data = ipc->getSharedData();
        // std::string buf = buffer + std::to_string(cnt++);
        // send_msg.body->type = cnt * 10;
        std::cout << "-----------------------\n";
        ((umsg::PointCloud *)sh_data->body)->clear_points();
        for(int i = 0 ; i < 9; i++)
        {
            cnt++;
            umsg::PointXYZIRL* point = ((umsg::PointCloud *)sh_data->body)->add_points(); 
            point->set_x(cnt + 0.123);
            point->set_y(cnt + 0.456);
            point->set_z(cnt + 0.789);
            std::cout << "i = " << i << std::endl;
            std::cout << "x : " << ((umsg::PointCloud *)sh_data->body)->points(i).x() << std::endl;
            std::cout << "y : " << ((umsg::PointCloud *)sh_data->body)->points(i).y() << std::endl;
            std::cout << "z : " << ((umsg::PointCloud *)sh_data->body)->points(i).z() << std::endl;
        }
        // send_msg.body->size = send_msg.body->msg->ByteSizeLong();

        ipc->writeBody();
        const umsg::PointXYZIRL& p = ((umsg::PointCloud *)sh_data->body)->points(0);  // points의 첫 번째 요소 가져오기
        float x = p.x();  // x 값 가져오기
        float y = p.y();  // y 값 가져오기
        float z = p.z();  // z 값 가져오기
        
        wait_timerfd(timer_fd);
        if(cnt > 100000)
            break;
    }
    ipc->Free();
    return 0;
}