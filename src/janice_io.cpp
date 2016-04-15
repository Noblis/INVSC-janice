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
#include <assert.h>

#include <janice_io.h>

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
        ENUM_CASE(INVALID_MEDIA)
        ENUM_CASE(MISSING_TEMPLATE_ID)
        ENUM_CASE(MISSING_FILE_NAME)
        ENUM_CASE(NULL_ATTRIBUTES)
        ENUM_CASE(MISSING_ATTRIBUTES)
        ENUM_CASE(FAILURE_TO_DETECT)
        ENUM_CASE(FAILURE_TO_ENROLL)
        ENUM_CASE(FAILURE_TO_SERIALIZE)
        ENUM_CASE(FAILURE_TO_DESERIALIZE)
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
    ENUM_COMPARE(INVALID_MEDIA, error)
    ENUM_COMPARE(MISSING_TEMPLATE_ID, error)
    ENUM_COMPARE(MISSING_FILE_NAME, error)
    ENUM_COMPARE(NULL_ATTRIBUTES, error)
    ENUM_COMPARE(MISSING_ATTRIBUTES, error)
    ENUM_COMPARE(FAILURE_TO_DETECT, error)
    ENUM_COMPARE(FAILURE_TO_ENROLL, error)
    ENUM_COMPARE(FAILURE_TO_SERIALIZE, error)
    ENUM_COMPARE(FAILURE_TO_DESERIALIZE, error)
    ENUM_COMPARE(NUM_ERRORS, error)
    ENUM_COMPARE(NOT_IMPLEMENTED, error)
    return JANICE_UNKNOWN_ERROR;
}

// For computing metrics
static vector<double> janice_load_media_samples;
static vector<double> janice_free_media_samples;
static vector<double> janice_detection_samples;
static vector<double> janice_create_template_samples;
static vector<double> janice_template_size_samples;
static vector<double> janice_serialize_template_samples;
static vector<double> janice_deserialize_template_samples;
static vector<double> janice_delete_serialized_template_samples;
static vector<double> janice_delete_template_samples;
static vector<double> janice_verify_samples;
static vector<double> janice_create_gallery_samples;
static vector<double> janice_gallery_size_samples;
static vector<double> janice_gallery_insert_samples;
static vector<double> janice_gallery_remove_samples;
static vector<double> janice_serialize_gallery_samples;
static vector<double> janice_deserialize_gallery_samples;
static vector<double> janice_delete_serialized_gallery_samples;
static vector<double> janice_delete_gallery_samples;
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

struct TemplateData
{
    vector<string> filenames;
    vector<janice_template_id> templateIDs;
    map<janice_template_id, int> subjectIDLUT;
    vector<janice_track> tracks;

    void release()
    {
        filenames.clear();
        templateIDs.clear();
        subjectIDLUT.clear();
        tracks.clear();
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
            string templateID, subjectID, filename, attributeValue;
            getline(attributeValues, templateID, ',');
            getline(attributeValues, subjectID, ',');
            getline(attributeValues, filename, ',');
            templateIDs.push_back(atoi(templateID.c_str()));
            subjectIDLUT.insert(make_pair(atoi(templateID.c_str()), atoi(subjectID.c_str())));
            filenames.push_back(filename);

            // Construct a track from the metadata
            janice_track track;
            janice_attributes attributes;
            for (int j = 0; getline(attributeValues, attributeValue, ','); j++) {
                double value = attributeValue.empty() ? NAN : atof(attributeValue.c_str());
                if (header[j] == "FRAME_NUMBER")
                    attributes.frame_number = value;
                else if (header[j] == "FACE_X")
                    attributes.face_x = value;
                else if (header[j] == "FACE_Y")
                    attributes.face_y = value;
                else if (header[j] == "FACE_WIDTH")
                    attributes.face_width = value;
                else if (header[j] == "FACE_HEIGHT")
                    attributes.face_height = value;
                else if (header[j] == "RIGHT_EYE_X")
                    attributes.right_eye_x = value;
                else if (header[j] == "RIGHT_EYE_Y")
                    attributes.right_eye_y = value;
                else if (header[j] == "LEFT_EYE_X")
                    attributes.left_eye_x = value;
                else if (header[j] == "LEFT_EYE_Y")
                    attributes.left_eye_y = value;
                else if (header[j] == "NOSE_BASE_X")
                    attributes.nose_base_x = value;
                else if (header[j] == "NOSE_BASE_Y")
                    attributes.nose_base_y = value;
                else if (header[j] == "FACE_YAW")
                    attributes.face_yaw = value;
                else if (header[j] == "FOREHEAD_VISIBLE")
                    attributes.forehead_visible = value;
                else if (header[j] == "EYES_VISIBLE")
                    attributes.eyes_visible = value;
                else if (header[j] == "NOSE_MOUTH_VISIBLE")
                    attributes.nose_mouth_visible = value;
                else if (header[j] == "INDOOR")
                    attributes.indoor = value;
                else if (header[j] == "GENDER")
                    track.gender = value;
                else if (header[j] == "AGE")
                    track.age = value;
                else if (header[j] == "SKIN_TONE")
                    track.skin_tone = value;
            }
            track.track.push_back(attributes);
            tracks.push_back(track);
        }
        if (verbose)
            fprintf(stderr, "\rEnrolling %zu/%zu", i, tracks.size());
    }

    TemplateData next()
    {
        TemplateData templateData;
        if (i >= tracks.size()) {
            fprintf(stderr, "\n");
        } else {
            const janice_template_id templateID = templateIDs[i];
            while ((i < tracks.size()) && (templateIDs[i] == templateID)) {
                templateData.templateIDs.push_back(templateIDs[i]);
                templateData.filenames.push_back(filenames[i]);
                templateData.tracks.push_back(tracks[i]);
                i++;
            }
            if (verbose)
                fprintf(stderr, "\rEnrolling %zu/%zu", i, tracks.size());
        }
        return templateData;
    }

    static janice_error create(const std::string &data_path, const TemplateData templateData, const janice_template_role role, janice_template *template_, janice_template_id *templateID, bool verbose)
    {
        clock_t start;

        // A set to hold all of the media and metadata required to make a full template
        vector<janice_association> associations;

        // Create a set of all the media used for this template
        for (size_t i = 0; i < templateData.templateIDs.size(); i++) {
            janice_media media;

            start = clock();
            JANICE_ASSERT(janice_load_media(data_path + templateData.filenames[i], media))
            _janice_add_sample(janice_load_media_samples, 1000.0 * (clock() - start) / CLOCKS_PER_SEC);

            janice_association association;
            association.media = media;
            association.metadata.push_back(templateData.tracks[i]);
            associations.push_back(association);
        }

        // Create the template
        start = clock();
        janice_error error = janice_create_template(associations, role, *template_);
        _janice_add_sample(janice_create_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

        // Check the result for errors
        if (error == JANICE_MISSING_ATTRIBUTES) {
            janice_missing_attributes_count++;
            if (verbose)
                printf("Missing attributes for: %s\n", templateData.filenames[0].c_str());
        } else if (error == JANICE_FAILURE_TO_ENROLL) {
            janice_failure_to_enroll_count++;
            if (verbose)
                printf("Failure to enroll: %s\n", templateData.filenames[0].c_str());
        } else if (error != JANICE_SUCCESS) {
            janice_other_errors_count++;
            printf("Warning: %s on: %s\n", janice_error_to_string(error),templateData.filenames[0].c_str());
        }

        // Free the media
        for (size_t i = 0; i < associations.size(); i++) {
            start = clock();
            JANICE_ASSERT(janice_free_media(associations[i].media));
            _janice_add_sample(janice_free_media_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);
        }

        *templateID = templateData.templateIDs[0];
        return JANICE_SUCCESS;
    }

    ~TemplateIterator() { release(); }
};

#ifndef JANICE_CUSTOM_CREATE_TEMPLATES

janice_error janice_create_templates_helper(const std::string &data_path, janice_metadata metadata, const std::string &templates_path, const std::string &templates_list_file, const janice_template_role role, bool verbose)
{
    clock_t start;

    // Create an iterator to loop through the templates
    TemplateIterator ti(metadata, true);

    // Preallocate some variables
    janice_template template_;
    janice_template_id templateID;

    // Set up file I/O
    ofstream templates_list_stream(templates_list_file.c_str(), std::ios::out | std::ios::ate);

    TemplateData templateData = ti.next();
    while (!templateData.templateIDs.empty()) {
        JANICE_CHECK(TemplateIterator::create(data_path, templateData, role, &template_, &templateID, verbose))

        // Serialize the template to a byte array. The array should be allocated by the function call
        janice_data *data = NULL; size_t data_len;
        start = clock();
        JANICE_CHECK(janice_serialize_template(template_, data, data_len));
        _janice_add_sample(janice_serialize_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

        // Add the template size to the metric logger
        _janice_add_sample(janice_template_size_samples, data_len / 1000.);

        // Useful strings
        char templateIDBuffer[10], subjectIDBuffer[10];
        sprintf(templateIDBuffer, "%zu", templateID);
        const string templateIDString(templateIDBuffer);
        sprintf(subjectIDBuffer, "%d", templateData.subjectIDLUT[templateID]);
        const string subjectIDString(subjectIDBuffer);
        const string templateOutputFile = templates_path + templateIDString + ".template";

        // Write the serialized template to disk
        ofstream template_stream(templateOutputFile.c_str(), ios::out | ios::binary);
        template_stream.write((char *)(&data_len), sizeof(size_t));
        template_stream.write((char *)data, data_len);
        template_stream.close();

        // Write the template metadata to the list
        templates_list_stream << templateIDString << "," << subjectIDString << "," << templateOutputFile << "\n";

        // Delete the serialized template
        start = clock();
        JANICE_CHECK(janice_delete_serialized_template(data, data_len));
        _janice_add_sample(janice_delete_serialized_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

        // Delete the actual template
        start = clock();
        JANICE_CHECK(janice_delete_template(template_));
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

static janice_error janice_load_templates_from_file(const std::string &templates_list_file, vector<janice_template> &templates, vector<janice_template_id> &template_ids, vector<int> &subject_ids)
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
        size_t data_len;
        template_stream.read((char *)(&data_len), sizeof(size_t));

        janice_data *data = new janice_data[data_len];
        template_stream.read((char *)data, data_len);
        if (!template_stream)
            return JANICE_UNKNOWN_ERROR;

        // Deserialize the template
        janice_template template_ = NULL;
        start = clock();
        JANICE_CHECK(janice_deserialize_template(data, data_len, template_));
        _janice_add_sample(janice_deserialize_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

        templates.push_back(template_);

        // Delete the serialized data (The harness does it since it allocated it)
        delete[] data;
    }
    templates_list_stream.close();

    return JANICE_SUCCESS;
}

#ifndef JANICE_CUSTOM_CREATE_GALLERY

janice_error janice_create_gallery_helper(const std::string &templates_list_file, const std::string &gallery_file, bool verbose)
{
    clock_t start;

    vector<janice_template> templates;
    vector<janice_template_id> template_ids;
    vector<int> subject_ids;
    JANICE_CHECK(janice_load_templates_from_file(templates_list_file, templates, template_ids, subject_ids));

    // Create the gallery
    janice_gallery gallery = NULL;
    start = clock();
    janice_create_gallery(templates, template_ids, gallery);
    _janice_add_sample(janice_create_gallery_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

    // Serialize the gallery to a byte array. The array should be allocated by the function call
    janice_data *data = NULL; size_t data_len;
    start = clock();
    JANICE_CHECK(janice_serialize_gallery(gallery, data, data_len));
    _janice_add_sample(janice_serialize_gallery_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

    // Add the gallery size to the metric logger (convert from bytes to KB)
    _janice_add_sample(janice_gallery_size_samples, data_len / 1000.);

    // Write the serialized gallery to disk
    ofstream gallery_stream(gallery_file.c_str(), ios::out | ios::binary);
    gallery_stream.write((char *)(&data_len), sizeof(size_t));
    gallery_stream.write((char *)data, data_len);
    gallery_stream.close();

    // Delete the serialized template
    start = clock();
    JANICE_CHECK(janice_delete_serialized_gallery(data, data_len));
    _janice_add_sample(janice_delete_serialized_gallery_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

    // Delete the actual gallery
    start = clock();
    JANICE_CHECK(janice_delete_gallery(gallery));
    _janice_add_sample(janice_delete_gallery_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

    if (verbose)
        janice_print_metrics(janice_get_metrics());

    return JANICE_SUCCESS;
}

#endif // JANICE_CUSTOM_CREATE_GALLERY

#ifndef JANICE_CUSTOM_VERIFY

janice_error janice_verify_helper(const std::string &templates_list_file_a, const std::string &templates_list_file_b, const std::string &scores_file, bool verbose)
{
    clock_t start;

    // Load the template sets
    vector<janice_template> templates_a, templates_b;
    vector<janice_template_id> template_ids_a, template_ids_b;
    vector<int> subject_ids_a, subject_ids_b;

    JANICE_CHECK(janice_load_templates_from_file(templates_list_file_a, templates_a, template_ids_a, subject_ids_a));
    JANICE_CHECK(janice_load_templates_from_file(templates_list_file_b, templates_b, template_ids_b, subject_ids_b));

    assert(templates_a.size() == templates_b.size());

    // Compare the templates and write the results to the scores file
    ofstream scores_stream(scores_file.c_str(), ios::out | ios::ate);
    for (size_t i = 0; i < templates_a.size(); i++) {
        double similarity;
        start = clock();
        janice_verify(templates_a[i], templates_b[i], similarity);
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

janice_error janice_search_helper(const std::string &probes_list_file, const std::string &gallery_list_file, const std::string &gallery_file, int num_requested_returns, const std::string &candidate_list_file, bool verbose)
{
    clock_t start;

    // Vectors to hold loaded data
    vector<janice_template> probe_templates, gallery_templates;
    vector<janice_template_id> probe_template_ids, gallery_template_ids;
    vector<int> probe_subject_ids, gallery_subject_ids;

    JANICE_CHECK(janice_load_templates_from_file(probes_list_file, probe_templates, probe_template_ids, probe_subject_ids));
    JANICE_CHECK(janice_load_templates_from_file(gallery_list_file, gallery_templates, gallery_template_ids, gallery_subject_ids))

    // Build template_id -> subject_id LUT for the gallery
    map<janice_template_id, int> subjectIDLUT;
    for (size_t i = 0; i < gallery_template_ids.size(); i++) {
        subjectIDLUT.insert(make_pair(gallery_template_ids[i], gallery_subject_ids[i]));

        start = clock();
        JANICE_CHECK(janice_delete_template(gallery_templates[i]))
        _janice_add_sample(janice_delete_template_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);
    }

    // Load the serialized gallery from disk
    ifstream gallery_stream(gallery_file.c_str(), ios::in | ios::binary);
    size_t data_len;
    gallery_stream.read((char *)(&data_len), sizeof(size_t));
    janice_data *data = new janice_data[data_len];
    gallery_stream.read((char *)data, data_len);
    if (!gallery_stream)
        return JANICE_UNKNOWN_ERROR;

    // Deserialize the template
    janice_gallery gallery = NULL;
    start = clock();
    JANICE_CHECK(janice_deserialize_gallery(data, data_len, gallery));
    _janice_add_sample(janice_deserialize_gallery_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

    // Delete the serialized data (The harness does it because it allocated it)
    delete[] data;

    ofstream candidate_stream(candidate_list_file.c_str(), ios::out | ios::ate);
    for (size_t i = 0; i < probe_templates.size(); i++) {
        vector<janice_template_id> gallery_template_ids;
        vector<double> similarities;
        start = clock();
        JANICE_CHECK(janice_search(probe_templates[i], gallery, num_requested_returns, gallery_template_ids, similarities));
        _janice_add_sample(janice_search_samples, 1000 * (clock() - start) / CLOCKS_PER_SEC);

        for (size_t j = 0; j < gallery_template_ids.size(); j++)
            candidate_stream << probe_template_ids[i] << "," << i << "," << gallery_template_ids[j] << "," << similarities[j]
                             << "," << (probe_subject_ids[i] == subjectIDLUT[gallery_template_ids[j]] ? "true" : "false") << "\n";

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

static janice_metric calculateMetric(const vector<double> &samples)
{
    janice_metric metric;
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
        metric.mean = std::numeric_limits<double>::quiet_NaN();
        metric.stddev = std::numeric_limits<double>::quiet_NaN();
    }

    return metric;
}

janice_metrics janice_get_metrics()
{
    janice_metrics metrics;
    metrics.janice_load_media_speed                 = calculateMetric(janice_load_media_samples);
    metrics.janice_free_media_speed                 = calculateMetric(janice_free_media_samples);
    metrics.janice_detection_speed                  = calculateMetric(janice_detection_samples);
    metrics.janice_create_template_speed            = calculateMetric(janice_create_template_samples);
    metrics.janice_template_size                    = calculateMetric(janice_template_size_samples);
    metrics.janice_serialize_template_speed         = calculateMetric(janice_serialize_template_samples);
    metrics.janice_deserialize_template_speed       = calculateMetric(janice_deserialize_template_samples);
    metrics.janice_delete_serialized_template_speed = calculateMetric(janice_delete_serialized_template_samples);
    metrics.janice_delete_template_speed            = calculateMetric(janice_delete_template_samples);
    metrics.janice_verify_speed                     = calculateMetric(janice_verify_samples);
    metrics.janice_create_gallery_speed             = calculateMetric(janice_create_gallery_samples);
    metrics.janice_gallery_size                     = calculateMetric(janice_gallery_size_samples);
    metrics.janice_gallery_insert_speed             = calculateMetric(janice_gallery_insert_samples);
    metrics.janice_gallery_remove_speed             = calculateMetric(janice_gallery_remove_samples);
    metrics.janice_serialize_gallery_speed          = calculateMetric(janice_serialize_gallery_samples);
    metrics.janice_deserialize_gallery_speed        = calculateMetric(janice_deserialize_gallery_samples);
    metrics.janice_delete_serialized_gallery_speed  = calculateMetric(janice_delete_serialized_gallery_samples);
    metrics.janice_delete_gallery_speed             = calculateMetric(janice_delete_gallery_samples);
    metrics.janice_search_speed                     = calculateMetric(janice_search_samples);
    metrics.janice_missing_attributes_count         = janice_missing_attributes_count;
    metrics.janice_failure_to_enroll_count          = janice_failure_to_enroll_count;
    metrics.janice_other_errors_count               = janice_other_errors_count;
    return metrics;
}

static void printMetric(FILE *file, const char *name, janice_metric metric, bool speed = true)
{
    if (metric.count > 0)
        fprintf(file, "%s\t%.2g\t%.2g\t%s\t%.2g\n", name, metric.mean, metric.stddev, speed ? "ms" : "KB", double(metric.count));
}

void janice_print_metrics(janice_metrics metrics)
{
    fprintf(stderr,     "API Symbol                      \tMean\tStdDev\tUnits\tCount\n");
    printMetric(stderr, "janice_load_media                ", metrics.janice_load_media_speed);
    printMetric(stderr, "janice_free_media                ", metrics.janice_free_media_speed);
    printMetric(stderr, "janice_detection                 ", metrics.janice_detection_speed);
    printMetric(stderr, "janice_create_template           ", metrics.janice_create_template_speed);
    printMetric(stderr, "janice_template_size             ", metrics.janice_template_size, false);
    printMetric(stderr, "janice_serialize_template        ", metrics.janice_serialize_template_speed);
    printMetric(stderr, "janice_deserialize_template      ", metrics.janice_deserialize_template_speed);
    printMetric(stderr, "janice_delete_serialized_template", metrics.janice_delete_serialized_template_speed);
    printMetric(stderr, "janice_delete_template           ", metrics.janice_delete_template_speed);
    printMetric(stderr, "janice_verify                    ", metrics.janice_verify_speed);
    printMetric(stderr, "janice_create_gallery            ", metrics.janice_create_gallery_speed);
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
    fprintf(stderr,     "janice_MISSING_ATTRIBUTES        \t%d\n", metrics.janice_missing_attributes_count);
    fprintf(stderr,     "janice_FAILURE_TO_ENROLL         \t%d\n", metrics.janice_failure_to_enroll_count);
    fprintf(stderr,     "All other errors                \t%d\n", metrics.janice_other_errors_count);
}
