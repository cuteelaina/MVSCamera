#include <MVSCamera.h>


CameraNoExcept::CameraNoExcept(std::shared_ptr<Camera> camera) : m_camera(camera) {}

bool CameraNoExcept::start_grabbing() noexcept
{
    try
    {
        m_camera->start_grabbing();
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::start_grabbing() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::stop_grabbing() noexcept
{
    try
    {
        m_camera->stop_grabbing();
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::stop_grabbing() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::get_frame(cv::Mat& frame, int timeout_ms) noexcept
{
    try
    {
        m_camera->get_frame(frame, timeout_ms);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::get_frame() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::get_int_value(const char* name, int& value) noexcept
{
    try
    {
        m_camera->get_int_value(name, value);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::get_int_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::set_int_value(const char* name, const int& value) noexcept
{
    try
    {
        m_camera->set_int_value(name, value);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::set_int_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::get_enum_value(const char* name, unsigned int& value) noexcept
{
    try
    {
        m_camera->get_enum_value(name, value);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::get_enum_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::set_enum_value(const char* name, const unsigned int& value) noexcept
{
    try
    {
        m_camera->set_enum_value(name, value);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::set_enum_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::get_bool_value(const char* name, bool& value) noexcept
{
    try
    {
        m_camera->get_bool_value(name, value);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::get_bool_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::set_bool_value(const char* name, const bool& value) noexcept
{
    try
    {
        m_camera->set_bool_value(name, value);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::set_bool_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::get_float_value(const char* name, float& value) noexcept
{
    try
    {
        m_camera->get_float_value(name, value);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::get_float_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::set_float_value(const char* name, const float& value) noexcept
{
    try
    {
        m_camera->set_float_value(name, value);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::set_float_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::get_string_value(const char* name, char* value, int size) noexcept
{
    try
    {
        m_camera->get_string_value(name, value, size);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::get_string_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::set_string_value(const char* name, const char* value) noexcept
{
    try
    {
        m_camera->set_string_value(name, value);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::set_string_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::set_command_value(const char* name) noexcept
{
    try
    {
        m_camera->set_command_value(name);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::set_command_value() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::set_gain(const float& gain) noexcept
{
    try
    {
        m_camera->set_gain(gain);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::set_gain() error: " << e.what() << std::endl;
        return false;
    }
}
bool CameraNoExcept::set_exposure_time(const float& exposure_time) noexcept
{
    try
    {
        m_camera->set_exposure_time(exposure_time);
        return true;
    }
    catch (CameraException& e)
    {
        std::cerr << "CameraNoExcept::set_exposure_time() error: " << e.what() << std::endl;
        return false;
    }
}