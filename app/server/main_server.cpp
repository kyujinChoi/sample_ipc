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
    // shData_t *sh_data;
    shData_t point_msg(SharedData::POINTCLOUD);
    shData_t log_msg(SharedData::LOG_EVENT);
    int timer_fd = init_timerfd(100);
    while(1)
    {
        // ipc->ReadHeader(sh_data->header);
        // sh_data = ipc->getSharedData();
        // std::string buf = buffer + std::to_string(cnt++);
        // send_msg.body->type = cnt * 10;
        std::cout << "-----------------------\n";
        if(cnt % SharedData::MAX_NUM == SharedData::POINTCLOUD)
        {
            
            ((umsg::PointCloud *)point_msg.body)->clear_points();
            for(int i = 0 ; i < 9; i++)
            {
                umsg::PointXYZIRL* point = ((umsg::PointCloud *)point_msg.body)->add_points(); 
                point->set_x(cnt + i + 0.123);
                point->set_y(cnt + i + 0.456);
                point->set_z(cnt + i + 0.789);
                std::cout << "i = " << i << std::endl;
                std::cout << "x : " << ((umsg::PointCloud *)point_msg.body)->points(i).x() << std::endl;
                std::cout << "y : " << ((umsg::PointCloud *)point_msg.body)->points(i).y() << std::endl;
                std::cout << "z : " << ((umsg::PointCloud *)point_msg.body)->points(i).z() << std::endl;
            }
            // send_msg.body->size = send_msg.body->msg->ByteSizeLong();

            ipc->writeBody(SharedData::POINTCLOUD, &point_msg);
        }
        else if(cnt % SharedData::MAX_NUM == SharedData::LOG_EVENT)
        {
            ((umsg::LogEvent *)log_msg.body)->set_obsol("MESSAGE : " + std::to_string(cnt));
            std::cout << log_msg.cnt << "'s message = " << ((umsg::LogEvent *)log_msg.body)->obsol() << std::endl;
            ipc->writeBody(SharedData::LOG_EVENT, &log_msg);
        }
        cnt++;
        wait_timerfd(timer_fd);
    }
    ipc->Free();
    return 0;
}