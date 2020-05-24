#include<stdio.h>
#include<iostream>
#include <iomanip>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include<ctime>
#include <sys/time.h>
#include<unistd.h>
#include "opencv/highgui.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"

#define MAX_PATH_LEN 256

#ifdef _MKDIR_LINUX
#define ACCESS(fileName,accessMode) _access(fileName,accessMode)
#define MKDIR(path) _mkdir(path)
#else
#define ACCESS(fileName,accessMode) access(fileName,accessMode)
#define MKDIR(path) mkdir(path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#endif

using namespace std;
using namespace cv;

// 从左到右依次判断文件夹是否存在,不存在就创建
// example: /home/root/mkdir/1/2/3/4/
// 注意:最后一个如果是文件夹的话,需要加上 '\' 或者 '/'
int32_t createDirectory(const std::string &directoryPath)
{
    std::cout<<"\n开始创建文件夹\n";
    uint32_t dirPathLen = directoryPath.length();
    if (dirPathLen > MAX_PATH_LEN)
    {
        return -1;
    }
    char tmpDirPath[MAX_PATH_LEN] = { 0 };
    for (uint32_t i = 0; i < dirPathLen; ++i)
    {
        tmpDirPath[i] = directoryPath[i];
        if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/')
        {
            if (ACCESS(tmpDirPath, 0) != 0)
            {
                int32_t ret = MKDIR(tmpDirPath);
                if (ret != 0)
                {
                    return ret;
                }
            }
        }
    }
    return 0;
}

int main(int argc,char* argv[])
{
    cv::VideoCapture capture(argv[1]);
    if(!capture.isOpened())
    {
        std::cout<<"video not open."<<std::endl;
        return 1;
    }

    //创建文件夹
    std::string file_address(argv[2], argv[2] + strlen(argv[2]));
    std::string SaveRGBAddress = file_address+"rgb/";
    if (argc == 3)
    {
        int return_num = createDirectory(argv[2]);//创建储存主文件夹
        createDirectory(SaveRGBAddress);//创建储存rgb图文件夹
    }
    else
    {
        std::cout<<"\n输入信息有误,不能创建文件夹\n";
        return -1;
    }

    //获取当前视频帧率
    double rate = capture.get(CV_CAP_PROP_FPS);
    std::cout << "当前视频帧率:" << rate << std::endl;
    unsigned int all_num= capture.get(CV_CAP_PROP_FRAME_COUNT);
    std::cout << "当前视频帧总数:" << all_num << std::endl; 
    //当前视频帧
    cv::Mat frame,frameResized;
    //每一帧之间的延时
    //与视频的帧率相对应
    int delay = 1000/rate;
    bool stop(false);
    std::ofstream f;
    string TS_file_address = file_address +"rgb.txt";
    f.open(TS_file_address, ios::out|ios::trunc);//覆盖，不存在则创建
    if(!f.fail())
    {
      f << "# color images" << std::endl;
      f << "# file: 'rgbd_dataset_XTU_treArches.zip'" << std::endl;
      f << "# timestamp filename" << std::endl;
    }

    f << std::fixed;

    while(!stop)
    {
        if(!capture.read(frame))
        {
            std::cout<<"no video frame"<<std::endl;
	    f.close();
            break;
        }
        //此处为添加对视频的每一帧的操作方法
	double Time_frame = capture.get(CV_CAP_PROP_POS_MSEC);
	Time_frame = Time_frame/1000.0;//转换为秒
	double current_Time = 1911291500.0;
	current_Time = current_Time+Time_frame;
	int frame_num = capture.get(CV_CAP_PROP_POS_FRAMES);
	std::cout<<"Frame Num : "<<frame_num;
	f << current_Time;
	f << " ";
	f << "rgb/" << std::to_string(current_Time) << ".png" << std::endl;
	std::cout << std::fixed << std::setprecision(6) <<  "    Timestamp : "<< current_Time << "   /   " << std::to_string(Time_frame) <<std::endl;
        //cv::imshow("video",frame);
	std::stringstream  ImageName;
	ImageName << std::to_string(current_Time) << ".png";
	std::string SaveFilesName = SaveRGBAddress+ImageName.str();
	int Image_size_Max = fmax(frame.rows,frame.cols);
	cv::namedWindow("Frame",CV_WINDOW_AUTOSIZE);
	cv::imshow("Frame",frame);
	if(Image_size_Max > 640)
	{
		float scale = (float)640.0f/(float)Image_size_Max; 
		cv::resize(frame,frameResized,cv::Size(),scale,scale);
	}
	else	
		frameResized = frame;
	cv::imwrite(SaveFilesName,frameResized);
        //引入延时
        //也可通过按键停止
        if(cv::waitKey(delay)>0)
        stop = true;
    }
    //关闭视频，手动调用析构函数（非必须）
    capture.release();
    return 0;
}
