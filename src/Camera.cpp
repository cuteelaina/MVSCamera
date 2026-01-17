#include <MVSCamera.h>


Camera::Camera(void* handler) : m_is_streaming(false)
{
    m_handle_manager = std::make_unique<HandleManager>(handler);
    HandleManager::Handle handle = m_handle_manager->get_camera();
    CameraException::check_error(MV_CC_OpenDevice(handle.m_handle), "create_camera", "MV_CC_OpenDevice");
    CameraException::check_error(MV_CC_SetImageNodeNum(handle.m_handle, image_node_num), "create_camera", "MV_CC_SetImageNodeNum");
    CameraException::check_error(MV_CC_SetEnumValue(handle.m_handle, "TriggerMode", MV_TRIGGER_MODE_OFF), "create_camera", "MV_CC_SetEnumValue");
    // MV_CC_SetEnumValue(handle.m_handle, "PixelFormat", PixelType_Gvsp_BGR8_Packed);
    // MV_CC_SetEnumValue(handle.m_handle, "PixelFormat", PixelType_Gvsp_BayerRG8);
    // MV_CC_SetEnumValue(handle.m_handle, "AcquisitionMode", MV_ACQ_MODE_CONTINUOUS);
    // MV_CC_SetEnumValue(handle.m_handle, "ADCBitDepth", 0u);
}

Camera::~Camera()
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    CameraException::check_error(MV_CC_CloseDevice(handle.m_handle), "destroy_camera", "MV_CC_CloseDevice");
}

void Camera::start_grabbing()
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    CameraException::check_error(MV_CC_StartGrabbing(handle.m_handle), "start_grabbing", "MV_CC_StartGrabbing");
    m_is_streaming = true;
}
void Camera::stop_grabbing()
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    m_is_streaming = false;
    CameraException::check_error(MV_CC_StopGrabbing(handle.m_handle), "stop_grabbing", "MV_CC_StopGrabbing");
}

bool is_compress_format(MvGvspPixelType enType)
{
    switch (enType)
    {
    case PixelType_Gvsp_HB_Mono8:
    case PixelType_Gvsp_HB_Mono10:
    case PixelType_Gvsp_HB_Mono10_Packed:
    case PixelType_Gvsp_HB_Mono12:
    case PixelType_Gvsp_HB_Mono12_Packed:
    case PixelType_Gvsp_HB_Mono16:
    case PixelType_Gvsp_HB_RGB8_Packed:
    case PixelType_Gvsp_HB_BGR8_Packed:
    case PixelType_Gvsp_HB_RGBA8_Packed:
    case PixelType_Gvsp_HB_BGRA8_Packed:
    case PixelType_Gvsp_HB_RGB16_Packed:
    case PixelType_Gvsp_HB_BGR16_Packed:
    case PixelType_Gvsp_HB_RGBA16_Packed:
    case PixelType_Gvsp_HB_BGRA16_Packed:
    case PixelType_Gvsp_HB_YUV422_Packed:
    case PixelType_Gvsp_HB_YUV422_YUYV_Packed:
    case PixelType_Gvsp_HB_BayerGR8:
    case PixelType_Gvsp_HB_BayerRG8:
    case PixelType_Gvsp_HB_BayerGB8:
    case PixelType_Gvsp_HB_BayerBG8:
    case PixelType_Gvsp_HB_BayerRBGG8:
    case PixelType_Gvsp_HB_BayerGB10:
    case PixelType_Gvsp_HB_BayerGB10_Packed:
    case PixelType_Gvsp_HB_BayerBG10:
    case PixelType_Gvsp_HB_BayerBG10_Packed:
    case PixelType_Gvsp_HB_BayerRG10:
    case PixelType_Gvsp_HB_BayerRG10_Packed:
    case PixelType_Gvsp_HB_BayerGR10:
    case PixelType_Gvsp_HB_BayerGR10_Packed:
    case PixelType_Gvsp_HB_BayerGB12:
    case PixelType_Gvsp_HB_BayerGB12_Packed:
    case PixelType_Gvsp_HB_BayerBG12:
    case PixelType_Gvsp_HB_BayerBG12_Packed:
    case PixelType_Gvsp_HB_BayerRG12:
    case PixelType_Gvsp_HB_BayerRG12_Packed:
    case PixelType_Gvsp_HB_BayerGR12:
    case PixelType_Gvsp_HB_BayerGR12_Packed:
        return true;
    default:
        return false;
    }
}

void Camera::get_frame(cv::Mat& frame, int timeout_ms)
{
    HandleManager::Handle handle = m_handle_manager->get_buffer();
    MV_FRAME_OUT image_info;
    CameraException::check_error(MV_CC_GetImageBuffer(handle.m_handle, &image_info, timeout_ms), "get_frame", "MV_CC_GetImageBuffer");

    // uint64_t timestamp = (static_cast<uint64_t>(image_info.stFrameInfo.nDevTimeStampHigh) << 32) | image_info.stFrameInfo.nDevTimeStampLow;
    // uint64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // std::cerr << "Timestamp: " << timestamp << " Now: " << now << " Diff: " << (now - timestamp) / 1000000 << " ms" << std::endl;

    if (is_compress_format(image_info.stFrameInfo.enPixelType))
    {
        try
        {
            MV_CC_HB_DECODE_PARAM decode_param;
            decode_param.pSrcBuf = image_info.pBufAddr;
            decode_param.nSrcLen = image_info.stFrameInfo.nFrameLen;
            get_int_value("PayloadSize", reinterpret_cast<int&>(decode_param.nDstBufSize));
            std::vector<unsigned char> decode_buffer(decode_param.nDstBufSize);
            decode_param.pDstBuf = decode_buffer.data();
            CameraException::check_error(MV_CC_HB_Decode(handle.m_handle, &decode_param), "get_frame", "MV_CC_HB_Decode");

            MV_CC_PIXEL_CONVERT_PARAM_EX convert_param;
            convert_param.nWidth = image_info.stFrameInfo.nWidth;
            convert_param.nHeight = image_info.stFrameInfo.nHeight;
            convert_param.pSrcData = decode_param.pDstBuf;
            convert_param.nSrcDataLen = decode_param.nDstBufLen;
            convert_param.enSrcPixelType = decode_param.enDstPixelType;
            convert_param.enDstPixelType = PixelType_Gvsp_BGR8_Packed;
            std::vector<unsigned char> buffer(convert_param.nWidth * convert_param.nHeight * 3);
            convert_param.pDstBuffer = buffer.data();
            convert_param.nDstBufferSize = static_cast<unsigned int>(buffer.size());
            CameraException::check_error(MV_CC_ConvertPixelTypeEx(handle.m_handle, &convert_param), "get_frame", "MV_CC_ConvertPixelTypeEx");
            cv::Mat img(image_info.stFrameInfo.nHeight, image_info.stFrameInfo.nWidth, CV_8UC3, buffer.data());
            frame = img.clone();
            CameraException::check_error(MV_CC_FreeImageBuffer(handle.m_handle, &image_info), "get_frame", "MV_CC_FreeImageBuffer");
            return;
        }
        catch (const CameraException& e)
        {
            try
            {
                CameraException::check_error(MV_CC_FreeImageBuffer(handle.m_handle, &image_info), "get_frame", "MV_CC_FreeImageBuffer");
            }
            catch (const CameraException& e2)
            {
                throw CameraException(std::string("Multiple errors occurred:\n") + e.what() + "\n" + e2.what());
            }
            throw e;
        }
    }

    if (image_info.stFrameInfo.enPixelType == PixelType_Gvsp_BGR8_Packed)
    {
        cv::Mat img(image_info.stFrameInfo.nHeight, image_info.stFrameInfo.nWidth, CV_8UC3, image_info.pBufAddr);
        frame = img.clone();
        CameraException::check_error(MV_CC_FreeImageBuffer(handle.m_handle, &image_info), "get_frame", "MV_CC_FreeImageBuffer");
        return;
    }

    MV_CC_PIXEL_CONVERT_PARAM_EX convert_param;
    convert_param.nWidth = image_info.stFrameInfo.nWidth;
    convert_param.nHeight = image_info.stFrameInfo.nHeight;
    convert_param.pSrcData = image_info.pBufAddr;
    convert_param.nSrcDataLen = image_info.stFrameInfo.nFrameLen;
    convert_param.enSrcPixelType = image_info.stFrameInfo.enPixelType;
    convert_param.enDstPixelType = PixelType_Gvsp_BGR8_Packed;

    std::vector<unsigned char> buffer(convert_param.nWidth * convert_param.nHeight * 3);
    convert_param.pDstBuffer = buffer.data();
    convert_param.nDstBufferSize = static_cast<unsigned int>(buffer.size());
    try
    {
        CameraException::check_error(MV_CC_ConvertPixelTypeEx(handle.m_handle, &convert_param), "get_frame", "MV_CC_ConvertPixelTypeEx");
    }
    catch (const CameraException& e)
    {
        try
        {
            CameraException::check_error(MV_CC_FreeImageBuffer(handle.m_handle, &image_info), "get_frame", "MV_CC_FreeImageBuffer");
        }
        catch (const CameraException& e2)
        {
            throw CameraException(std::string("Multiple errors occurred:\n") + e.what() + "\n" + e2.what());
        }
        throw e;
    }

    cv::Mat img(image_info.stFrameInfo.nHeight, image_info.stFrameInfo.nWidth, CV_8UC3, buffer.data());
    frame = img.clone();
    CameraException::check_error(MV_CC_FreeImageBuffer(handle.m_handle, &image_info), "get_frame", "MV_CC_FreeImageBuffer");
}

void Camera::get_int_value(const char* name, int& value)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    MVCC_INTVALUE_EX v;
    CameraException::check_error(MV_CC_GetIntValueEx(handle.m_handle, name, &v), "get_int_value", "MV_CC_GetIntValueEx");
    value = v.nCurValue;
}
void Camera::set_int_value(const char* name, const int& value)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    CameraException::check_error(MV_CC_SetIntValueEx(handle.m_handle, name, value), "set_int_value", "MV_CC_SetIntValueEx");
}
void Camera::get_enum_value(const char* name, unsigned int& value)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    MVCC_ENUMVALUE_EX v;
    CameraException::check_error(MV_CC_GetEnumValueEx(handle.m_handle, name, &v), "get_enum_value", "MV_CC_GetEnumValueEx");
    value = v.nCurValue;
}
void Camera::set_enum_value(const char* name, const unsigned int& value)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    CameraException::check_error(MV_CC_SetEnumValue(handle.m_handle, name, value), "set_enum_value", "MV_CC_SetEnumValue");
}
void Camera::get_bool_value(const char* name, bool& value)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    bool v = false;
    CameraException::check_error(MV_CC_GetBoolValue(handle.m_handle, name, &v), "get_bool_value", "MV_CC_GetBoolValue");
    value = v;
}
void Camera::set_bool_value(const char* name, const bool& value)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    CameraException::check_error(MV_CC_SetBoolValue(handle.m_handle, name, value), "set_bool_value", "MV_CC_SetBoolValue");
}
void Camera::get_float_value(const char* name, float& value)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    MVCC_FLOATVALUE v;
    CameraException::check_error(MV_CC_GetFloatValue(handle.m_handle, name, &v), "get_float_value", "MV_CC_GetFloatValue");
    value = v.fCurValue;
}
void Camera::set_float_value(const char* name, const float& value)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    CameraException::check_error(MV_CC_SetFloatValue(handle.m_handle, name, value), "set_float_value", "MV_CC_SetFloatValue");
}
void Camera::get_string_value(const char* name, char* value, int size)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    MVCC_STRINGVALUE v;
    CameraException::check_error(MV_CC_GetStringValue(handle.m_handle, name, &v), "get_string_value", "MV_CC_GetStringValue");
    strncpy(value, v.chCurValue, size);
}
void Camera::set_string_value(const char* name, const char* value)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    CameraException::check_error(MV_CC_SetStringValue(handle.m_handle, name, value), "set_string_value", "MV_CC_SetStringValue");
}
void Camera::set_command_value(const char* name)
{
    HandleManager::Handle handle = m_handle_manager->get_camera();
    CameraException::check_error(MV_CC_SetCommandValue(handle.m_handle, name), "set_command_value", "MV_CC_SetCommandValue");
}

void Camera::set_gain(const float& gain) { set_float_value("Gain", gain); }
void Camera::set_exposure_time(const float& exposure_time) { set_float_value("ExposureTime", exposure_time); }