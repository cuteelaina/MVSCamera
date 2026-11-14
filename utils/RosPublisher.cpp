#include <MVSCamera.h>
#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <limits>
#include <string>

static void on_gain_trackbar(int pos, void* userdata)
{
    if (!userdata)
        return;
    auto cam = reinterpret_cast<CameraNoExcept*>(userdata);
    cam->set_gain(pos / 10.0f);
}

static void on_exposure_trackbar(int pos, void* userdata)
{
    if (!userdata)
        return;
    auto cam = reinterpret_cast<CameraNoExcept*>(userdata);
    cam->set_exposure_time(static_cast<float>(pos));
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "gige_camera_publisher");
    ros::NodeHandle nh;

    // 枚举相机（与 capture 一致）
    CameraManager camera_manager;
    CameraManager::CameraInfoList camera_list = camera_manager.list_cameras();
    for (int i = 0; i < static_cast<int>(camera_list.size()); i++)
        std::cout << "Camera " << i << ": "
                  << camera_list.get_serial_number(i) << " - "
                  << camera_list.get_model_name(i) << std::endl;

    if (camera_list.is_empty())
    {
        ROS_FATAL("No camera found");
        return 1;
    }

    // 询问相机索引
    std::cout << "Enter camera index to open: " << std::flush;
    int camera_index = 0;
    std::cin >> camera_index;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    // 询问发布话题
    std::cout << "Enter topic to publish [default: /camera/image_raw]: " << std::flush;
    std::string topic;
    std::getline(std::cin, topic);
    if (topic.empty())
        topic = "/camera/image_raw";

    // 创建相机并开始采集
    auto camera = camera_manager.create_camera(camera_list, camera_index);
    CameraNoExcept safe_camera(camera);
    if (!safe_camera.start_grabbing())
    {
        ROS_FATAL("Failed to start grabbing");
        return 1;
    }

    // 本地预览窗口 + 曝光/增益轨迹条
    const std::string win_title = "GigE Camera Preview - " + topic;
    cv::namedWindow(win_title, cv::WINDOW_NORMAL);
    cv::resizeWindow(win_title, 1280, 720);

    int gain_pos = 70;
    int exposure_pos = 20000;
    cv::createTrackbar("Gain x0.1", win_title, &gain_pos, 170, on_gain_trackbar, &safe_camera);
    cv::createTrackbar("Exposure", win_title, &exposure_pos, 200000, on_exposure_trackbar, &safe_camera);
    on_gain_trackbar(gain_pos, &safe_camera);
    on_exposure_trackbar(exposure_pos, &safe_camera);

    // 发布 mono8 到话题
    ros::Publisher pub = nh.advertise<sensor_msgs::Image>(topic, 1);
    ROS_INFO("Publishing mono8 to %s", topic.c_str());

    cv::Mat frame, gray;
    ros::Rate rate(30);
    while (ros::ok())
    {
        if (!safe_camera.get_frame(frame, 100))
            continue;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv_bridge::CvImage cv_img;
        cv_img.header.stamp = ros::Time::now();
        cv_img.encoding = "mono8";
        cv_img.image = gray;
        pub.publish(cv_img.toImageMsg());

        cv::imshow(win_title, frame);
        rate.sleep();


        // 用一个属性判断窗口状态（紧跟 waitKey 之后、imshow 之前检查）：
        //   0  → 窗口存在且仍是 namedWindow(WINDOW_NORMAL) 的原始窗口，继续
        //   1  → 窗口被 imshow 重建为 WINDOW_AUTOSIZE，退出
        //  -1  → 窗口已被关闭（不存在），退出
        if (cv::getWindowProperty(win_title, cv::WND_PROP_AUTOSIZE) != 0)
        {
            ROS_INFO("Preview window closed or recreated, shutting down.");
            break;
        }

        int key = cv::waitKey(1);
        if (key == 27)
            break;
    }

    cv::destroyAllWindows();
    safe_camera.stop_grabbing();
    return 0;
}
