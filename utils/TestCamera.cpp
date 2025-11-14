#include <MVSCamera.h>
#include <chrono>
#include <opencv2/opencv.hpp>
#include <queue>

static void on_gain_trackbar(int pos, void* userdata)
{
    if (!userdata)
        return;
    auto cam = reinterpret_cast<CameraNoExcept*>(userdata);
    float gain = pos / 10.0f;
    cam->set_gain(gain);
}

static void on_exposure_trackbar(int pos, void* userdata)
{
    if (!userdata)
        return;
    auto cam = reinterpret_cast<CameraNoExcept*>(userdata);
    float exposure = pos;
    cam->set_exposure_time(exposure);
}

int main()
{
    CameraManager camera_manager;
    auto camera = camera_manager.create_camera(camera_manager.list_cameras(), 0);
    CameraNoExcept safe_camera(camera);
    safe_camera.start_grabbing();

    const char* win_name = "Camera";
    cv::namedWindow(win_name, cv::WINDOW_AUTOSIZE);

    int gain_pos = 170;
    int exposure_pos = 4000;

    cv::createTrackbar("Gain x0.1", win_name, &gain_pos, 170, on_gain_trackbar, &safe_camera);
    cv::createTrackbar("Exposure", win_name, &exposure_pos, 50000, on_exposure_trackbar, &safe_camera);

    on_gain_trackbar(gain_pos, &safe_camera);
    on_exposure_trackbar(exposure_pos, &safe_camera);

    std::queue<long long> frame_time;
    long long frame_time_sum = 0;

    // 使用 steady_clock 记录两帧间隔
    auto prev_time = std::chrono::steady_clock::now();

    while (true)
    {
        cv::Mat frame;

        if (!safe_camera.get_frame(frame, 100))
        {
            if (cv::waitKey(1) == 27)
                break;
            continue;
        }

        // 以成功获取到帧时的时间为基准，计算两帧间隔（毫秒）
        auto now = std::chrono::steady_clock::now();
        long long delta_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now - prev_time).count();
        prev_time = now;

        // 如果 delta_ms 为 0（首次或极短间隔），用 1ms 避免除以零
        if (delta_ms <= 0)
            delta_ms = 1;

        frame_time.push(delta_ms);
        frame_time_sum += delta_ms;
        const size_t window_size = 60;
        if (frame_time.size() > window_size)
        {
            frame_time_sum -= frame_time.front();
            frame_time.pop();
        }

        double avg_time = frame_time.empty() ? 0.0 : static_cast<double>(frame_time_sum) / frame_time.size();
        float fps = (avg_time > 0.0) ? static_cast<float>(1000.0 / avg_time) : 0.0f;

        cv::putText(frame, "FPS: " + std::to_string(fps), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        float camera_fps = 0.0f;
        safe_camera.get_float_value("ResultingFrameRate", camera_fps);
        cv::putText(frame, "Device FPS: " + std::to_string(camera_fps), cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        cv::imshow(win_name, frame);
        if (cv::waitKey(1) == 27)
        {
            break;
        }
    }
    return 0;
}