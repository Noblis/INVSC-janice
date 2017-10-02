#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <fast-cpp-csv-parser/csv.h>

#include <unordered_map>

void print_usage()
{
    printf("Usage: janice_enroll_detections sdk_path temp_path data_path input_file role output_path [-algorithm <algorithm>] [-threads <int>] [-gpu <int>]\n");
}

int main(int argc, char* argv[])
{
    const int min_args = 7;
    const int max_args = 13;

    if (argc < min_args || argc > max_args) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    const std::string sdk_path    = argv[1];
    const std::string temp_path   = argv[2];
    const std::string data_path   = argv[3];
    const std::string input_file  = argv[4];
    const std::string role_str    = argv[5];
    const std::string output_file = argv[6];

    std::string algorithm;
    int num_threads, gpu;
    if (!parse_optional_args(argc, argv, min_args, max_args, algorithm, num_threads, gpu))
        exit(EXIT_FAILURE);
    
    // Check input
    if (strcmp(get_ext(input_file), "csv") != 0) {
        printf("input_file must be \".csv\" format.\n");
        exit(EXIT_FAILURE);
    }

    // Convert role
    JaniceEnrollmentType role;
    if      (role_str == "Reference")    role = Janice11Reference;
    else if (role_str == "Verification") role = Janice11Verification;
    else if (role_str == "Probe")        role = Janice1NProbe;
    else if (role_str == "Gallery")      role = Janice1NGallery;
    else if (role_str == "Cluster")      role = JaniceCluster;
    else {
        printf("Invalid template role. Valid roles are [Reference | Verification | Probe | Gallery | Cluster]\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the API
    // TODO: Right now we only allow a single GPU to be used
    JANICE_ASSERT(janice_initialize(sdk_path, temp_path, algorithm, num_threads, &gpu, 1))

    // Unused defaults for context parameters
    JaniceDetectionPolicy policy;
    uint32_t min_object_size = 0;
    double threshold = 0;
    uint32_t max_returns = 0;
    double hint = 0;

    JaniceContext context = nullptr;
    JANICE_ASSERT(janice_create_context(policy, min_object_size, role, threshold, max_returns, hint, &context))

    // Parse the metadata file
    io::CSVReader<7> metadata(input_file);
    metadata.read_header(io::ignore_extra_column, "FILENAME", "TEMPLATE_ID", "FRAME", "RECT_X", "RECT_Y", "RECT_WIDTH", "RECT_HEIGHT");

    std::unordered_map<int, std::vector<JaniceDetection>> detections_lut;

    // Load the metadata
    std::string filename;
    int template_id, frame;
    JaniceRect rect;
    while (metadata.read_row(filename, template_id, frame, rect.x, rect.y, rect.width, rect.height)) {
        JaniceMediaIterator media;
        JANICE_ASSERT(janice_io_opencv_create_media_iterator((std::string(data_path) + filename).c_str(), &media))

        JaniceDetection detection;
        JANICE_ASSERT(janice_create_detection_from_rect(media, rect, frame, &detection))

        if (detections_lut.find(template_id) == detections_lut.end()) {
            detections_lut.insert(std::make_pair(template_id, std::vector<JaniceDetection>{detection}));
        } else {
            detections_lut[template_id].push_back(detection);
        }

        JANICE_ASSERT(media->free(&media))
    }

    // Convert the LUT into a detections group object
    JaniceDetectionsGroup detections_group;
    detections_group.group = new JaniceDetections[detections_lut.size()];
    detections_group.length = detections_lut.size();

    std::vector<int> template_ids; // Store template ids to maintain a consistent ordering
    for (auto it = detections_lut.begin(), int i = 0; it != detections_lut.end(); ++it, ++i) {
        detections_group.group[i].detections = it.second.data();
        detections_group.group[i].length     = it.second.size();

        template_ids.push_back(it.first);
    }

    // Run batch enrollment
    JaniceTemplates tmpls;
    JANICE_ASSERT(janice_enroll_from_detections_batch(detections_group, context, &tmpls))

    // Assert we got the correct number of templates (1 list for each media)
    if (tmpls.length != detections_group.length) {
        printf("Incorrect return value. The number of templates should match the number of detection lists\n");
        exit(EXIT_FAILURE);
    }

    // Clear the detections
    for (size_t i = 0; i < detections_group.length; ++i)
        JANICE_ASSERT(janice_clear_detections(&detections_group.group[i]))
    
    delete[] detections_group.group;
    detections_group.group = nullptr;

    // Write the templates to disk
    FILE* output = fopen((output_path "/templates.csv").c_str(), "w+");
    fprintf(output, "FILENAME,TEMPLATE_ID\n");

    for (size_t i = 0; i < tmpls.length; ++i) {
        JaniceTemplate tmpl = tmpls[i];
    
        // Write the template to disk
        std::string tmpl_file = output_path + "/" + std::stoi(template_ids[i]) + ".tmpl";
        JANICE_ASSERT(janice_write_template(tmpl, tmpl_file.c_str()))

        fprintf(output, "%s,%d\n", tmpl_file.c_str(), template_ids[i]);
    }

    // Free the templates
    JANICE_ASSERT(janice_clear_templates(&tmpls))

    // Finalize the API
    JANICE_ASSERT(janice_finalize())

    return 0;
}
