#pragma once
#include <MvCameraControl.h>
#include <opencv2/opencv.hpp>

class Camera
{
public:
    static const int image_node_num = 4;

    class HandleManager
    {
    private:
        void* m_handle = nullptr;
        std::mutex m_camera_lock;
        std::mutex m_buffer_lock;

    public:
        struct Handle
        {
            void* m_handle;
            std::unique_lock<std::mutex> m_lock;

            Handle(void* p, std::mutex& m);
        };

        explicit HandleManager(void* h);
        ~HandleManager();

        HandleManager(const HandleManager&) = delete;
        HandleManager& operator=(const HandleManager&) = delete;
        HandleManager(HandleManager&&) = delete;
        HandleManager& operator=(HandleManager&&) = delete;

        Handle get_camera();
        Handle get_buffer();
    };

private:
    std::unique_ptr<HandleManager> m_handle_manager;
    bool m_is_open = false;
    bool m_is_streaming = false;

public:
    explicit Camera(void* handle);
    ~Camera();
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;
    Camera(Camera&&) = delete;
    Camera& operator=(Camera&&) = delete;

    friend class CameraManager;
    void start_grabbing();
    void stop_grabbing();

    void get_frame(cv::Mat& frame, int timeout_ms = 1000);

    void get_int_value(const char* name, int& value);
    void set_int_value(const char* name, const int& value);
    void get_enum_value(const char* name, unsigned int& value);
    void set_enum_value(const char* name, const unsigned int& value);
    void get_bool_value(const char* name, bool& value);
    void set_bool_value(const char* name, const bool& value);
    void get_float_value(const char* name, float& value);
    void set_float_value(const char* name, const float& value);
    void get_string_value(const char* name, char* value, int size);
    void set_string_value(const char* name, const char* value);
    void set_command_value(const char* name);

    void set_gain(const float& gain);
    void set_exposure_time(const float& exposure_time);
};

class CameraNoExcept
{
private:
    std::shared_ptr<Camera> m_camera;

public:
    explicit CameraNoExcept(std::shared_ptr<Camera> camera);
    bool start_grabbing() noexcept;
    bool stop_grabbing() noexcept;

    bool get_frame(cv::Mat& frame, int timeout_ms = 1000) noexcept;

    bool get_int_value(const char* name, int& value) noexcept;
    bool set_int_value(const char* name, const int& value) noexcept;
    bool get_enum_value(const char* name, unsigned int& value) noexcept;
    bool set_enum_value(const char* name, const unsigned int& value) noexcept;
    bool get_bool_value(const char* name, bool& value) noexcept;
    bool set_bool_value(const char* name, const bool& value) noexcept;
    bool get_float_value(const char* name, float& value) noexcept;
    bool set_float_value(const char* name, const float& value) noexcept;
    bool get_string_value(const char* name, char* value, int size) noexcept;
    bool set_string_value(const char* name, const char* value) noexcept;
    bool set_command_value(const char* name) noexcept;

    bool set_gain(const float& gain) noexcept;
    bool set_exposure_time(const float& exposure_time) noexcept;
};

class CameraManager
{
    static bool initialized;
    static std::mutex init_mutex;

public:
    class CameraInfoList
    {
        friend class CameraManager;

    private:
        std::shared_ptr<MV_CC_DEVICE_INFO_LIST> m_device_info_list;
        explicit CameraInfoList(const std::shared_ptr<MV_CC_DEVICE_INFO_LIST>& device_info_list);

    public:
        size_t size() const;
        bool is_empty() const;
        std::string get_model_name(size_t index) const;
        std::string get_serial_number(size_t index) const;
    };

    CameraManager();

    CameraInfoList list_cameras();

    std::shared_ptr<Camera> create_camera(const CameraInfoList& info, int id);
};

class CameraException : public std::runtime_error
{
    static bool check_error(int status_code, std::string function_name = "", std::string step_name = "");
    friend class CameraManager;
    friend class Camera;

public:
    explicit CameraException(const std::string& message);
    CameraException(const std::string& message, const std::string& function_name);
};