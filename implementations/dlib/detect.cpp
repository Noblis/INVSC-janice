#include "types.hpp"
#include "utils.hpp"

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
    assert(janice_dlib::_globals);

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
            dets = janice_dlib::_globals->fd(mat);
        } catch (const std::exception& e) {
            std::cerr << "Face detection failed. Error: [" << e.what() << "]" << std::endl;
            return JANICE_UNKNOWN_ERROR;
        }

        // Filter the rectangles if the policy is Best or Largest
        if (context->policy == JaniceDetectBest) {
            std::sort(dets.begin(), dets.end(), [](const dlib::mmod_rect& r1, const dlib::mmod_rect& r2) {
                if (r1.detection_confidence == r2.detection_confidence) {
                    return r1.rect.left() < r2.rect.left();
                }

                return r1.detection_confidence > r2.detection_confidence;
            });
            dets.resize(1);
        } else if (context->policy == JaniceDetectLargest) {
            std::sort(dets.begin(), dets.end(), [](const dlib::mmod_rect& r1, const dlib::mmod_rect& r2) {
                if (r1.rect.area() == r2.rect.area()) {
                    return r1.rect.left() < r2.rect.left();
                }

                return r1.rect.area() > r2.rect.area();
            });
            dets.resize(1);
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
    assert(janice_dlib::_globals);

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

        janice_clear_detections(&detections);
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Detection with callback failed");

    return JANICE_SUCCESS;
}

JaniceError janice_detect_batch(const JaniceMediaIterators* its, const JaniceContext* context, JaniceDetectionsGroup* group, JaniceErrors* errors)
{
    assert(janice_dlib::_globals);

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
    assert(janice_dlib::_globals);

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
            }

            janice_clear_detections(&detections);
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
