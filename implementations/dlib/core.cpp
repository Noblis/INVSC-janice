#include "types.hpp"
#include "utils.hpp"

namespace janice_dlib
{

std::unique_ptr<Globals> _globals;

} // namespace janice_dlib

JaniceError janice_initialize(const char* _sdk_path, const char*, const char*, const char*, const int num_threads, const int* gpus, const int num_gpus)
{
    try {
        if (!janice_dlib::_globals) {
            std::string sdk_path(_sdk_path);

            std::string face_detector_filename = sdk_path + "/mmod_human_face_detector.dat";
            std::string shape_predictor_filename = sdk_path + "/shape_predictor_5_face_landmarks.dat";
            std::string feature_extractor_filename = sdk_path + "/dlib_face_recognition_resnet_model_v1.dat";

            if (!janice_dlib::file_exists(face_detector_filename)) {
                std::cerr << "Face detector file: [" << face_detector_filename << "] not found or can't be read." << std::endl;
                return JANICE_INVALID_SDK_PATH;
            }

            if (!janice_dlib::file_exists(shape_predictor_filename)) {
                std::cerr << "Shape predictor file: [" << shape_predictor_filename << "] not found or can't be read." << std::endl;
                return JANICE_INVALID_SDK_PATH;
            }

            if (!janice_dlib::file_exists(feature_extractor_filename)) {
                std::cerr << "Feature extractor file: [" << feature_extractor_filename << "] not found or can't be read." << std::endl;
                return JANICE_INVALID_SDK_PATH;
            }

            janice_dlib::_globals.reset(new janice_dlib::Globals());

            dlib::deserialize(std::string(sdk_path) + "/mmod_human_face_detector.dat") >> janice_dlib::_globals->fd;
            dlib::deserialize(std::string(sdk_path) + "/shape_predictor_5_face_landmarks.dat") >> janice_dlib::_globals->sp;
            dlib::deserialize(std::string(sdk_path) + "/dlib_face_recognition_resnet_model_v1.dat") >> janice_dlib::_globals->fe;

            janice_dlib::_globals->num_threads = num_threads;

            if (num_gpus > 0) {
#ifndef DLIB_WITH_CUDA
                std::string gpu_str = std::to_string(gpus[0]);
                for (int i = 1; i < num_gpus; ++i) {
                    gpu_str += "," + std::to_string(gpus[i]);
                }

                std::cerr << "You specified GPUs: [" << gpu_str << "] but DLib was not build with CUDA support. Rebuild with -DDLIB_WITH_CUDA" << std::endl;
                return JANICE_BAD_SDK_CONFIG;
#else
                if (num_gpus > 1) {
                    std::cerr << "The JanICE Dlib SDK can only leverage a single GPU. You provided: [" << num_gpus << "] gpus. Using GPU: [" << gpus[0] << "] only." << std::endl;
                }

                dlib::cuda::set_device(gpus[0]);
#endif
            }
        }
    }
    CATCH_AND_LOG(JANICE_MISSING_DATA, "Unable to initialize SDK");

    return JANICE_SUCCESS;
}

JaniceError janice_set_log_level(JaniceLogLevel /* level */)
{
    return JANICE_SUCCESS;
}

JaniceError janice_api_version(uint32_t* major, uint32_t* minor, uint32_t* patch)
{
    assert(major != nullptr);
    assert(minor != nullptr);
    assert(patch != nullptr);

    *major = 7;
    *minor = 0;
    *patch = 0;

    return JANICE_SUCCESS;
}

JaniceError janice_sdk_version(uint32_t* major, uint32_t* minor, uint32_t* patch)
{
    assert(major != nullptr);
    assert(minor != nullptr);
    assert(patch != nullptr);

    *major = 1;
    *minor = 0;
    *patch = 0;

    return JANICE_SUCCESS;
}

JaniceError janice_get_current_configuration(JaniceConfiguration* configuration)
{
    assert(configuration != nullptr);

    configuration->values = nullptr;
    configuration->length = 0;

    return JANICE_SUCCESS;
}

JaniceError janice_clear_configuration(JaniceConfiguration* /* configuration */)
{
    return JANICE_SUCCESS;
}

JaniceError janice_init_default_context(JaniceContext* context)
{
    assert(context != nullptr);

    context->policy = JaniceDetectAll;
    context->min_object_size = 36;
    context->role = Janice1NProbe;
    context->threshold = 0.6; // 0.6 is indictive of a strong match according to DLibs documentation
    context->max_returns = 50;
    context->hint = 0.5;
    context->batch_policy = JaniceFlagAndFinish;

    return JANICE_SUCCESS;
}

JaniceError janice_free_buffer(uint8_t** buffer)
{
    if (buffer && *buffer) {
        delete[] *buffer;
        *buffer = nullptr;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_errors(JaniceErrors* errors)
{
    if (errors) {
        delete[] errors->errors;
        errors->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_fine_tune(const JaniceMediaIterator*, const JaniceDetectionsGroup*, int**, const char*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_finalize()
{
    return JANICE_SUCCESS;
}

