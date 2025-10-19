#include <MVSCamera.h>
#include <opencv2/opencv.hpp>

static void on_gain_trackbar(int pos, void* userdata)
{
    if (!userdata)
        return;
    auto cam = reinterpret_cast<CameraNoExcept*>(userdata);
    float gain = pos / 10.0f;
    cam->set_gain(gain);
}

static void on_exposure_trackbarar(int pos, void* userdata)
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

    int gain_pos = 70;
    int exposure_pos = 20000;

    cv::createTrackbar("Gain x0.1", win_name, &gain_pos, 170, on_gain_trackbar, &safe_camera);
    cv::createTrackbar("Exposure", win_name, &exposure_pos, 50000, on_exposure_trackbarar, &safe_camera);
    std::queue<long long> frame_time;
    long long frame_time_sum = 0;
    long long last_time = 0;

    while (true)
    {
        cv::Mat frame;
        auto start_time = std::chrono::high_resolution_clock::now();

        if (!safe_camera.get_frame(frame, 1000))
        {
            last_time += 1000;
            continue;
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        long long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() + last_time;
        last_time = 0;
        frame_time.push(duration);
        frame_time_sum += duration;
        if (frame_time.size() > 30)
        {
            frame_time_sum -= frame_time.front();
            frame_time.pop();
        }
        double avg_time = static_cast<double>(frame_time_sum) / frame_time.size();
        double fps = 1000.0 / avg_time;
        cv::putText(frame, "FPS: " + std::to_string(static_cast<int>(fps)), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        cv::imshow(win_name, frame);
        if (cv::waitKey(1) == 27)
        {
            break;
        }
    }
    return 0;
}