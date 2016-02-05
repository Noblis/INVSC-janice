// This file is designed to have no dependencies outside the C++ Standard Library
#include <algorithm>
#include <cmath>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <sstream>
#include <vector>

#include "../include/janice_io.h"

using namespace std;

#define ENUM_CASE(X) case JANICE_##X: return #X;
#define ENUM_COMPARE(X,Y) if (!strcmp(#X, Y)) return JANICE_##X;

const char *janice_error_to_string(janice_error error)
{
    switch (error) {
        ENUM_CASE(SUCCESS)
        ENUM_CASE(UNKNOWN_ERROR)
        ENUM_CASE(OUT_OF_MEMORY)
        ENUM_CASE(INVALID_SDK_PATH)
        ENUM_CASE(OPEN_ERROR)
        ENUM_CASE(READ_ERROR)
        ENUM_CASE(WRITE_ERROR)
        ENUM_CASE(PARSE_ERROR)
        ENUM_CASE(INVALID_IMAGE)
        ENUM_CASE(INVALID_VIDEO)
        ENUM_CASE(MISSING_TEMPLATE_ID)
        ENUM_CASE(MISSING_FILE_NAME)
        ENUM_CASE(NULL_ATTRIBUTES)
        ENUM_CASE(MISSING_ATTRIBUTES)
        ENUM_CASE(FAILURE_TO_DETECT)
        ENUM_CASE(FAILURE_TO_ENROLL)
        ENUM_CASE(NUM_ERRORS)
        ENUM_CASE(NOT_IMPLEMENTED)
    }
    return "UNKNOWN_ERROR";
}

janice_error janice_error_from_string(const char *error)
{
    ENUM_COMPARE(SUCCESS, error)
    ENUM_COMPARE(UNKNOWN_ERROR, error)
    ENUM_COMPARE(OUT_OF_MEMORY, error)
    ENUM_COMPARE(INVALID_SDK_PATH, error)
    ENUM_COMPARE(OPEN_ERROR, error)
    ENUM_COMPARE(READ_ERROR, error)
    ENUM_COMPARE(WRITE_ERROR, error)
    ENUM_COMPARE(PARSE_ERROR, error)
    ENUM_COMPARE(INVALID_IMAGE, error)
    ENUM_COMPARE(INVALID_VIDEO, error)
    ENUM_COMPARE(MISSING_TEMPLATE_ID, error)
    ENUM_COMPARE(MISSING_FILE_NAME, error)
    ENUM_COMPARE(NULL_ATTRIBUTES, error)
    ENUM_COMPARE(MISSING_ATTRIBUTES, error)
    ENUM_COMPARE(FAILURE_TO_DETECT, error)
    ENUM_COMPARE(FAILURE_TO_ENROLL, error)
    ENUM_COMPARE(NUM_ERRORS, error)
    ENUM_COMPARE(NOT_IMPLEMENTED, error)
    return JANICE_UNKNOWN_ERROR;
}

// For computing metrics
static vector<double> janice_initialize_template_samples;
static vector<double> janice_detection_samples;
static vector<double> janice_augment_samples;
static vector<double> janice_finalize_template_samples;
static vector<double> janice_finalize_gallery_samples;
static vector<double> janice_read_image_samples;
static vector<double> janice_free_image_samples;
static vector<double> janice_verify_samples;
static vector<double> janice_template_size_samples;
static vector<double> janice_gallery_size_samples;
static vector<double> janice_search_samples;
static int janice_missing_attributes_count = 0;
static int janice_failure_to_detect_count = 0;
static int janice_failure_to_enroll_count = 0;
static int janice_other_errors_count = 0;

static void _janice_add_sample(vector<double> &samples, double sample);

#ifndef JANICE_CUSTOM_ADD_SAMPLE

static void _janice_add_sample(vector<double> &samples, double sample)
{
    samples.push_back(sample);
}

#endif // JANICE_CUSTOM_ADD_SAMPLE

struct Rectangle
{
    double x, y, width, height;

    Rectangle()
        : x(0), y(0), width(0), height(0) {}
    Rectangle(double x_, double y_, double width_, double height_)
        : x(x_), y(y_), width(width_), height(height_) {}
    Rectangle(const Rectangle &other)
        : x(other.x), y(other.y), width(other.width), height(other.height) {}

    Rectangle& operator=(const Rectangle &rhs)
    {
        x = rhs.x;
        y = rhs.y;
        width = rhs.width;
        height = rhs.height;
        return *this;
    }

    Rectangle intersected(const Rectangle &other) const
    {
        double x_min = max(x, other.x);
        double x_max = min(x+width, other.x+other.width);
        if (x_max > x_min) {
            double y_min = max(y, other.y);
            double y_max = min(y+height, other.y+other.height);
            if (y_max > y_min)
                return Rectangle(x_min, y_min, x_max-x_min, y_max-y_min);
        }
        return Rectangle();
    }

    double area() const { return width * height; }
    double overlap (const Rectangle &other) const
    {
        const Rectangle intersection(intersected(other));
        return intersection.area() / (area() + other.area() - intersection.area());
    }
};

struct TemplateData
{
    vector<string> fileNames;
    vector<janice_template_id> templateIDs;
    map<janice_template_id, int> subjectIDLUT;
    vector<janice_attributes*> attributeLists;

    void release()
    {
        fileNames.clear();
        templateIDs.clear();
        subjectIDLUT.clear();
        for (size_t j=0; j<attributeLists.size(); j++)
            delete attributeLists[j];
        attributeLists.clear();
    }
};

struct TemplateIterator : public TemplateData
{
    size_t i;
    bool verbose;

    TemplateIterator(janice_metadata metadata, bool verbose)
        : i(0), verbose(verbose)
    {
        ifstream file(metadata);

        // Parse header
        string line;
        getline(file, line);
        istringstream attributeNames(line);
        string attributeName;
        getline(attributeNames, attributeName, ','); // TEMPLATE_ID
        getline(attributeNames, attributeName, ','); // SUBJECT_ID
        getline(attributeNames, attributeName, ','); // FILE_NAME
        vector<string> header;
        while (getline(attributeNames, attributeName, ','))
            header.push_back(attributeName);

        // Parse rows
        while (getline(file, line)) {
            istringstream attributeValues(line);
            string templateID, subjectID, fileName, attributeValue;
            getline(attributeValues, templateID, ',');
            getline(attributeValues, subjectID, ',');
            getline(attributeValues, fileName, ',');
            templateIDs.push_back(atoi(templateID.c_str()));
            subjectIDLUT.insert(pair<janice_template_id,int>(atoi(templateID.c_str()), atoi(subjectID.c_str())));
            fileNames.push_back(fileName);

            // Construct attribute list
            janice_attributes *attributes = new janice_attributes;
            for (int j=0; getline(attributeValues, attributeValue, ','); j++) {
                double value = attributeValue.empty() ? NAN : atof(attributeValue.c_str());
                if (header[j].compare(string("FRAME_RATE")) == 0)
                    attributes->frame_rate = value;
                else if (header[j].compare(string("FACE_X")) == 0)
                    attributes->face_x = value;
                else if (header[j].compare(string("FACE_Y")) == 0)
                    attributes->face_y = value;
                else if (header[j].compare(string("FACE_WIDTH")) == 0)
                    attributes->face_width = value;
                else if (header[j].compare(string("FACE_HEIGHT")) == 0)
                    attributes->face_height = value;
                else if (header[j].compare(string("RIGHT_EYE_X")) == 0)
                    attributes->right_eye_x = value;
                else if (header[j].compare(string("RIGHT_EYE_Y")) == 0)
                    attributes->right_eye_y = value;
                else if (header[j].compare(string("LEFT_EYE_X")) == 0)
                    attributes->left_eye_x = value;
                else if (header[j].compare(string("LEFT_EYE_Y")) == 0)
                    attributes->left_eye_y = value;
                else if (header[j].compare(string("NOSE_BASE_X")) == 0)
                    attributes->nose_base_x = value;
                else if (header[j].compare(string("NOSE_BASE_Y")) == 0)
                    attributes->nose_base_y = value;
                else if (header[j].compare(string("FACE_YAW")) == 0)
                    attributes->face_yaw = value;
                else if (header[j].compare(string("GENDER")) == 0)
                    attributes->gender = value;
                else if (header[j].compare(string("SKIN_TONE")) == 0)
                    attributes->skin_tone = value;
                else if (header[j].compare(string("AGE")) == 0)
                    attributes->age = value;
            }
            attributeLists.push_back(attributes);
        }
        if (verbose)
            fprintf(stderr, "\rEnrolling %zu/%zu", i, attributeLists.size());
    }

    TemplateData next()
    {
        TemplateData templateData;
        if (i >= attributeLists.size()) {
            fprintf(stderr, "\n");
        } else {
            const janice_template_id templateID = templateIDs[i];
            while ((i < attributeLists.size()) && (templateIDs[i] == templateID)) {
                templateData.templateIDs.push_back(templateIDs[i]);
                templateData.fileNames.push_back(fileNames[i]);
                janice_attributes *attributes = new janice_attributes;
                memcpy(attributes, attributeLists[i], sizeof(janice_attributes));
                templateData.attributeLists.push_back(attributes);
                i++;
            }
            if (verbose)
                fprintf(stderr, "\rEnrolling %zu/%zu", i, attributeLists.size());
        }
        return templateData;
    }

    static janice_error create(const char *data_path, const TemplateData templateData, janice_template *template_, janice_template_id *templateID, bool verbose)
    {
        clock_t start = clock();
        JANICE_CHECK(janice_allocate_template(template_))
        _janice_add_sample(janice_initialize_template_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

        const size_t num_requested_detections = 10;
        for (size_t i=0; i<templateData.templateIDs.size(); i++) {
            janice_attributes *attributes = new janice_attributes;
            memcpy(attributes, templateData.attributeLists[i], sizeof(janice_attributes));

            janice_image image;

            start = clock();
            JANICE_ASSERT(janice_read_image((data_path + templateData.fileNames[i]).c_str(), &image))
            _janice_add_sample(janice_read_image_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

            size_t num_actual_detections = 0;
            janice_attributes *attributes_array = new janice_attributes[num_requested_detections];

            start = clock();
            janice_error error = janice_detect(image, attributes_array, num_requested_detections, &num_actual_detections);
            _janice_add_sample(janice_detection_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);
            if (error == JANICE_FAILURE_TO_DETECT || num_actual_detections == 0) {
                janice_failure_to_detect_count++;
                if (verbose)
                    printf("Failure to detect face in %s\n", templateData.fileNames[i].c_str());
            } else if (error != JANICE_SUCCESS) {
                janice_other_errors_count++;
                printf("Warning: %s on: %s\n", janice_error_to_string(error),templateData.fileNames[i].c_str());
            } else {
                double overlap = 0.5;
                Rectangle truth(attributes->face_x, attributes->face_y, attributes->face_width, attributes->face_height);
                for (size_t j=0; j<min(num_requested_detections, num_actual_detections); j++) {
                    Rectangle detected(attributes_array[j].face_x, attributes_array[j].face_y, attributes_array[j].face_width, attributes_array[j].face_height);
                    if (detected.overlap(truth) > overlap) {
                        overlap = detected.overlap(truth);
                        attributes->face_x = detected.x;
                        attributes->face_y = detected.y;
                        attributes->face_width = detected.width;
                        attributes->face_height = detected.height;
                    }
                }
                if (overlap == 0.5) {
                    janice_failure_to_detect_count++;
                    if (verbose)
                        printf("Failure to detect face in %s\n", templateData.fileNames[i].c_str());
                }
            }
            delete[] attributes_array;

            start = clock();
            error = janice_augment(image, attributes, *template_);
            if (error == JANICE_MISSING_ATTRIBUTES) {
                janice_missing_attributes_count++;
                if (verbose)
                    printf("Missing attributes for: %s\n", templateData.fileNames[i].c_str());
            } else if (error == JANICE_FAILURE_TO_ENROLL) {
                janice_failure_to_enroll_count++;
                if (verbose)
                    printf("Failure to enroll: %s\n", templateData.fileNames[i].c_str());
            } else if (error != JANICE_SUCCESS) {
                janice_other_errors_count++;
                printf("Warning: %s on: %s\n", janice_error_to_string(error),templateData.fileNames[i].c_str());
            }
            _janice_add_sample(janice_augment_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

            start = clock();
            janice_free_image(image);
            _janice_add_sample(janice_free_image_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

            delete attributes;
        }

        *templateID = templateData.templateIDs[0];
        return JANICE_SUCCESS;
    }

    ~TemplateIterator() { release(); }
};

#ifndef JANICE_CUSTOM_CREATE_TEMPLATES

janice_error janice_create_templates(const char *data_path, janice_metadata metadata, const char *output_prefix, int verbose)
{
    TemplateIterator ti(metadata, true);
    janice_template template_;
    janice_template_id templateID;
    TemplateData templateData = ti.next();

    while (!templateData.templateIDs.empty()) {
        JANICE_CHECK(TemplateIterator::create(data_path, templateData, &template_, &templateID, verbose))
        JANICE_ASSERT(janice_finalize_template(template_))

        char template_path[1000];
        sprintf(template_path, "%s/%lu.template", output_prefix, templateID);
        JANICE_ASSERT(janice_write_template(template_, template_path));

        JANICE_ASSERT(janice_free_template(template_));
        templateData.release();
        templateData = ti.next();
    }

    return JANICE_SUCCESS;
}

#endif // JANICE_CUSTOM_CREATE_TEMPLATES

#ifndef JANICE_CUSTOM_CREATE_GALLERY

janice_error janice_create_gallery(const char *data_path, janice_metadata metadata, janice_gallery_path gallery_path, int verbose)
{
    TemplateIterator ti(metadata, true);
    janice_template template_;
    janice_template_id templateID;
    TemplateData templateData = ti.next();

    janice_gallery gallery;
    JANICE_ASSERT(janice_allocate_gallery(&gallery))

    while (!templateData.templateIDs.empty()) {
        JANICE_CHECK(TemplateIterator::create(data_path, templateData, &template_, &templateID, verbose))
        JANICE_ASSERT(janice_finalize_template(template_))
        JANICE_ASSERT(janice_enroll(template_, templateID, gallery))

        templateData.release();
        templateData = ti.next();
    }

    JANICE_ASSERT(janice_write_gallery(gallery, gallery_path))
    JANICE_ASSERT(janice_free_gallery(gallery))

    return JANICE_SUCCESS;
}

#endif // JANICE_CUSTOM_CREATE_GALLERY

static janice_metric calculateMetric(const vector<double> &samples)
{
    janice_metric metric;
    metric.count = samples.size();

    if (metric.count > 0) {
        metric.mean = 0;
        for (size_t i=0; i<samples.size(); i++)
            metric.mean += samples[i];
        metric.mean /= samples.size();

        metric.stddev = 0;
        for (size_t i=0; i<samples.size(); i++)
            metric.stddev += pow(samples[i] - metric.mean, 2.0);
        metric.stddev = sqrt(metric.stddev / samples.size());
    } else {
        metric.mean = std::numeric_limits<double>::quiet_NaN();
        metric.stddev = std::numeric_limits<double>::quiet_NaN();
    }

    return metric;
}

janice_metrics janice_get_metrics()
{
    janice_metrics metrics;
    metrics.janice_initialize_template_speed = calculateMetric(janice_initialize_template_samples);
    metrics.janice_detection_speed           = calculateMetric(janice_detection_samples);
    metrics.janice_augment_speed             = calculateMetric(janice_augment_samples);
    metrics.janice_finalize_template_speed   = calculateMetric(janice_finalize_template_samples);
    metrics.janice_read_image_speed          = calculateMetric(janice_read_image_samples);
    metrics.janice_free_image_speed          = calculateMetric(janice_free_image_samples);
    metrics.janice_verify_speed              = calculateMetric(janice_verify_samples);
    metrics.janice_gallery_size_speed        = calculateMetric(janice_gallery_size_samples);
    metrics.janice_finalize_gallery_speed    = calculateMetric(janice_finalize_gallery_samples);
    metrics.janice_search_speed              = calculateMetric(janice_search_samples);
    metrics.janice_template_size             = calculateMetric(janice_template_size_samples);
    metrics.janice_missing_attributes_count  = janice_missing_attributes_count;
    metrics.janice_failure_to_detect_count   = janice_failure_to_detect_count;
    metrics.janice_failure_to_enroll_count   = janice_failure_to_enroll_count;
    metrics.janice_other_errors_count        = janice_other_errors_count;
    return metrics;
}

static void printMetric(const char *name, janice_metric metric, bool speed = true)
{
    if (metric.count > 0)
        printf("%s\t%.2g\t%.2g\t%s\t%.2g\n", name, metric.mean, metric.stddev, speed ? "ms" : "KB", double(metric.count));
}

void janice_print_metrics(janice_metrics metrics)
{
    printf(     "API Symbol               \tMean\tStdDev\tUnits\tCount\n");
    printMetric("janice_initialize_template", metrics.janice_initialize_template_speed);
    printMetric("janice_detect             ", metrics.janice_detection_speed);
    printMetric("janice_augment            ", metrics.janice_augment_speed);
    printMetric("janice_finalize_template  ", metrics.janice_finalize_template_speed);
    printMetric("janice_read_image         ", metrics.janice_read_image_speed);
    printMetric("janice_free_image         ", metrics.janice_free_image_speed);
    printMetric("janice_verify             ", metrics.janice_verify_speed);
    printMetric("janice_gallery_size       ", metrics.janice_gallery_size_speed);
    printMetric("janice_finalize_gallery   ", metrics.janice_finalize_gallery_speed);
    printMetric("janice_search             ", metrics.janice_search_speed);
    printMetric("janice_flat_template      ", metrics.janice_template_size, false);
    printf("\n\n");
    printf("janice_error             \tCount\n");
    printf("JANICE_MISSING_ATTRIBUTES\t%d\n", metrics.janice_missing_attributes_count);
    printf("JANICE_FAILURE_TO_DETECT \t%d\n", metrics.janice_failure_to_detect_count);
    printf("JANICE_FAILURE_TO_ENROLL \t%d\n", metrics.janice_failure_to_enroll_count);
    printf("All other errors        \t%d\n", metrics.janice_other_errors_count);
}
