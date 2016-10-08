// These file is designed to have no dependencies outside the C++ Standard Library
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
#include <set>
#include <assert.h>

#include <janice_io.h>

using namespace std;

#define ENUM_CASE(X) case JANICE_##X: return #X;
#define ENUM_COMPARE(X,Y) if (!strcmp(#X, Y)) return JANICE_##X;

const char *janice_error_to_string(JaniceError error)
{
    switch (error) {
        ENUM_CASE(SUCCESS)
        ENUM_CASE(UNKNOWN_ERROR)
        ENUM_CASE(OUT_OF_MEMORY)
        ENUM_CASE(INVALID_SDK_PATH)
        ENUM_CASE(BAD_SDK_CONFIG)
        ENUM_CASE(BAD_LICENSE)
        ENUM_CASE(MISSING_DATA)
        ENUM_CASE(INVALID_GPU)
        ENUM_CASE(OPEN_ERROR)
        ENUM_CASE(READ_ERROR)
        ENUM_CASE(WRITE_ERROR)
        ENUM_CASE(PARSE_ERROR)
        ENUM_CASE(INVALID_MEDIA)
        ENUM_CASE(DUPLICATE_ID)
        ENUM_CASE(MISSING_ID)
        ENUM_CASE(MISSING_FILE_NAME)
        ENUM_CASE(INCORRECT_ROLE)
        ENUM_CASE(FAILURE_TO_ENROLL)
        ENUM_CASE(FAILURE_TO_SERIALIZE)
        ENUM_CASE(FAILURE_TO_DESERIALIZE)
        ENUM_CASE(NOT_SET)
        ENUM_CASE(NOT_IMPLEMENTED)
        ENUM_CASE(NUM_ERRORS)
    }
    return "UNKNOWN_ERROR";
}

JaniceError janice_error_from_string(const char *error)
{
    ENUM_COMPARE(SUCCESS, error)
    ENUM_COMPARE(UNKNOWN_ERROR, error)
    ENUM_COMPARE(OUT_OF_MEMORY, error)
    ENUM_COMPARE(INVALID_SDK_PATH, error)
    ENUM_COMPARE(BAD_SDK_CONFIG, error)
    ENUM_COMPARE(MISSING_DATA, error)
    ENUM_COMPARE(INVALID_GPU, error)
    ENUM_COMPARE(OPEN_ERROR, error)
    ENUM_COMPARE(READ_ERROR, error)
    ENUM_COMPARE(WRITE_ERROR, error)
    ENUM_COMPARE(PARSE_ERROR, error)
    ENUM_COMPARE(INVALID_MEDIA, error)
    ENUM_COMPARE(DUPLICATE_ID, error)
    ENUM_COMPARE(MISSING_ID, error)
    ENUM_COMPARE(MISSING_FILE_NAME, error)
    ENUM_COMPARE(INCORRECT_ROLE, error)
    ENUM_COMPARE(FAILURE_TO_ENROLL, error)
    ENUM_COMPARE(FAILURE_TO_SERIALIZE, error)
    ENUM_COMPARE(FAILURE_TO_DESERIALIZE, error)
    ENUM_COMPARE(NOT_SET, error)
    ENUM_COMPARE(NOT_IMPLEMENTED, error)
    ENUM_COMPARE(NUM_ERRORS, error)
    return JANICE_UNKNOWN_ERROR;
}

// For computing metrics
static vector<double> janice_load_media_samples;
static vector<double> janice_free_media_samples;
static vector<double> janice_detection_samples;
static vector<double> janice_get_rects_samples;
static vector<double> janice_get_confidence_samples;
static vector<double> janice_get_offset_samples;
static vector<double> janice_create_template_samples;
static vector<double> janice_template_size_samples;
static vector<double> janice_serialize_template_samples;
static vector<double> janice_deserialize_template_samples;
static vector<double> janice_delete_serialized_template_samples;
static vector<double> janice_delete_template_samples;
static vector<double> janice_verify_samples;
static vector<double> janice_create_gallery_samples;
static vector<double> janice_prepare_gallery_samples;
static vector<double> janice_gallery_size_samples;
static vector<double> janice_gallery_insert_samples;
static vector<double> janice_gallery_remove_samples;
static vector<double> janice_serialize_gallery_samples;
static vector<double> janice_deserialize_gallery_samples;
static vector<double> janice_delete_serialized_gallery_samples;
static vector<double> janice_delete_gallery_samples;
static vector<double> janice_search_samples;
static int janice_failure_to_enroll_count = 0;
static int janice_other_errors_count = 0;

static void _janice_add_sample(vector<double> &samples, double sample);

#ifndef JANICE_CUSTOM_ADD_SAMPLE

static void _janice_add_sample(vector<double> &samples, double sample)
{
    samples.push_back(sample);
}

#endif // JANICE_CUSTOM_ADD_SAMPLE

#ifndef JANICE_CUSTOM_DETECT

JaniceError janice_detect_helper(const string &data_path, JaniceMetadata metadata, const uint32_t min_face_size, const string &detection_list_file, bool verbose)
{
    clock_t start;

    ifstream file(metadata);
    ofstream output(detection_list_file);

    // Write the header
    output << "File,Frame,Face_X,Face_Y,Face_Width,Face_Height,Confidence\n";

    // Parse the header
    string line;
    getline(file, line);

    while (getline(file, line)) {
        istringstream attributes(line);
        string filename;
        getline(attributes, filename, ',');

        JaniceMedia media;

        start = clock();
        JANICE_ASSERT(janice_load_media(data_path + filename, media))
        _janice_add_sample(janice_load_media_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

        vector<JaniceDetection> detections;

        start = clock();
        JANICE_ASSERT(janice_detect(media, min_face_size, detections))
        _janice_add_sample(janice_detection_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

        for (size_t i = 0; i < detections.size(); i++) {
            const JaniceDetection &detection = detections[i];

            vector<JaniceRect> rects;
            start = clock();
            JANICE_ASSERT(janice_get_rects_from_detection(detection, rects))
            _janice_add_sample(janice_get_rects_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

            double confidence;
            start = clock();
            JANICE_ASSERT(janice_get_confidence_from_detection(detection, confidence))
            _janice_add_sample(janice_get_confidence_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

            int offset;
            start = clock();
            JANICE_ASSERT(janice_get_frame_offset_from_detection(detection, offset))
            _janice_add_sample(janice_get_offset_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

            for (size_t j = 0; j < rects.size(); j++) {
                const JaniceRect &rect = rects[j];
                output << filename               << ","
                       << to_string(offset + j)  << ","
                       << to_string(rect.x)      << ","
                       << to_string(rect.y)      << ","
                       << to_string(rect.width)  << ","
                       << to_string(rect.height) << ","
                       << to_string(confidence)  << "\n";
            }
        }

        janice_free_media(media);
    }

    file.close();
    output.close();

    if (verbose)
        janice_print_metrics(janice_get_metrics());

    return JANICE_SUCCESS;
}

#endif // JANICE_CUSTOM_DETECT

struct TemplateData
{
    vector<string> filenames;
    vector<JaniceTemplateId> templateIDs;
    map<JaniceTemplateId, int> subjectIDLUT;
    vector<JaniceRect> rects;
    vector<uint32_t> frames;

    void release()
    {
        filenames.clear();
        templateIDs.clear();
        subjectIDLUT.clear();
        rects.clear();
        frames.clear();
    }
};

struct TemplateIterator : public TemplateData
{
    size_t i;
    bool verbose;

    TemplateIterator(JaniceMetadata metadata, bool verbose)
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
            string templateID, subjectID, filename, attributeValue;
            getline(attributeValues, templateID, ',');
            getline(attributeValues, subjectID, ',');
            getline(attributeValues, filename, ',');
            templateIDs.push_back(atoi(templateID.c_str()));
            subjectIDLUT.insert(make_pair(atoi(templateID.c_str()), atoi(subjectID.c_str())));
            filenames.push_back(filename);

            // Construct a track from the metadata
            JaniceRect rect;
            uint32_t frame;

            for (int j = 0; getline(attributeValues, attributeValue, ','); j++) {
                double value = attributeValue.empty() ? NAN : atof(attributeValue.c_str());
                if (header[j] == "FACE_X")
                    rect.x = value;
                else if (header[j] == "FACE_Y")
                    rect.y = value;
                else if (header[j] == "FACE_WIDTH")
                    rect.width = value;
                else if (header[j] == "FACE_HEIGHT")
                    rect.height = value;
                else if (header[j] == "FRAME")
                    frame = value;
            }

            rects.push_back(rect);
            frames.push_back(frame);
        }
        if (verbose)
            fprintf(stderr, "\rEnrolling %zu/%zu", i, rects.size());
    }

    TemplateData next()
    {
        TemplateData templateData;
        if (i >= rects.size()) {
            fprintf(stderr, "\n");
        } else {
            const JaniceTemplateId templateID = templateIDs[i];
            while ((i < rects.size()) && (templateIDs[i] == templateID)) {
                templateData.templateIDs.push_back(templateIDs[i]);
                templateData.filenames.push_back(filenames[i]);
                templateData.rects.push_back(rects[i]);
                templateData.frames.push_back(frames[i]);
                i++;
            }
            if (verbose)
                fprintf(stderr, "\rEnrolling %zu/%zu", i, rects.size());
        }
        return templateData;
    }

    static JaniceError create(const string &data_path, const TemplateData templateData, const JaniceTemplateRole role, JaniceTemplate *tmpl, JaniceTemplateId *templateID, bool verbose)
    {
        clock_t start;

        // A set to hold all of the media and metadata required to make a full template
        vector<JaniceDetection> detections;

        // Create a set of all the media used for this template
        for (size_t i = 0; i < templateData.templateIDs.size(); i++) {
            JaniceMedia media;

            start = clock();
            JANICE_ASSERT(janice_load_media(data_path + templateData.filenames[i], media))
            _janice_add_sample(janice_load_media_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

            JaniceDetection detection;
            if (janice_create_detection(media, templateData.rects[i], templateData.frames[i], detection) != JANICE_SUCCESS)
                continue;

            detections.push_back(detection);

            start = clock();
            JANICE_ASSERT(janice_free_media(media))
            _janice_add_sample(janice_free_media_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);
        }

        // Create the template
        start = clock();
        JaniceError error = janice_create_template(detections, role, *tmpl);
        _janice_add_sample(janice_create_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

        // Check the result for errors
        if (error == JANICE_FAILURE_TO_ENROLL) {
            janice_failure_to_enroll_count++;
            if (verbose)
                printf("Failure to enroll: %s\n", templateData.filenames[0].c_str());
        } else if (error != JANICE_SUCCESS) {
            janice_other_errors_count++;
            printf("Warning: %s on: %s\n", janice_error_to_string(error),templateData.filenames[0].c_str());
        }

        *templateID = templateData.templateIDs[0];
        return JANICE_SUCCESS;
    }

    ~TemplateIterator() { release(); }
};

#ifndef JANICE_CUSTOM_CREATE_TEMPLATES

JaniceError janice_create_templates_helper(const string &data_path, JaniceMetadata metadata, const string &templates_path, const string &templates_list_file, const JaniceTemplateRole role, bool verbose)
{
    clock_t start;

    // Create an iterator to loop through the templates
    TemplateIterator ti(metadata, true);

    // Preallocate some variables
    JaniceTemplate tmpl;
    JaniceTemplateId templateID;

    // Set up file I/O
    ofstream templates_list_stream(templates_list_file.c_str(), ios::out | ios::ate);

    TemplateData templateData = ti.next();
    while (!templateData.templateIDs.empty()) {
        JANICE_CHECK(TemplateIterator::create(data_path, templateData, role, &tmpl, &templateID, verbose))

        // Useful strings
        char templateIDBuffer[10], subjectIDBuffer[10];
        sprintf(templateIDBuffer, "%u", templateID);
        const string templateIDString(templateIDBuffer);
        sprintf(subjectIDBuffer, "%d", templateData.subjectIDLUT[templateID]);
        const string subjectIDString(subjectIDBuffer);
        const string templateOutputFile = templates_path + templateIDString + ".template";

        // Serialize the template to a file.
        ofstream template_stream(templateOutputFile.c_str(), ios::out | ios::binary);
        start = clock();
        JANICE_CHECK(janice_serialize_template(tmpl, template_stream));
        _janice_add_sample(janice_serialize_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);
        template_stream.close();

        // Write the template metadata to the list
        templates_list_stream << templateIDString << "," << subjectIDString << "," << templateOutputFile << "\n";

        // Delete the template
        start = clock();
        JANICE_CHECK(janice_delete_template(tmpl));
        _janice_add_sample(janice_delete_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

        // Move to the next template
        templateData = ti.next();
    }
    templates_list_stream.close();

    if (verbose)
        janice_print_metrics(janice_get_metrics());

    return JANICE_SUCCESS;
}

#endif // JANICE_CUSTOM_CREATE_TEMPLATES

static JaniceError janice_load_templates_from_file(const string &templates_list_file, vector<JaniceTemplate> &tmpls, vector<JaniceTemplateId> &template_ids, vector<int> &subject_ids)
{
    clock_t start;

    ifstream templates_list_stream(templates_list_file.c_str());
    string line;

    while (getline(templates_list_stream, line)) {
        istringstream row(line);
        string template_id, subject_id, template_file;
        getline(row, template_id, ',');
        getline(row, subject_id, ',');
        getline(row, template_file, ',');

        template_ids.push_back(atoi(template_id.c_str()));
        subject_ids.push_back(atoi(subject_id.c_str()));

        // Load the serialized template from disk
        ifstream template_stream(template_file.c_str(), ios::in | ios::binary);
        JaniceTemplate tmpl = NULL;

        start = clock();
        JANICE_CHECK(janice_deserialize_template(tmpl, template_stream));
        _janice_add_sample(janice_deserialize_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);
        template_stream.close();

        tmpls.push_back(tmpl);
    }
    templates_list_stream.close();

    return JANICE_SUCCESS;
}

#ifndef JANICE_CUSTOM_CREATE_GALLERY

JaniceError janice_create_gallery_helper(const string &templates_list_file, const string &gallery_file, bool verbose)
{
    clock_t start;

    vector<JaniceTemplate> tmpls;
    vector<JaniceTemplateId> template_ids;
    vector<int> subject_ids;
    JANICE_CHECK(janice_load_templates_from_file(templates_list_file, tmpls, template_ids, subject_ids));

    // Create the gallery
    JaniceGallery gallery = NULL;
    start = clock();
    JANICE_CHECK(janice_create_gallery(tmpls, template_ids, gallery));
    _janice_add_sample(janice_create_gallery_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

    // Prepare the gallery for searching
    start = clock();
    janice_prepare_gallery(gallery);
    _janice_add_sample(janice_prepare_gallery_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

    // Serialize the gallery to a file.
    ofstream gallery_stream(gallery_file.c_str(), ios::out | ios::binary);
    start = clock();
    JANICE_CHECK(janice_serialize_gallery(gallery, gallery_stream));
    _janice_add_sample(janice_serialize_gallery_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);
    gallery_stream.close();

    // Delete the gallery
    start = clock();
    JANICE_CHECK(janice_delete_gallery(gallery));
    _janice_add_sample(janice_delete_gallery_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

    if (verbose)
        janice_print_metrics(janice_get_metrics());

    return JANICE_SUCCESS;
}

#endif // JANICE_CUSTOM_CREATE_GALLERY

#ifndef JANICE_CUSTOM_VERIFY

JaniceError janice_verify_helper(const string &templates_list_file_a, const string &templates_list_file_b, const string &scores_file, bool verbose)
{
    clock_t start;

    // Load the template sets
    vector<JaniceTemplate> tmpls_a, tmpls_b;
    vector<JaniceTemplateId> template_ids_a, template_ids_b;
    vector<int> subject_ids_a, subject_ids_b;

    JANICE_CHECK(janice_load_templates_from_file(templates_list_file_a, tmpls_a, template_ids_a, subject_ids_a));
    JANICE_CHECK(janice_load_templates_from_file(templates_list_file_b, tmpls_b, template_ids_b, subject_ids_b));

    assert(tmpls_a.size() == tmpls_b.size());

    // Compare the templates and write the results to the scores file
    ofstream scores_stream(scores_file.c_str(), ios::out | ios::ate);
    for (size_t i = 0; i < tmpls_a.size(); i++) {
        double similarity;
        start = clock();
        janice_verify(tmpls_a[i], tmpls_b[i], similarity);
        _janice_add_sample(janice_verify_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

        scores_stream << template_ids_a[i] << "," << template_ids_b[i] << "," << similarity << ","
                      << (subject_ids_a[i] == subject_ids_b[i] ? "true" : "false") << "\n";
    }
    scores_stream.close();

    if (verbose)
        janice_print_metrics(janice_get_metrics());

    return JANICE_SUCCESS;
}

#endif // JANICE_CUSTOM_VERIFY

#ifndef JANICE_CUSTOM_SEARCH

JaniceError janice_search_helper(const string &probes_list_file, const string &gallery_list_file, const string &gallery_file, int num_requested_returns, const string &candidate_list_file, bool verbose)
{
    clock_t start;

    // Vectors to hold loaded data
    vector<JaniceTemplate> probe_templates, gallery_templates;
    vector<JaniceTemplateId> probe_template_ids, gallery_template_ids;
    vector<int> probe_subject_ids, gallery_subject_ids;

    JANICE_CHECK(janice_load_templates_from_file(probes_list_file, probe_templates, probe_template_ids, probe_subject_ids));
    JANICE_CHECK(janice_load_templates_from_file(gallery_list_file, gallery_templates, gallery_template_ids, gallery_subject_ids))

    // Build template_id -> subject_id LUT for the gallery
    map<JaniceTemplateId, int> subjectIDLUT;
    for (size_t i = 0; i < gallery_template_ids.size(); i++) {
        subjectIDLUT.insert(make_pair(gallery_template_ids[i], gallery_subject_ids[i]));

        start = clock();
        JANICE_CHECK(janice_delete_template(gallery_templates[i]))
        _janice_add_sample(janice_delete_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);
    }

    // Load the serialized gallery from disk
    ifstream gallery_stream(gallery_file.c_str(), ios::in | ios::binary);
    JaniceGallery gallery = NULL;
    start = clock();
    JANICE_CHECK(janice_deserialize_gallery(gallery, gallery_stream));
    _janice_add_sample(janice_deserialize_gallery_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

    ofstream candidate_stream(candidate_list_file.c_str(), ios::out | ios::ate);
    for (size_t i = 0; i < probe_templates.size(); i++) {
        vector<JaniceTemplateId> return_template_ids;
        vector<double> similarities;
        start = clock();
        JANICE_CHECK(janice_search(probe_templates[i], gallery, num_requested_returns, return_template_ids, similarities));
        _janice_add_sample(janice_search_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

        for (size_t j = 0; j < return_template_ids.size(); j++)
            candidate_stream << probe_template_ids[i] << "," << j << "," << return_template_ids[j] << "," << similarities[j]
                             << "," << (probe_subject_ids[i] == subjectIDLUT[return_template_ids[j]] ? "true" : "false") << "\n";

        start = clock();
        JANICE_CHECK(janice_delete_template(probe_templates[i]))
        _janice_add_sample(janice_delete_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);
    }
    candidate_stream.close();

    if (verbose)
        janice_print_metrics(janice_get_metrics());

    return JANICE_SUCCESS;
}

#endif // JANICE_CUSTOM_SEARCH

static JaniceMetric calculateMetric(const vector<double> &samples)
{
    JaniceMetric metric;
    metric.count = samples.size();

    if (metric.count > 0) {
        metric.mean = 0;
        for (size_t i = 0; i < samples.size(); i++)
            metric.mean += samples[i];
        metric.mean /= samples.size();

        metric.stddev = 0;
        for (size_t i = 0; i < samples.size(); i++)
            metric.stddev += pow(samples[i] - metric.mean, 2.0);
        metric.stddev = sqrt(metric.stddev / samples.size());
    } else {
        metric.mean = numeric_limits<double>::quiet_NaN();
        metric.stddev = numeric_limits<double>::quiet_NaN();
    }

    return metric;
}

JaniceMetrics janice_get_metrics()
{
    JaniceMetrics metrics;
    metrics.janice_load_media_speed                 = calculateMetric(janice_load_media_samples);
    metrics.janice_free_media_speed                 = calculateMetric(janice_free_media_samples);
    metrics.janice_detection_speed                  = calculateMetric(janice_detection_samples);
    metrics.janice_get_rects_speed                  = calculateMetric(janice_get_rects_samples);
    metrics.janice_get_confidence_speed             = calculateMetric(janice_get_confidence_samples);
    metrics.janice_get_offset_speed                 = calculateMetric(janice_get_offset_samples);
    metrics.janice_create_template_speed            = calculateMetric(janice_create_template_samples);
    metrics.janice_template_size                    = calculateMetric(janice_template_size_samples);
    metrics.janice_serialize_template_speed         = calculateMetric(janice_serialize_template_samples);
    metrics.janice_deserialize_template_speed       = calculateMetric(janice_deserialize_template_samples);
    metrics.janice_delete_serialized_template_speed = calculateMetric(janice_delete_serialized_template_samples);
    metrics.janice_delete_template_speed            = calculateMetric(janice_delete_template_samples);
    metrics.janice_verify_speed                     = calculateMetric(janice_verify_samples);
    metrics.janice_create_gallery_speed             = calculateMetric(janice_create_gallery_samples);
    metrics.janice_prepare_gallery_speed            = calculateMetric(janice_prepare_gallery_samples);
    metrics.janice_gallery_size                     = calculateMetric(janice_gallery_size_samples);
    metrics.janice_gallery_insert_speed             = calculateMetric(janice_gallery_insert_samples);
    metrics.janice_gallery_remove_speed             = calculateMetric(janice_gallery_remove_samples);
    metrics.janice_serialize_gallery_speed          = calculateMetric(janice_serialize_gallery_samples);
    metrics.janice_deserialize_gallery_speed        = calculateMetric(janice_deserialize_gallery_samples);
    metrics.janice_delete_serialized_gallery_speed  = calculateMetric(janice_delete_serialized_gallery_samples);
    metrics.janice_delete_gallery_speed             = calculateMetric(janice_delete_gallery_samples);
    metrics.janice_search_speed                     = calculateMetric(janice_search_samples);
    metrics.janice_failure_to_enroll_count          = janice_failure_to_enroll_count;
    metrics.janice_other_errors_count               = janice_other_errors_count;
    return metrics;
}

static void printMetric(FILE *file, const char *name, JaniceMetric metric, bool speed = true)
{
    if (metric.count > 0)
        fprintf(file, "%s\t%.2g\t%.2g\t%s\t%.2g\n", name, metric.mean, metric.stddev, speed ? "ms" : "KB", double(metric.count));
}

void janice_print_metrics(JaniceMetrics metrics)
{
    fprintf(stderr,     "API Symbol                      \tMean\tStdDev\tUnits\tCount\n");
    printMetric(stderr, "janice_load_media                ", metrics.janice_load_media_speed);
    printMetric(stderr, "janice_free_media                ", metrics.janice_free_media_speed);
    printMetric(stderr, "janice_detection                 ", metrics.janice_detection_speed);
    printMetric(stderr, "janice_get_rects                 ", metrics.janice_get_rects_speed);
    printMetric(stderr, "janice_get_confidence            ", metrics.janice_get_confidence_speed);
    printMetric(stderr, "janice_get_offset                ", metrics.janice_get_offset_speed);
    printMetric(stderr, "janice_create_template           ", metrics.janice_create_template_speed);
    printMetric(stderr, "janice_template_size             ", metrics.janice_template_size, false);
    printMetric(stderr, "janice_serialize_template        ", metrics.janice_serialize_template_speed);
    printMetric(stderr, "janice_deserialize_template      ", metrics.janice_deserialize_template_speed);
    printMetric(stderr, "janice_delete_serialized_template", metrics.janice_delete_serialized_template_speed);
    printMetric(stderr, "janice_delete_template           ", metrics.janice_delete_template_speed);
    printMetric(stderr, "janice_verify                    ", metrics.janice_verify_speed);
    printMetric(stderr, "janice_create_gallery            ", metrics.janice_create_gallery_speed);
    printMetric(stderr, "janice_prepare_gallery           ", metrics.janice_prepare_gallery_speed);
    printMetric(stderr, "janice_gallery_size              ", metrics.janice_gallery_size, false);
    printMetric(stderr, "janice_gallery_insert            ", metrics.janice_gallery_insert_speed);
    printMetric(stderr, "janice_gallery_remove            ", metrics.janice_gallery_remove_speed);
    printMetric(stderr, "janice_serialize_gallery         ", metrics.janice_serialize_gallery_speed);
    printMetric(stderr, "janice_deserialize_gallery       ", metrics.janice_deserialize_gallery_speed);
    printMetric(stderr, "janice_delete_serialized_gallery ", metrics.janice_delete_serialized_gallery_speed);
    printMetric(stderr, "janice_delete_gallery            ", metrics.janice_delete_gallery_speed);
    printMetric(stderr, "janice_search                    ", metrics.janice_search_speed);
    fprintf(stderr,     "\n\n");
    fprintf(stderr,     "janice_error                     \tCount\n");
    fprintf(stderr,     "JANICE_FAILURE_TO_ENROLL         \t%d\n", metrics.janice_failure_to_enroll_count);
    fprintf(stderr,     "All other errors                \t%d\n", metrics.janice_other_errors_count);
}
