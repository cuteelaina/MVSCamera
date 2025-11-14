#include <MVSCamera.h>

int main()
{
    CameraManager camera_manager;
    auto devices = camera_manager.list_cameras();

    for (int i = 0; i < devices.size(); i++)
    {
        std::string serial_number = devices.get_serial_number(i);
        std::cout << "Camera " << i << " Serial Number: " << serial_number << std::endl;
    }
    return 0;
}