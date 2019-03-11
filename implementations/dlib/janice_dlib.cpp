#include <janice.h>

#include <dlib/dnn.h>
#include <dlib/image_processing.h>

#include <iostream>
#include <fstream>
#include <memory>

// ----------------------------------------------------------------------------

#define CATCH_AND_LOG(ERROR_CODE, MSG)                        \
catch (std::exception& e) {                                   \
    std::cerr << MSG << ". Error: " << e.what() << std::endl; \
    return ERROR_CODE;                                        \
} catch (...) {                                               \
    std::cerr << MSG << ". Unknown error." << std::endl;      \
    return ERROR_CODE;                                        \
}

// ----------------------------------------------------------------------------------------
// The following defines the DLIB deep face detector

template <long num_filters, typename SUBNET> using con5d = dlib::con<num_filters,5,5,2,2,SUBNET>;
template <long num_filters, typename SUBNET> using con5  = dlib::con<num_filters,5,5,1,1,SUBNET>;

template <typename SUBNET> using downsampler = dlib::relu<dlib::affine<con5d<32, dlib::relu<dlib::affine<con5d<32, dlib::relu<dlib::affine<con5d<16,SUBNET>>>>>>>>>;
template <typename SUBNET> using rcon5       = dlib::relu<dlib::affine<con5<45,SUBNET>>>;

using face_detector_net_type = 
    dlib::loss_mmod<
      dlib::con<1,9,9,1,1,
        rcon5<
          rcon5<
            rcon5<
              downsampler<
                dlib::input_rgb_image_pyramid<
                  dlib::pyramid_down<6>
                >
              >
            >
          >
        >
      >
    >;

// ----------------------------------------------------------------------------------------
//
// The next bit of code defines a ResNet network.  It's basically copied
// and pasted from the dnn_imagenet_ex.cpp example, except we replaced the loss
// layer with loss_metric and made the network somewhat smaller.  Go read the introductory
// dlib DNN examples to learn what all this stuff means.
//
// Also, the dnn_metric_learning_on_images_ex.cpp example shows how to train this network.
// The dlib_face_recognition_resnet_model_v1 model used by this example was trained using
// essentially the code shown in dnn_metric_learning_on_images_ex.cpp except the
// mini-batches were made larger (35x15 instead of 5x5), the iterations without progress
// was set to 10000, and the training dataset consisted of about 3 million images instead of
// 55.  Also, the input layer was locked to images of size 150.

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = dlib::add_prev1<block<N,BN,1,dlib::tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = dlib::add_prev2<dlib::avg_pool<2,2,2,2,dlib::skip1<dlib::tag2<block<N,BN,2,dlib::tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET> 
using block  = BN<dlib::con<N,3,3,1,1,dlib::relu<BN<dlib::con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = dlib::relu<residual<block,N,dlib::affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block,N,dlib::affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

using feature_extractor_net_type = 
    dlib::loss_metric<
      dlib::fc_no_bias<128,
        dlib::avg_pool_everything<
          alevel0<
            alevel1<
              alevel2<
                alevel3<
                  alevel4<
                    dlib::max_pool<3,3,2,2,
                      dlib::relu<
                        dlib::affine<
                          dlib::con<32,7,7,2,2,
                            dlib::input_rgb_image_sized<150>
                          >
                        >
                      >
                    >
                  >
                >
              >
            >
          >
        >
      >
    >;

// ----------------------------------------------------------------------------
// Utility functions

namespace
{

inline bool file_exists(const std::string& filename)
{
    return std::ifstream(filename).good();
}

} // anonymous namespace

// ----------------------------------------------------------------------------

struct Globals
{
    face_detector_net_type fd;
    dlib::shape_predictor sp;
    feature_extractor_net_type fe;

    int num_threads;
};
static std::unique_ptr<Globals> _globals;

JaniceError janice_initialize(const char* _sdk_path, const char*, const char*, const char*, const int num_threads, const int* gpus, const int num_gpus)
{
    try {
        if (!_globals) {
            std::string sdk_path(_sdk_path);

            std::string face_detector_filename = sdk_path + "/mmod_human_face_detector.dat";
            std::string shape_predictor_filename = sdk_path + "/shape_predictor_5_face_landmarks.dat";
            std::string feature_extractor_filename = sdk_path + "/dlib_face_recognition_resnet_model_v1.dat";

            if (!file_exists(face_detector_filename)) {
                std::cerr << "Face detector file: [" << face_detector_filename << "] not found or can't be read." << std::endl;
                return JANICE_INVALID_SDK_PATH;
            }

            if (!file_exists(shape_predictor_filename)) {
                std::cerr << "Shape predictor file: [" << shape_predictor_filename << "] not found or can't be read." << std::endl;
                return JANICE_INVALID_SDK_PATH;
            }

            if (!file_exists(feature_extractor_filename)) {
                std::cerr << "Feature extractor file: [" << feature_extractor_filename << "] not found or can't be read." << std::endl;
                return JANICE_INVALID_SDK_PATH;
            }

            _globals.reset(new Globals());

            dlib::deserialize(std::string(sdk_path) + "/mmod_human_face_detector.dat") >> _globals->fd;
            dlib::deserialize(std::string(sdk_path) + "/shape_predictor_5_face_landmarks.dat") >> _globals->sp;
            dlib::deserialize(std::string(sdk_path) + "/dlib_face_recognition_resnet_model_v1.dat") >> _globals->fe;

            _globals->num_threads = num_threads;

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
    *major = 7;
    *minor = 0;
    *patch = 0;

    return JANICE_SUCCESS;
}

JaniceError janice_sdk_version(uint32_t* major, uint32_t* minor, uint32_t* patch)
{
    *major = 1;
    *minor = 0;
    *patch = 0;

    return JANICE_SUCCESS;
}

JaniceError janice_get_current_configuration(JaniceConfiguration* configuration)
{
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
    context->policy = JaniceDetectAll;
    context->min_object_size = 36;
    context->role = Janice1NProbe;
    context->threshold = 0.0;
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

JaniceError janice_clear_errors(JaniceErrors*)
{
    return JANICE_NOT_IMPLEMENTED;
}

struct JaniceDetectionType
{
    JaniceTrack track;
};

JaniceError janice_create_detection_from_rect(JaniceMediaIterator* /* media */, 
                                              const JaniceRect* rect,
                                              const uint32_t frame,
                                              JaniceDetection* detection)
{
    try {
        *detection = new JaniceDetectionType();

        (*detection)->track.length      = 1;
        (*detection)->track.rects       = new JaniceRect[(*detection)->track.length];
        (*detection)->track.confidences = new float[(*detection)->track.length];
        (*detection)->track.frames      = new uint32_t[(*detection)->track.length];

        (*detection)->track.rects[0]       = *rect;
        (*detection)->track.confidences[0] = 1.000000f;
        (*detection)->track.frames[0]      = frame;
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Failed to create detection from rect");

    return JANICE_SUCCESS;
}

JaniceError janice_create_detection_from_track(JaniceMediaIterator* /* media */,
                                               const JaniceTrack* track,
                                               JaniceDetection* detection)
{
    try {
        *detection = new JaniceDetectionType();

        (*detection)->track.length      = track->length;
        (*detection)->track.rects       = new JaniceRect[(*detection)->track.length];
        (*detection)->track.confidences = new float[(*detection)->track.length];
        (*detection)->track.frames      = new uint32_t[(*detection)->track.length];

        for (size_t i = 0; i < track->length; ++i) {
            (*detection)->track.rects[i]       = track->rects[i];
            (*detection)->track.confidences[i] = track->confidences[i];
            (*detection)->track.frames[i]      = track->frames[i];
        }
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Failed to create detection from track");

    return JANICE_SUCCESS;
}

JaniceError janice_detect(JaniceMediaIterator* it, const JaniceContext* context, JaniceDetections* detections)
{
    assert(_globals);

    assert(it != nullptr);
    assert(context != nullptr);
    assert(detections != nullptr);

    try {
        JaniceImage img;
        { // Get the next image from the iterator
            JaniceError err = it->next(it, &img);
            if (err != JANICE_SUCCESS) {
                std::cerr << "janice_detect => Failed to load image. Error: [" << janice_error_to_string(err) << "]" << std::endl;
                return err;
            }
        }

        if (img.channels != 3) {
            std::cerr << "janice_detect => Invalid image. Requires 3 channel BGR (opencv-style) images. Got: [" << img.channels << "] channel image" << std::endl;
            return JANICE_INVALID_MEDIA;
        }

        // Convert JanICE Image to DLib matrix
        dlib::matrix<dlib::rgb_pixel> mat(img.rows, img.cols);
        {
            const int row_step = img.cols * img.channels;
            const int col_step = img.channels;

            // Copy the data from img -> mat. img is in BGR order so we need to flip
            // the pixels
            for (size_t r = 0; r < img.rows; ++r) {
                for (size_t c = 0; c < img.cols; ++c) {
                    mat(r, c).red   = img.data[r * row_step + c * col_step + 2];
                    mat(r, c).green = img.data[r * row_step + c * col_step + 1];
                    mat(r, c).blue  = img.data[r * row_step + c * col_step + 0];
                }
            }
        }

        // DLibs minimum face size is ~40 pixels. Anything smaller than that and we
        // need to scale up the image
        if (context->min_object_size < 40) {
            dlib::pyramid_up(mat);
        }

        std::vector<dlib::mmod_rect> dets;
        try {
            dets = _globals->fd(mat);
        } catch (const std::exception& e) {
            std::cerr << "Face detection failed. Error: [" << e.what() << "]" << std::endl;
            return JANICE_UNKNOWN_ERROR;
        }

        detections->length = dets.size();
        detections->detections = new JaniceDetection[detections->length];

        for (size_t i = 0; i < dets.size(); ++i) {
            JaniceDetection detection = new JaniceDetectionType();

            detection->track.length = 1;
            detection->track.rects = new JaniceRect[detection->track.length];
            detection->track.frames = new uint32_t[detection->track.length];
            detection->track.confidences = new float[detection->track.length];

            JaniceRect rect;
            rect.x = dets[i].rect.left();
            rect.y = dets[i].rect.top();
            rect.width = dets[i].rect.width();
            rect.height = dets[i].rect.height();

            detection->track.rects[0] = rect;
            detection->track.frames[0] = 0;
            detection->track.confidences[0] = dets[i].detection_confidence;

            detections->detections[i] = detection;
        }
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Detection failed");

    return JANICE_SUCCESS;
}

JaniceError janice_detect_with_callback(JaniceMediaIterator* it, const JaniceContext* context, JaniceDetectionCallback callback, void* user_data)
{
    assert(_globals);

    assert(it != nullptr);
    assert(context != nullptr);
    assert(callback != nullptr);

    try {
        JaniceDetections detections;
        {
            JaniceError err = janice_detect(it, context, &detections);
            if (err != JANICE_SUCCESS) {
                janice_clear_detections(&detections);
                return err;
            }
        }

        { // Pass the detections to the callback
            for (size_t i = 0; i < detections.length; ++i) {
                JaniceError err = callback(&detections.detections[i], i, user_data);
                if (err == JANICE_CALLBACK_EXIT_IMMEDIATELY) {
                    janice_clear_detections(&detections);
                    return err;
                }
            }
        }
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Detection with callback failed");

    return JANICE_SUCCESS;
}

JaniceError janice_detect_batch(const JaniceMediaIterators* its, const JaniceContext* context, JaniceDetectionsGroup* group, JaniceErrors* errors)
{
    assert(_globals);

    assert(its != nullptr);
    assert(context != nullptr);
    assert(group != nullptr);
    assert(errors != nullptr);

    JaniceError return_code = JANICE_SUCCESS;
    try {
        group->length = its->length;
        group->group = new JaniceDetections[group->length];

        errors->length = its->length;
        errors->errors = new JaniceError[errors->length];

        for (size_t i = 0; i < its->length; ++i) {
            errors->errors[i] = janice_detect(&its->media[i], context, &group->group[i]);

            if (errors->errors[i] != JANICE_SUCCESS) {
                if (context->batch_policy == JaniceAbortEarly) {
                    errors->length = i + 1;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else { // JaniceFlagAndFinish
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                }
            }
        }
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Batch detection failed");

    return return_code;
}

JaniceError janice_detect_batch_with_callback(const JaniceMediaIterators* its, const JaniceContext* context, JaniceDetectionCallback callback, void* user_data, JaniceErrors* errors)
{
    assert(_globals);

    assert(its != nullptr);
    assert(context != nullptr);
    assert(callback != nullptr);
    assert(errors != nullptr);

    JaniceError return_code = JANICE_SUCCESS;
    try {
        errors->length = its->length;
        errors->errors = new JaniceError[errors->length];

        for (size_t i = 0; i < its->length; ++i) {
            JaniceDetections detections;
            errors->errors[i] = janice_detect(&its->media[i], context, &detections);

            if (errors->errors[i] != JANICE_SUCCESS) {
                if (context->batch_policy == JaniceAbortEarly) {
                    errors->length = i + 1;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else { // JaniceFlagAndFinish
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                }
            } else {
                for (size_t j = 0; j < detections.length; ++j) {
                    JaniceError callback_code = callback(&detections.detections[j], j, user_data);
                    if (callback_code == JANICE_CALLBACK_EXIT_IMMEDIATELY) {
                        janice_clear_detections(&detections);
                        return callback_code;
                    }
                }

                janice_clear_detections(&detections);
            }
        }
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Batch detection failed");

    return return_code;
}

JaniceError janice_detection_get_track(const JaniceDetection detection, JaniceTrack* track)
{
    try {
        track->length      = detection->track.length;
        track->rects       = new JaniceRect[track->length];
        track->frames      = new uint32_t[track->length];
        track->confidences = new float[track->length];

        for (size_t i = 0; i < track->length; ++i) {
            track->rects[i]       = detection->track.rects[i];
            track->frames[i]      = detection->track.frames[i];
            track->confidences[i] = detection->track.confidences[i];
        }
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Failed to get track from detection");

    return JANICE_SUCCESS;
}

JaniceError janice_detection_get_attribute(const JaniceDetection, const char*, char**)
{
    return JANICE_INVALID_ATTRIBUTE_KEY;
}

JaniceError janice_serialize_detection(const JaniceDetection detection, uint8_t** data, size_t* len)
{
    assert(detection != nullptr);

    try {
        *len = detection->track.length * 24;
        *data = new uint8_t[*len];

        uint8_t* ptr = *data;

        for (size_t i = 0; i < detection->track.length; ++i) {
            std::memcpy(ptr, (const char*) (&detection->track.rects[i].x), sizeof(int));
            ptr += sizeof(int);
            std::memcpy(ptr, (const char*) (&detection->track.rects[i].y), sizeof(int));
            ptr += sizeof(int);
            std::memcpy(ptr, (const char*) (&detection->track.rects[i].width), sizeof(int));
            ptr += sizeof(int);
            std::memcpy(ptr, (const char*) (&detection->track.rects[i].height), sizeof(int));
            ptr += sizeof(int);

            std::memcpy(ptr, (const char*) (&detection->track.frames[i]), sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            std::memcpy(ptr, (const char*) (&detection->track.confidences[i]), sizeof(float));
            ptr += sizeof(float);
        }
    }
    CATCH_AND_LOG(JANICE_FAILURE_TO_SERIALIZE, "Failed to serialize detection");

    return JANICE_SUCCESS;
}

JaniceError janice_deserialize_detection(const uint8_t* data, const size_t len, JaniceDetection* detection)
{
    assert(data != nullptr);

    if (len % 24 != 0) {
        std::cerr << "A serialized detection has to have a length that is a multiple of 24. Given length is: [" << len << "]" << std::endl;
        return JANICE_FAILURE_TO_DESERIALIZE;
    }

    try {
        JaniceTrack track;
        track.length = len / 24;
        track.rects = new JaniceRect[track.length];
        track.frames = new uint32_t[track.length];
        track.confidences = new float[track.length];

        uint8_t* ptr = const_cast<uint8_t*>(data);

        for (size_t i = 0; i < track.length; ++i) {
            std::memcpy((char*) (&track.rects[i].x), ptr, sizeof(int));
            ptr += sizeof(int);
            std::memcpy((char*) (&track.rects[i].y), ptr, sizeof(int));
            ptr += sizeof(int);
            std::memcpy((char*) (&track.rects[i].width), ptr, sizeof(int));
            ptr += sizeof(int);
            std::memcpy((char*) (&track.rects[i].height), ptr, sizeof(int));
            ptr += sizeof(int);

            std::memcpy((char*) (&track.frames[i]), ptr, sizeof(uint32_t));
            ptr += sizeof(uint32_t);

            std::memcpy((char*) (&track.confidences[i]), ptr, sizeof(float));
            ptr += sizeof(float);
        }

        *detection = new JaniceDetectionType();
        (*detection)->track = track;
    }
    CATCH_AND_LOG(JANICE_FAILURE_TO_DESERIALIZE, "Failed to deserialize");

    return JANICE_SUCCESS;
}

JaniceError janice_read_detection(const char* filename, JaniceDetection* detection)
{
    assert(filename != nullptr);

    try {
        FILE* file = fopen(filename, "rb");
        if (!file) {
            std::cerr << "Unable to open: [" << filename << "] for reading" << std::endl;
            return JANICE_BAD_ARGUMENT;
        }

        JaniceDetection _detection = new JaniceDetectionType();
        fread((char*) (&(_detection->track.length)), sizeof(size_t), 1, file);

        _detection->track.rects       = new JaniceRect[_detection->track.length];
        _detection->track.confidences = new float[_detection->track.length];
        _detection->track.frames      = new uint32_t[_detection->track.length];

        for (size_t i = 0; i < _detection->track.length; ++i) {
            fread((char*) (&(_detection->track.rects[i].x)), sizeof(int), 1, file);
            fread((char*) (&(_detection->track.rects[i].y)), sizeof(int), 1, file);
            fread((char*) (&(_detection->track.rects[i].width)), sizeof(int), 1, file);
            fread((char*) (&(_detection->track.rects[i].height)), sizeof(int), 1, file);

            fread((char*) (&(_detection->track.frames[i])), sizeof(uint32_t), 1, file);

            fread((char*) (&(_detection->track.confidences[i])), sizeof(float), 1, file);
        }

        fclose(file);

        *detection = _detection;
    }
    CATCH_AND_LOG(JANICE_READ_ERROR, "Failure to read detection");

    return JANICE_SUCCESS;
}

JaniceError janice_write_detection(const JaniceDetection detection, const char* filename)
{
    assert(detection != nullptr);
    assert(filename != nullptr);

    try {
        FILE* file = fopen(filename, "wb");
        if (!file) {
            std::cerr << "Unable to open file: [" << filename << "] for writing" << std::endl;
            return JANICE_BAD_ARGUMENT;
        }

        fwrite((const char*) (&(detection->track.length)), sizeof(size_t), 1, file);

        for (size_t i = 0; i < detection->track.length; ++i) {
            fwrite((const char*) (&(detection->track.rects[i].x)), sizeof(int), 1, file);
            fwrite((const char*) (&(detection->track.rects[i].y)), sizeof(int), 1, file);
            fwrite((const char*) (&(detection->track.rects[i].width)), sizeof(int), 1, file);
            fwrite((const char*) (&(detection->track.rects[i].height)), sizeof(int), 1, file);

            fwrite((const char*) (&(detection->track.frames[i])), sizeof(uint32_t), 1, file);

            fwrite((const char*) (&(detection->track.confidences[i])), sizeof(float), 1, file);
        }

        fclose(file);
    }
    CATCH_AND_LOG(JANICE_WRITE_ERROR, "Failed to write detection");

    return JANICE_SUCCESS;
}

JaniceError janice_free_detection(JaniceDetection* detection)
{
    if (detection && *detection) {
        janice_clear_track(&(*detection)->track);

        delete *detection;
        *detection = nullptr;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_detections(JaniceDetections* detections)
{
    if (detections) {
        for (size_t i = 0; i < detections->length; ++i) {
            janice_free_detection(&detections->detections[i]);
        }
        delete[] detections->detections;
        detections->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_detections_group(JaniceDetectionsGroup* group)
{
    if (group) {
        for (size_t i = 0; i < group->length; ++i) {
            janice_clear_detections(&group->group[i]);
        }
        delete[] group->group;
        group->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_track(JaniceTrack* track)
{
    if (track) {
        delete[] track->rects;
        delete[] track->frames;
        delete[] track->confidences;
        track->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_free_attribute(char**)
{
    return JANICE_SUCCESS;
}

JaniceError janice_fine_tune(const JaniceMediaIterator*, const JaniceDetectionsGroup*, int**, const char*)
{
    return JANICE_NOT_IMPLEMENTED;
}

struct JaniceTemplateType
{
    dlib::matrix<float,0,1> fv;

    int left_eye_x, left_eye_y;
    int right_eye_x, right_eye_y;
    int nose_x, nose_y;
};

JaniceError janice_enroll_from_media(JaniceMediaIterator* it, const JaniceContext* context, JaniceTemplates* tmpls, JaniceDetections* detections)
{
    assert(_globals);

    assert(it != nullptr);
    assert(context != nullptr);
    assert(tmpls != nullptr);
    assert(detections != nullptr);

    try {
        JaniceImage img;
        { // Get the next image from the iterator
            JaniceError err = it->next(it, &img);
            if (err != JANICE_SUCCESS) {
                std::cerr << "janice_detect => Failed to load image. Error: [" << janice_error_to_string(err) << "]" << std::endl;
                return err;
            }
        }

        if (img.channels != 3) {
            std::cerr << "janice_detect => Invalid image. Requires 3 channel BGR (opencv-style) images. Got: [" << img.channels << "] channel image" << std::endl;
            return JANICE_INVALID_MEDIA;
        }

        // Convert JanICE Image to DLib matrix
        dlib::matrix<dlib::rgb_pixel> mat(img.rows, img.cols);
        {
            const int row_step = img.cols * img.channels;
            const int col_step = img.channels;

            // Copy the data from img -> mat. img is in BGR order so we need to flip
            // the pixels
            for (size_t r = 0; r < img.rows; ++r) {
                for (size_t c = 0; c < img.cols; ++c) {
                    mat(r, c).red   = img.data[r * row_step + c * col_step + 2];
                    mat(r, c).green = img.data[r * row_step + c * col_step + 1];
                    mat(r, c).blue  = img.data[r * row_step + c * col_step + 0];
                }
            }
        }

        // DLibs minimum face size is ~40 pixels. Anything smaller than that and we
        // need to scale up the image
        if (context->min_object_size < 40) {
            dlib::pyramid_up(mat);
        }

        std::vector<dlib::mmod_rect> dets;
        try {
            dets = _globals->fd(mat);
        } catch (const std::exception& e) {
            std::cerr << "Face detection failed. Error: [" << e.what() << "]" << std::endl;
            return JANICE_UNKNOWN_ERROR;
        }

        detections->length = dets.size();
        detections->detections = new JaniceDetection[detections->length];

        tmpls->length = dets.size();
        tmpls->tmpls = new JaniceTemplate[tmpls->length];

        for (size_t i = 0; i < dets.size(); ++i) {
            JaniceDetection detection = new JaniceDetectionType();

            detection->track.length = 1;
            detection->track.rects = new JaniceRect[detection->track.length];
            detection->track.frames = new uint32_t[detection->track.length];
            detection->track.confidences = new float[detection->track.length];

            JaniceRect rect;
            rect.x = dets[i].rect.left();
            rect.y = dets[i].rect.top();
            rect.width = dets[i].rect.width();
            rect.height = dets[i].rect.height();

            detection->track.rects[0] = rect;
            detection->track.frames[0] = 0;
            detection->track.confidences[0] = dets[i].detection_confidence;

            detections->detections[i] = detection;
        }

        if (dets.empty()) {
            return JANICE_SUCCESS;
        }

        std::vector<dlib::matrix<dlib::rgb_pixel>> faces;
        for (size_t i = 0; i < dets.size(); ++i) {
            dlib::full_object_detection shape = _globals->sp(mat, dets[i].rect);
            dlib::matrix<dlib::rgb_pixel> face_chip;
            dlib::extract_image_chip(mat, dlib::get_face_chip_details(shape, 150, 0.25), face_chip);
            faces.push_back(std::move(face_chip));

            dlib::point left_eye_left_corner = shape.part(0);
            dlib::point left_eye_right_corner = shape.part(1);
        }

        std::vector<dlib::matrix<float,0,1>> feature_vectors = _globals->fe(faces);

        for (size_t i = 0; i < feature_vectors.size(); ++i) {
            tmpls->tmpls[i] = new JaniceTemplateType();
            tmpls->tmpls[i]->fv = feature_vectors[i];
        }
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Enroll from media failed");

    return JANICE_SUCCESS;
}

JaniceError janice_enroll_from_media_with_callback(JaniceMediaIterator* it, const JaniceContext* context, JaniceEnrollMediaCallback callback, void* user_data)
{
    assert(_globals);

    assert(it != nullptr);
    assert(context != nullptr);
    assert(callback != nullptr);

    try {
        JaniceDetections detections;
        JaniceTemplates tmpls;
        {
            JaniceError err = janice_enroll_from_media(it, context, &tmpls, &detections);
            if (err != JANICE_SUCCESS) {
                janice_clear_detections(&detections);
                janice_clear_templates(&tmpls);

                return err;
            }
        }

        for (size_t i = 0; i < detections.length; ++i) {
            JaniceError err = callback(&tmpls.tmpls[i], &detections.detections[i], i, user_data);
            if (err == JANICE_CALLBACK_EXIT_IMMEDIATELY) {
                janice_clear_detections(&detections);
                janice_clear_templates(&tmpls);

                return err;
            }
        }

        janice_clear_detections(&detections);
        janice_clear_templates(&tmpls);
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Enroll from media callback failed");

    return JANICE_SUCCESS;
}

JaniceError janice_enroll_from_media_batch(const JaniceMediaIterators* its, const JaniceContext* context, JaniceTemplatesGroup* tmpls_group, JaniceDetectionsGroup* detections_group, JaniceErrors* errors)
{
    assert(_globals);

    assert(its != nullptr);
    assert(context != nullptr);
    assert(tmpls_group != nullptr);
    assert(detections_group != nullptr);
    assert(errors != nullptr);

    JaniceError return_code = JANICE_SUCCESS;
    try {
        tmpls_group->length = its->length;
        tmpls_group->group = new JaniceTemplates[tmpls_group->length];

        detections_group->length = its->length;
        detections_group->group = new JaniceDetections[detections_group->length];

        errors->length = its->length;
        errors->errors = new JaniceError[errors->length];

        for (size_t i = 0; i < its->length; ++i) {
            errors->errors[i] = janice_enroll_from_media(&its->media[i], context, &tmpls_group->group[i], &detections_group->group[i]);

            if (errors->errors[i] != JANICE_SUCCESS) {
                if (context->batch_policy == JaniceAbortEarly) {
                    errors->length = i + 1;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else { // JaniceFlagAndFinish
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                }
            }
        }
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Enroll from media batch failed");

    return return_code;
}

JaniceError janice_enroll_from_media_batch_with_callback(const JaniceMediaIterators* its, const JaniceContext* context, JaniceEnrollMediaCallback callback, void* user_data,  JaniceErrors* errors)
{
    assert(_globals);

    assert(its != nullptr);
    assert(context != nullptr);
    assert(callback != nullptr);
    assert(errors != nullptr);

    JaniceError return_code = JANICE_SUCCESS;
    try {
        errors->length = its->length;
        errors->errors = new JaniceError[errors->length];

        for (size_t i = 0; i < its->length; ++i) {
            JaniceDetections detections;
            JaniceTemplates tmpls;
            {
                errors->errors[i] = janice_enroll_from_media(&its->media[i], context, &tmpls, &detections);
                if (errors->errors[i] != JANICE_SUCCESS) {
                    if (context->batch_policy == JaniceAbortEarly) {
                        janice_clear_detections(&detections);
                        janice_clear_templates(&tmpls);

                        errors->length = i + 1;
                        return JANICE_BATCH_ABORTED_EARLY;
                    } else { // JaniceFlagAndFinish
                        return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                    }
                }
            }

            for (size_t j = 0; j < detections.length; ++j) {
                JaniceError err = callback(&tmpls.tmpls[j], &detections.detections[j], j, user_data);
                if (err == JANICE_CALLBACK_EXIT_IMMEDIATELY) {
                    janice_clear_detections(&detections);
                    janice_clear_templates(&tmpls);

                    errors->length = i + 1;
                    return JANICE_BATCH_ABORTED_EARLY;
                }
            }
        }
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Enroll from media batch with callback");

    return return_code;
}

JaniceError janice_enroll_from_detections(const JaniceMediaIterators* its, const JaniceDetections* detections, const JaniceContext* context, JaniceTemplate* tmpl)
{
    assert(_globals);

    assert(its != nullptr);
    assert(detections != nullptr);
    assert(context != nullptr);

    try {
        if (its->length == 0 || detections->length == 0) {
            std::cerr << "No media given to janice_enroll_from_detections" << std::endl;
            return JANICE_SUCCESS;
        }

        std::vector<dlib::matrix<dlib::rgb_pixel>> faces;
        for (size_t i = 0; i < its->length; ++i) {
            JaniceImage img;
            { // Get the next image from the iterator
                JaniceMediaIterator it = its->media[i];

                JaniceError err = it.next(&it, &img);
                if (err != JANICE_SUCCESS) {
                    std::cerr << "janice_detect => Failed to load image. Error: [" << janice_error_to_string(err) << "]" << std::endl;
                    return err;
                }

                it.reset(&it);
            }

            if (img.channels != 3) {
                std::cerr << "janice_detect => Invalid image. Requires 3 channel BGR (opencv-style) images. Got: [" << img.channels << "] channel image" << std::endl;
                return JANICE_INVALID_MEDIA;
            }

            dlib::matrix<dlib::rgb_pixel> mat(img.rows, img.cols);
            {
                const int row_step = img.cols * img.channels;
                const int col_step = img.channels;

                // Copy the data from img -> mat. img is in BGR order so we need to flip
                // the pixels
                for (size_t r = 0; r < img.rows; ++r) {
                    for (size_t c = 0; c < img.cols; ++c) {
                        mat(r, c).red   = img.data[r * row_step + c * col_step + 2];
                        mat(r, c).green = img.data[r * row_step + c * col_step + 1];
                        mat(r, c).blue  = img.data[r * row_step + c * col_step + 0];
                    }
                }
            }

            JaniceTrack track = detections->detections[i]->track;
            JaniceRect  rect  = track.rects[0];

            dlib::rectangle drect(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);

            dlib::full_object_detection shape = _globals->sp(mat, drect);
            dlib::matrix<dlib::rgb_pixel> face_chip;
            dlib::extract_image_chip(mat, dlib::get_face_chip_details(shape, 150, 0.25), face_chip);
            faces.push_back(std::move(face_chip));

            //dlib::point left_eye_left_corner = shape.part(0);
            //dlib::point left_eye_right_corner = shape.part(1);
        }

        std::vector<dlib::matrix<float,0,1>> feature_vectors = _globals->fe(faces);

        dlib::matrix<float, 0, 1> mean = feature_vectors[0];
        for (size_t i = 1; i < feature_vectors.size(); ++i) {
            mean += feature_vectors[i];
        }

        *tmpl = new JaniceTemplateType();
        (*tmpl)->fv = (mean / feature_vectors.size());
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Enroll from media failed");

    return JANICE_SUCCESS;
}

JaniceError janice_enroll_from_detections_batch(const JaniceMediaIteratorsGroup* its_group, const JaniceDetectionsGroup* detections_group, const JaniceContext* context, JaniceTemplates* tmpls, JaniceErrors* errors)
{
    assert(_globals);

    assert(its_group != nullptr);
    assert(detections_group != nullptr);
    assert(context != nullptr);
    assert(tmpls != nullptr);
    assert(errors != nullptr);

    JaniceError return_code = JANICE_SUCCESS;

    if (its_group->length == 0 || detections_group->length == 0) {
        std::cerr << "No media given to janice_enroll_from_detections_batch" << std::endl;
        return JANICE_SUCCESS;
    }

    errors->length = its_group->length;
    errors->errors = new JaniceError[errors->length];

    std::vector<dlib::matrix<dlib::rgb_pixel>> faces;
    for (size_t i = 0; i < its_group->length; ++i) {
        for (size_t j = 0; j < its_group->group[i].length; ++j) {
            JaniceImage img;
            { // Get the next image from the iterator
                JaniceMediaIterator it = its_group->group[i].media[j];
                JaniceError err = it.next(&it, &img);
                if (err != JANICE_SUCCESS) {
                    std::cerr << "janice_detect => Failed to load image. Error in media: [" << i << "][" << j << "]. Error: [" << janice_error_to_string(err) << "]" << std::endl;

                    errors->errors[i] = err;
                    if (context->batch_policy == JaniceAbortEarly) {
                        return_code = JANICE_BATCH_ABORTED_EARLY;
                        errors->length = i;
                        return;
                    } else {
                        return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                        break;
                    }
                }

                it.reset(&it);
            }

            if (img.channels != 3) {
                std::cerr << "janice_detect => Invalid image. Requires 3 channel BGR (opencv-style) images. Got: [" << img.channels << "] channel image" << std::endl;

                errors->errors[i] = JANICE_INVALID_MEDIA;
                if (context->batch_policy == JaniceAbortEarly) {
                    return_code = JANICE_BATCH_ABORTED_EARLY;
                    errors->length = i;
                    return;
                } else {
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                    break;
                }
            }

            dlib::matrix<dlib::rgb_pixel> mat(img.rows, img.cols);
            {
                const int row_step = img.cols * img.channels;
                const int col_step = img.channels;

                // Copy the data from img -> mat. img is in BGR order so we need to flip
                // the pixels
                for (size_t r = 0; r < img.rows; ++r) {
                    for (size_t c = 0; c < img.cols; ++c) {
                        mat(r, c).red   = img.data[r * row_step + c * col_step + 2];
                        mat(r, c).green = img.data[r * row_step + c * col_step + 1];
                        mat(r, c).blue  = img.data[r * row_step + c * col_step + 0];
                    }
                }
            }

            JaniceTrack track = detections_group->group[i].detections[j]->track;
            JaniceRect  rect  = track.rects[0];

            dlib::rectangle drect(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);

            dlib::full_object_detection shape = _globals->sp(mat, drect);
            dlib::matrix<dlib::rgb_pixel> face_chip;
            dlib::extract_image_chip(mat, dlib::get_face_chip_details(shape, 150, 0.25), face_chip);
            faces.push_back(std::move(face_chip));

            //dlib::point left_eye_left_corner = shape.part(0);
            //dlib::point left_eye_right_corner = shape.part(1);
        }
    }

    std::vector<dlib::matrix<float,0,1>> feature_vectors = _globals->fe(faces);

    tmpls->length = its_group->length;
    tmpls->tmpls = new JaniceTemplate[tmpls->length];

    int idx = 0;
    for (size_t i = 0; its_group->length; ++i) {
        dlib::matrix<float, 0, 1> mean = feature_vectors[idx++];
        for (size_t j = 1; j < its_group->length; ++j) {
            mean += feature_vectors[idx++];
        }

        tmpls->tmpls[i] = new JaniceTemplateType();
        tmpls->tmpls[i]->fv = (mean / feature_vectors.size());
    }

    return return_code;
}

JaniceError janice_enroll_from_detections_batch_with_callback(const JaniceMediaIteratorsGroup* its_group, const JaniceDetectionsGroup* detections_group, const JaniceContext* context, JaniceEnrollDetectionsCallback callback, void* user_data, JaniceErrors* errors)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_template_is_fte(const JaniceTemplate tmpl, int* fte)
{
    assert(tmpl != nullptr);
    assert(fte != nullptr);

    *fte = 0; // never FTE

    return JANICE_SUCCESS;
}

JaniceError janice_template_get_attribute(const JaniceTemplate, const char*, char**)
{
    return JANICE_INVALID_ATTRIBUTE_KEY;
}

JaniceError janice_template_get_feature_vector(const JaniceTemplate, const JaniceFeatureVectorType, void**, size_t*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_serialize_template(const JaniceTemplate, uint8_t**, size_t*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_deserialize_template(const uint8_t*, const size_t, JaniceTemplate*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_read_template(const char*, JaniceTemplate*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_write_template(const JaniceTemplate, const char*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_free_template(JaniceTemplate*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_clear_templates(JaniceTemplates*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_clear_templates_group(JaniceTemplatesGroup*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_free_feature_vector(void**)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_verify(const JaniceTemplate, const JaniceTemplate, double*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_verify_batch(const JaniceTemplates*, const JaniceTemplates*, const JaniceContext*, JaniceSimilarities*, JaniceErrors*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_clear_similarities(JaniceSimilarities*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_clear_similarities_group(JaniceSimilaritiesGroup*)
{
    return JANICE_NOT_IMPLEMENTED;
}

struct JaniceGalleryType
{
    std::map<size_t, JaniceTemplateType> tmpls;
};

JaniceError janice_create_gallery(const JaniceTemplates*, const JaniceTemplateIds*, JaniceGallery*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_gallery_reserve(JaniceGallery, const size_t n)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_gallery_insert(JaniceGallery, const JaniceTemplate, const uint64_t)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_gallery_insert_batch(JaniceGallery, const JaniceTemplates*, const JaniceTemplateIds*, const JaniceContext*, JaniceErrors*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_gallery_remove(JaniceGallery, const uint64_t)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_gallery_remove_batch(JaniceGallery, const JaniceTemplateIds*, const JaniceContext*, JaniceErrors*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_gallery_prepare(JaniceGallery)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_serialize_gallery(const JaniceGallery, uint8_t**, size_t*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_deserialize_gallery(const uint8_t*, const size_t, JaniceGallery*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_read_gallery(const char*, JaniceGallery*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_write_gallery(const JaniceGallery, const char*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_free_gallery(JaniceGallery*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_clear_template_ids(JaniceTemplateIds*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_clear_template_ids_group(JaniceTemplateIdsGroup*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_search(const JaniceTemplate, const JaniceGallery, const JaniceContext*, JaniceSimilarities*, JaniceTemplateIds*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_search_batch(const JaniceTemplates*, const JaniceGallery, const JaniceContext*, JaniceSimilaritiesGroup*, JaniceTemplateIdsGroup*, JaniceErrors*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_cluster_media(const JaniceMediaIterators*, const JaniceContext*, JaniceClusterIdsGroup*, JaniceClusterConfidencesGroup*, JaniceDetectionsGroup*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_cluster_templates(const JaniceTemplates*, const JaniceContext*, JaniceClusterIds*, JaniceClusterConfidences*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_clear_cluster_ids(JaniceClusterIds*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_clear_cluster_ids_group(JaniceClusterIdsGroup*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_clear_cluster_confidences(JaniceClusterConfidences*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_clear_cluster_confidences_group(JaniceClusterConfidencesGroup*)
{
    return JANICE_NOT_IMPLEMENTED;
}

JaniceError janice_finalize()
{
    return JANICE_SUCCESS;
}
