#include <MVSCamera.h>
#include <algorithm>
#include <filesystem>
#include <opencv2/opencv.hpp>

const double show_scale = 0.5;
const int chessboard_width = 11; // 12-1
const int chessboard_height = 8; // 9-1
const int chessboard_size = 25;  // millimeters

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
    if (argc != 1 && std::string(argv[1]) == "captured")
    {
        std::vector<std::string> serial_numbers;
        if (std::filesystem::exists("CaptureResult"))
        {
            for (const auto& entry : std::filesystem::directory_iterator("CaptureResult"))
                if (entry.is_directory())
                    serial_numbers.push_back(entry.path().filename().string());
        }

        if (serial_numbers.empty())
        {
            std::cout << "No captured data found in CaptureResult." << std::endl;
            return 0;
        }

        for (size_t i = 0; i < serial_numbers.size(); i++)
            std::cout << "Camera " << i << ": " << serial_numbers[i] << std::endl;

        std::cout << "Enter camera index to calibrate: ";
        int camera_index = 0;
        std::cin >> camera_index;

        if (camera_index < 0 || camera_index >= serial_numbers.size())
        {
            std::cout << "Invalid index." << std::endl;
            return 0;
        }

        std::string selected_serial = serial_numbers[camera_index];
        std::string capture_path = "CaptureResult/" + selected_serial;
        std::vector<std::string> images;
        for (const auto& entry : std::filesystem::directory_iterator(capture_path))
        {
            if (entry.path().extension() == ".png")
                images.push_back(entry.path().string());
        }
        std::sort(images.begin(), images.end());

        std::vector<std::vector<cv::Point3f>> object_points;
        std::vector<std::vector<cv::Point2f>> image_points;
        cv::Size image_size;
        int valid_count = 0;

        for (const auto& img_path : images)
        {
            cv::Mat frame = cv::imread(img_path);
            if (frame.empty())
                continue;

            if (image_size.width == 0 && image_size.height == 0)
                image_size = frame.size();

            std::vector<cv::Point2f> corners;
            cv::Mat gray;
            cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
            bool pattern_founded = cv::findChessboardCorners(gray, cv::Size(chessboard_width, chessboard_height), corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK);

            if (pattern_founded)
            {
                cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.0001));

                cv::drawChessboardCorners(frame, cv::Size(chessboard_width, chessboard_height), corners, pattern_founded);
                cv::imshow("Corners", frame);
                cv::waitKey(0);

                std::vector<cv::Point3f> obj;
                for (int i = 0; i < chessboard_height; i++)
                    for (int j = 0; j < chessboard_width; j++)
                        obj.emplace_back(j * chessboard_size, i * chessboard_size, 0.0f);

                object_points.push_back(obj);
                image_points.push_back(corners);
                valid_count++;
                std::cout << "Processed " << img_path << ": Pattern Found" << std::endl;
            }
            else
            {
                std::cout << "Processed " << img_path << ": Pattern Not Found" << std::endl;
            }
        }
        cv::destroyAllWindows();

        if (valid_count >= 10)
        {
            std::cout << "Calibrating camera with " << valid_count << " captures..." << std::endl;
            cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64F);
            cv::Mat dist_coeffs = cv::Mat::zeros(8, 1, CV_64F);
            std::vector<cv::Mat> rvecs, tvecs;
            double rms = cv::calibrateCamera(object_points, image_points, image_size, camera_matrix, dist_coeffs, rvecs, tvecs);
            std::cout << "Calibration done with RMS error = " << rms << std::endl;
            std::cout << "Camera Matrix: " << std::endl << camera_matrix << std::endl;
            std::cout << "Distortion Coefficients: " << std::endl << dist_coeffs << std::endl;

            if (!std::filesystem::exists("CalibrationResult"))
                std::filesystem::create_directory("CalibrationResult");
            cv::FileStorage fs("CalibrationResult/" + selected_serial + ".yaml", cv::FileStorage::WRITE);
            fs << "Camera_Matrix" << camera_matrix;
            fs << "Distortion_Coefficients" << dist_coeffs;
            fs.release();
            std::cout << "Calibration data saved to CalibrationResult/" << selected_serial << ".yaml" << std::endl;

            // 计算重投影误差
            double total_error = 0.0;
            int total_points = 0;
            for (size_t i = 0; i < object_points.size(); i++)
            {
                std::vector<cv::Point2f> projected_points;
                cv::projectPoints(object_points[i], rvecs[i], tvecs[i], camera_matrix, dist_coeffs, projected_points);
                double error = cv::norm(image_points[i], projected_points, cv::NORM_L2);
                total_error += error * error;
                total_points += object_points[i].size();
                std::cout << "Capture " << i + 1 << ": Reprojection Error = " << std::sqrt(error * error / object_points[i].size()) << std::endl;
            }
            double avg_error = total_error / total_points;
            std::cout << "Average reprojection error: " << avg_error << std::endl;
        }
        else
        {
            std::cout << "Not enough valid captures for calibration (found " << valid_count << ")." << std::endl;
        }

        return 0;
    }

    CameraManager camera_manager;
    CameraManager::CameraInfoList camera_list = camera_manager.list_cameras();
    for (int i = 0; i < camera_list.size(); i++)
        std::cout << "Camera " << i << ": " << camera_list.get_serial_number(i) << " - " << camera_list.get_model_name(i) << std::endl;
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
    cv::createTrackbar("Exposure", win_name, &exposure_pos, 100000, on_exposure_trackbar, &safe_camera);
    std::queue<long long> frame_time;
    long long frame_time_sum = 0;
    long long last_time = 0;
    int capture_count = 0;
    cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64F), dist_coeffs = cv::Mat::zeros(8, 1, CV_64F);
    std::vector<std::vector<cv::Point3f>> object_points;
    std::vector<std::vector<cv::Point2f>> image_points;
    cv::Size image_size;

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

        cv::Mat show_frame = frame.clone();
        cv::putText(show_frame, "FPS: " + std::to_string(static_cast<int>(fps)), cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);

        if (image_size.width == 0 && image_size.height == 0)
            image_size = frame.size();
        std::vector<cv::Point2f> corners;
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        bool pattern_founded = false;
        pattern_founded = cv::findChessboardCorners(gray, cv::Size(chessboard_width, chessboard_height), corners, cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE | cv::CALIB_CB_FAST_CHECK);
        if (pattern_founded)
        {
            cv::putText(show_frame, "Pattern Found", cv::Point(10, 70), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
            cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.0001));
            cv::drawChessboardCorners(show_frame, cv::Size(chessboard_width, chessboard_height), corners, pattern_founded);
        }

        cv::resize(show_frame, show_frame, cv::Size(), show_scale, show_scale);

        cv::imshow(win_name, show_frame);
        int key = cv::waitKey(1);
        if (key == 27 || key == 'q')
            break;
        if (key == 's' && pattern_founded)
        {
            std::vector<cv::Point3f> obj;
            for (int i = 0; i < chessboard_height; i++)
                for (int j = 0; j < chessboard_width; j++)
                    obj.emplace_back(j * chessboard_size, i * chessboard_size, 0.0f);
            object_points.push_back(obj);
            image_points.push_back(corners);
            capture_count++;
            std::cout << "Captured " << capture_count << " frames." << std::endl;
        }
    }
    if (capture_count >= 10)
    {
        std::cout << "Calibrating camera with " << capture_count << " captures..." << std::endl;
        std::vector<cv::Mat> rvecs, tvecs;
        double reproj_error = 0.0;
        double rms = cv::calibrateCamera(object_points, image_points, image_size, camera_matrix, dist_coeffs, rvecs, tvecs);
        std::cout << "Calibration done with RMS error = " << rms << std::endl;
        std::cout << "Camera Matrix: " << std::endl << camera_matrix << std::endl;
        std::cout << "Distortion Coefficients: " << std::endl << dist_coeffs << std::endl;

        if (!std::filesystem::exists("CalibrationResult"))
            std::filesystem::create_directory("CalibrationResult");
        cv::FileStorage fs("CalibrationResult/" + camera_list.get_serial_number(camera_index) + ".yaml", cv::FileStorage::WRITE);
        fs << "Camera_Matrix" << camera_matrix;
        fs << "Distortion_Coefficients" << dist_coeffs;
        fs.release();
        std::cout << "Calibration data saved to CalibrationResult/" << camera_list.get_serial_number(camera_index) << ".yaml" << std::endl;

        // 计算重投影误差
        double total_error = 0.0;
        int total_points = 0;
        for (size_t i = 0; i < object_points.size(); i++)
        {
            std::vector<cv::Point2f> projected_points;
            cv::projectPoints(object_points[i], rvecs[i], tvecs[i], camera_matrix, dist_coeffs, projected_points);
            double error = cv::norm(image_points[i], projected_points, cv::NORM_L2);
            total_error += error * error;
            total_points += object_points[i].size();
            std::cout << "Capture " << i + 1 << ": Reprojection Error = " << std::sqrt(error * error / object_points[i].size()) << std::endl;
        }
        double avg_error = total_error / total_points;
        std::cout << "Average reprojection error: " << avg_error << std::endl;
    }
    else
    {
        std::cout << "Not enough captures for calibration." << std::endl;
    }
    return 0;
}