#include <MVSCamera.h>
#include <filesystem>
#include <opencv2/opencv.hpp>

const double show_scale = 0.5;
double target_fps = 125.0;

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

int main(int argc, char** argv)
{
    if (argc > 1)
        target_fps = std::stod(argv[1]);

    CameraManager camera_manager;
    CameraManager::CameraInfoList camera_list = camera_manager.list_cameras();
    for (int i = 0; i < camera_list.size(); i++)
        std::cout << "Camera " << i << ": " << camera_list.get_serial_number(i) << std::endl;
    std::cout << "Enter camera index to open: ";
    int camera_index = 0;
    std::cin >> camera_index;
    auto camera = camera_manager.create_camera(camera_list, camera_index);

    CameraNoExcept safe_camera(camera);
    safe_camera.start_grabbing();

    const char* win_name = "Camera";
    cv::namedWindow(win_name, cv::WINDOW_AUTOSIZE);

    int gain_pos = 70;
    int exposure_pos = 20000;

    cv::createTrackbar("Gain x0.1", win_name, &gain_pos, 170, on_gain_trackbar, &safe_camera);
    cv::createTrackbar("Exposure", win_name, &exposure_pos, 50000, on_exposure_trackbar, &safe_camera);
    std::queue<long long> frame_time;
    long long frame_time_sum = 0;
    long long last_time = 0;
    int record_count = 0;
    bool recording = false;
    cv::VideoWriter video_writer;

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
        if (recording && video_writer.isOpened())
            video_writer.write(frame);

        cv::Mat show_frame = frame.clone();
        cv::putText(show_frame, "FPS: " + std::to_string(static_cast<int>(fps)), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
        if (!recording)
            cv::putText(show_frame, "Press 's' to start recording", cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);
        else
            cv::putText(show_frame, "Recording... Press 's' to stop", cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);

        cv::resize(show_frame, show_frame, cv::Size(), show_scale, show_scale);

        cv::imshow(win_name, show_frame);
        int key = cv::waitKey(1);
        if (key == 27 || key == 'q')
            break;
        if (key == 's')
        {
            if (!recording)
            {
                if (!std::filesystem::exists("RecordResult"))
                    std::filesystem::create_directory("RecordResult");
                if (!std::filesystem::exists("RecordResult/" + camera_list.get_serial_number(camera_index)))
                    std::filesystem::create_directory("RecordResult/" + camera_list.get_serial_number(camera_index));
                char filename[64];
                sprintf(filename, "./RecordResult/%s/record_%03d.avi", camera_list.get_serial_number(camera_index).c_str(), ++record_count);
                video_writer.open(filename, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), target_fps, frame.size());
                std::cout << "Opening video writer with FPS: " << target_fps << " Size: " << frame.size() << std::endl;
                if (!video_writer.isOpened())
                {
                    std::cerr << "Failed to open video writer!" << std::endl;
                    recording = false;
                }
                else
                    std::cout << "Started recording to " << filename << std::endl;
            }
            else
                video_writer.release();
            recording = !recording;
        }
    }
    return 0;
}