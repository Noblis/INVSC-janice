#include "types.hpp"
#include "utils.hpp"

JaniceError janice_enroll_from_media(JaniceMediaIterator* it, const JaniceContext* context, JaniceTemplates* tmpls, JaniceDetections* detections)
{
    assert(janice_dlib::_globals);

    assert(it != nullptr);
    assert(context != nullptr);
    assert(tmpls != nullptr);
    assert(detections != nullptr);

    tmpls->length = 0;
    detections->length = 0;

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

        it->free_image(&img);

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
            dlib::full_object_detection shape = janice_dlib::_globals->sp(mat, dets[i].rect);
            dlib::matrix<dlib::rgb_pixel> face_chip;
            dlib::extract_image_chip(mat, dlib::get_face_chip_details(shape, 150, 0.25), face_chip);
            faces.push_back(std::move(face_chip));

            dlib::point left_eye_left_corner = shape.part(0);
            dlib::point left_eye_right_corner = shape.part(1);
        }

        std::vector<dlib::matrix<float,0,1>> feature_vectors = janice_dlib::_globals->fe(faces);

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
    assert(janice_dlib::_globals);

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
    assert(janice_dlib::_globals);

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

            janice_clear_detections(&detections);
            janice_clear_templates(&tmpls);
        }
    }
    CATCH_AND_LOG(JANICE_UNKNOWN_ERROR, "Enroll from media batch with callback");

    return return_code;
}

JaniceError janice_enroll_from_detections(const JaniceMediaIterators* its, const JaniceDetections* detections, const JaniceContext* context, JaniceTemplate* tmpl)
{
    assert(janice_dlib::_globals);

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

            its->media[i].free_image(&img);

            JaniceTrack track = detections->detections[i]->track;
            JaniceRect  rect  = track.rects[0];

            dlib::rectangle drect(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);

            dlib::full_object_detection shape = janice_dlib::_globals->sp(mat, drect);
            dlib::matrix<dlib::rgb_pixel> face_chip;
            dlib::extract_image_chip(mat, dlib::get_face_chip_details(shape, 150, 0.25), face_chip);
            faces.push_back(std::move(face_chip));

            //dlib::point left_eye_left_corner = shape.part(0);
            //dlib::point left_eye_right_corner = shape.part(1);
        }

        std::vector<dlib::matrix<float,0,1>> feature_vectors = janice_dlib::_globals->fe(faces);

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
    assert(janice_dlib::_globals);

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
                        errors->length = i;
                        return JANICE_BATCH_ABORTED_EARLY;
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
                    errors->length = i;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else {
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                    break;
                }
            }

            dlib::matrix<dlib::rgb_pixel> mat(img.rows, img.cols);
            try {
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
            } catch (const std::exception& e) {
                std::cerr << "Failed to convert JanICE image to DLIB matrix. Error: [" << e.what() << "]" << std::endl;

                errors->errors[i] = JANICE_INVALID_MEDIA;
                if (context->batch_policy == JaniceAbortEarly) {
                    errors->length = i;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else {
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                    break;
                }
            }

            its_group->group[i].media[j].free_image(&img);

            JaniceTrack track = detections_group->group[i].detections[j]->track;
            JaniceRect  rect  = track.rects[0];

            dlib::rectangle drect(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);

            try {
                dlib::full_object_detection shape = janice_dlib::_globals->sp(mat, drect);
                dlib::matrix<dlib::rgb_pixel> face_chip;
                dlib::extract_image_chip(mat, dlib::get_face_chip_details(shape, 150, 0.25), face_chip);
                faces.push_back(std::move(face_chip));
            } catch (const std::exception& e) {
                std::cerr << "Failed to extract and align face crop. Error: [" << e.what() << "]" << std::endl;

                errors->errors[i] = JANICE_UNKNOWN_ERROR;
                if (context->batch_policy == JaniceAbortEarly) {
                    errors->length = i;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else {
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                    break;
                }
            }

            //dlib::point left_eye_left_corner = shape.part(0);
            //dlib::point left_eye_right_corner = shape.part(1);
        }
    }

    try {
        std::vector<dlib::matrix<float,0,1>> feature_vectors = janice_dlib::_globals->fe(faces);

        tmpls->length = its_group->length;
        tmpls->tmpls = new JaniceTemplate[tmpls->length];

        int idx = 0;
        for (size_t i = 0; its_group->length; ++i) {
            dlib::matrix<float, 0, 1> mean = feature_vectors[idx++];
            for (size_t j = 1; j < its_group->group[i].length; ++j) {
                mean += feature_vectors[idx++];
            }

            tmpls->tmpls[i] = new JaniceTemplateType();
            tmpls->tmpls[i]->fv = (mean / its_group->group[i].length);
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to enroll all faces. Error: [" << e.what() << "]" << std::endl;

        for (size_t i = 0; i < errors->length; ++i) {
            errors->errors[i] = JANICE_UNKNOWN_ERROR;
        }

        return_code = context->batch_policy == JaniceAbortEarly ? JANICE_BATCH_ABORTED_EARLY : JANICE_BATCH_FINISHED_WITH_ERRORS;
    }

    return return_code;
}

JaniceError janice_enroll_from_detections_batch_with_callback(const JaniceMediaIteratorsGroup* its_group, const JaniceDetectionsGroup* detections_group, const JaniceContext* context, JaniceEnrollDetectionsCallback callback, void* user_data, JaniceErrors* errors)
{
    assert(janice_dlib::_globals);

    assert(its_group != nullptr);
    assert(detections_group != nullptr);
    assert(context != nullptr);
    assert(callback != nullptr);
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
                        errors->length = i;
                        return JANICE_BATCH_ABORTED_EARLY;
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
                    errors->length = i;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else {
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                    break;
                }
            }

            dlib::matrix<dlib::rgb_pixel> mat(img.rows, img.cols);
            try {
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
            } catch (const std::exception& e) {
                std::cerr << "Failed to convert JanICE image to DLIB matrix. Error: [" << e.what() << "]" << std::endl;

                errors->errors[i] = JANICE_INVALID_MEDIA;
                if (context->batch_policy == JaniceAbortEarly) {
                    errors->length = i;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else {
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                    break;
                }
            }

            JaniceTrack track = detections_group->group[i].detections[j]->track;
            JaniceRect  rect  = track.rects[0];

            dlib::rectangle drect(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);

            try {
                dlib::full_object_detection shape = janice_dlib::_globals->sp(mat, drect);
                dlib::matrix<dlib::rgb_pixel> face_chip;
                dlib::extract_image_chip(mat, dlib::get_face_chip_details(shape, 150, 0.25), face_chip);
                faces.push_back(std::move(face_chip));
            } catch (const std::exception& e) {
                std::cerr << "Failed to extract and align face crop. Error: [" << e.what() << "]" << std::endl;

                errors->errors[i] = JANICE_UNKNOWN_ERROR;
                if (context->batch_policy == JaniceAbortEarly) {
                    errors->length = i;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else {
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                    break;
                }
            }

            //dlib::point left_eye_left_corner = shape.part(0);
            //dlib::point left_eye_right_corner = shape.part(1);
        }
    }

    try {
        std::vector<dlib::matrix<float,0,1>> feature_vectors = janice_dlib::_globals->fe(faces);

        int idx = 0;
        for (size_t i = 0; its_group->length; ++i) {
            dlib::matrix<float, 0, 1> mean = feature_vectors[idx++];
            for (size_t j = 1; j < its_group->group[i].length; ++j) {
                mean += feature_vectors[idx++];
            }

            JaniceTemplate tmpl = new JaniceTemplateType();
            tmpl->fv = (mean / its_group->group[i].length);

            JaniceError err = callback(&tmpl, i, user_data);

            delete tmpl;

            if (err == JANICE_CALLBACK_EXIT_IMMEDIATELY) {
                errors->length = i;
                return err;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to enroll all faces. Error: [" << e.what() << "]" << std::endl;

        for (size_t i = 0; i < errors->length; ++i) {
            errors->errors[i] = JANICE_UNKNOWN_ERROR;
        }

        return_code = context->batch_policy == JaniceAbortEarly ? JANICE_BATCH_ABORTED_EARLY : JANICE_BATCH_FINISHED_WITH_ERRORS;
    }

    return return_code;
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

JaniceError janice_template_get_feature_vector(const JaniceTemplate tmpl, JaniceFeatureVectorType* fv, void** buffer, size_t* len)
{
    assert(tmpl != nullptr);
    assert(fv != nullptr);
    assert(len != nullptr);

    float* _buffer = nullptr;
    try {
        *fv = JaniceFloat;
        *len = tmpl->fv.nr();
        _buffer = new float[*len];

        memcpy(_buffer, tmpl->fv.begin(), *len * sizeof(float));

        *buffer = (void*) _buffer;
    } catch (const std::exception& e) {
        std::cerr << "Failed to get feature vector. Error: [" << e.what() << "]" << std::endl;

        if (_buffer) {
            delete[] _buffer;
        }

        return JANICE_UNKNOWN_ERROR;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_serialize_template(const JaniceTemplate tmpl, uint8_t** buffer, size_t* len)
{
    assert(tmpl != nullptr);
    assert(len != nullptr);

    float* _buffer = nullptr;
    try {
        _buffer = new float[tmpl->fv.nr()];
        *len = tmpl->fv.nr() * sizeof(float);

        memcpy(_buffer, tmpl->fv.begin(), *len);

        *buffer = (uint8_t*) _buffer;
    } catch (const std::exception& e) {
        std::cerr << "Failed to serialize template. Error: [" << e.what() << "]" << std::endl;

        if (_buffer) {
            delete[] _buffer;
        }

        return JANICE_UNKNOWN_ERROR;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_deserialize_template(const uint8_t* buffer, const size_t len, JaniceTemplate* tmpl)
{
    assert(buffer != nullptr);
    assert(len % sizeof(float) == 0); // Len should be divisible by the size of a float

    *tmpl = nullptr;
    try {
        *tmpl = new JaniceTemplateType();
        (*tmpl)->fv = dlib::matrix<float, 0, 1>(len / sizeof(float));

        memcpy((*tmpl)->fv.begin(), buffer, len);
    } catch (const std::exception& e) {
        std::cerr << "Failed to deserialize template. Error: [" << e.what() << "]" << std::endl;

        if ((*tmpl)) {
            delete *tmpl;
            *tmpl = nullptr;
        }

        return JANICE_UNKNOWN_ERROR;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_read_template(const char* filename, JaniceTemplate* tmpl)
{
    *tmpl = nullptr;
    try {
        *tmpl = new JaniceTemplateType();
        dlib::deserialize(filename) >> (*tmpl)->fv;
    } catch (const std::exception& e) {
        std::cerr << "Failed to read template. Error: [" << e.what() << "]" << std::endl;

        if ((*tmpl)) {
            delete *tmpl;
            *tmpl = nullptr;
        }

        return JANICE_READ_ERROR;
    }
    return JANICE_SUCCESS;
}

JaniceError janice_write_template(const JaniceTemplate tmpl, const char* filename)
{
    assert(tmpl != nullptr);

    try {
        dlib::serialize(filename) << tmpl->fv;
    } catch (const std::exception& e) {
        std::cerr << "Failed to write template. Error: [" << e.what() << "]" << std::endl;

        return JANICE_WRITE_ERROR;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_free_template(JaniceTemplate* tmpl)
{
    if (tmpl && *tmpl) {
        delete *tmpl;
        *tmpl = nullptr;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_templates(JaniceTemplates* tmpls)
{
    if (tmpls) {
        for (size_t i = 0; i < tmpls->length; ++i) {
            janice_free_template(&tmpls->tmpls[i]);
        }

        delete[] tmpls->tmpls;
        tmpls->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_templates_group(JaniceTemplatesGroup* tmpls_group)
{
    if (tmpls_group) {
        for (size_t i = 0; i < tmpls_group->length; ++i) {
            janice_clear_templates(&tmpls_group->group[i]);
        }

        delete[] tmpls_group->group;
        tmpls_group->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_free_feature_vector(void** fv)
{
    if (fv && *fv) {
        float* _fv = (float*) *fv;
        delete[] _fv;
    }

    return JANICE_SUCCESS;
}
