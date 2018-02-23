#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <fast-cpp-csv-parser/csv.h>

#include <unordered_map>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
    std::string output_path = argv[6];

    std::string algorithm;
    int num_threads = 0;
    int gpu = 0;
    if (!parse_optional_args(argc, argv, min_args, max_args, algorithm, num_threads, gpu))
        exit(EXIT_FAILURE);
    
    // Check input
    if (get_ext(input_file) != "csv") {
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
    JANICE_ASSERT(janice_initialize(sdk_path.c_str(), temp_path.c_str(), algorithm.c_str(), num_threads, &gpu, 1));

    // Unused defaults for context parameters
    JaniceDetectionPolicy policy = JaniceDetectAll;
    uint32_t min_object_size = 0;
    double threshold = 0;
    uint32_t max_returns = 0;
    double hint = 0;

    JaniceContext context = nullptr;
    JANICE_ASSERT(janice_create_context(policy, min_object_size, role, threshold, max_returns, hint, &context));

    // Parse the metadata file
    io::CSVReader<7> metadata(input_file);
    metadata.read_header(io::ignore_extra_column, "FILENAME", "TEMPLATE_ID", "SUBJECT_ID", "FACE_X", "FACE_Y", "FACE_WIDTH", "FACE_HEIGHT");

    std::unordered_map<int, std::vector<JaniceDetection>> detections_lut;
    std::unordered_map<int, int> subject_id_lut;

    // Load the metadata
    std::string filename;
    int template_id, subject_id;
    JaniceRect rect;
    while (metadata.read_row(filename, template_id, subject_id, rect.x, rect.y, rect.width, rect.height)) {
        JaniceMediaIterator media;
        JANICE_ASSERT(janice_io_opencv_create_media_iterator((std::string(data_path) + filename).c_str(), &media));

        JaniceDetection detection;
        JANICE_ASSERT(janice_create_detection_from_rect(media, rect, 0, &detection));

        if (detections_lut.find(template_id) == detections_lut.end()) {
            detections_lut.insert(std::make_pair(template_id, std::vector<JaniceDetection>{detection}));
        } else {
            detections_lut[template_id].push_back(detection);
        }

        subject_id_lut[template_id] = subject_id;

        JANICE_ASSERT(media->free(&media));
    }

    // Convert the LUT into a detections group object
    JaniceDetectionsGroup detections_group;
    detections_group.group = new JaniceDetections[detections_lut.size()];
    detections_group.length = detections_lut.size();

    std::vector<int> template_ids; // Store template ids to maintain a consistent ordering

    size_t idx = 0; // map template id range to 0-N
    for (auto entry : detections_lut) {
        detections_group.group[idx].length   = entry.second.size();
        detections_group.group[idx].detections = new JaniceDetection[entry.second.size()];
        for (size_t i = 0; i < entry.second.size(); ++i)
            detections_group.group[idx].detections[i] = entry.second[i];
        ++idx;

        template_ids.push_back(entry.first);
    }

    // Run batch enrollment
    JaniceTemplates tmpls;
    JANICE_ASSERT(janice_enroll_from_detections_batch(detections_group, context, &tmpls));

    // Assert we got the correct number of templates (1 list for each media)
    if (tmpls.length != detections_group.length) {
        printf("Incorrect return value. The number of templates should match the number of detection lists\n");
        exit(EXIT_FAILURE);
    }

    janice_clear_detections_group(&detections_group);
#if 0
    // Clear the detections
    for (size_t i = 0; i < detections_group.length; ++i) {
        for (size_t j = 0; j < detections_group.group[i].length; ++j)
          JANICE_ASSERT(janice_free_detection(&detections_group.group[i].detections[j]));
        delete[] detections_group.group[i].detections;
    }
    delete[] detections_group.group;
#endif

    // Write the templates to disk
    struct stat stat_buf;
    if (*(output_path.end()) == '/') {
      output_path = output_path.substr(0, output_path.length() - 1);
    }
    std::string output_file_name(output_path);
    // taa: If we were given the name of an existing directory, we'll append "templates.csv"
    // and use that. Otherwise, we take the output_path as a file name, and use it unmodified.
    if (stat(output_path.c_str(), &stat_buf) == 0 &&
        (stat_buf.st_mode & S_IFDIR) != 0) {
      output_file_name = output_path + "/templates.csv";
    }
    else {
      size_t last_slash = output_path.rfind("/");
      // We need the directory part to write the templates.
      output_path = output_path.substr(0, last_slash);
    }
    FILE* output = fopen(output_file_name.c_str(), "w+");
    // taa: Use old-style headers.
    fprintf(output, "FILENAME,TEMPLATE_ID,SUBJECT_ID\n");

    for (size_t i = 0; i < tmpls.length; ++i) {
        std::string tmpl_file = output_path + "/" + std::to_string(template_ids[i]) + ".tmpl";
        JANICE_ASSERT(janice_write_template(tmpls.tmpls[i], tmpl_file.c_str()));

        fprintf(output, "%s,%d,%d\n", tmpl_file.c_str(), template_ids[i], subject_id_lut[template_ids[i]]);
    }

    // Close the file
    fclose(output);

    // Free the templates
    JANICE_ASSERT(janice_clear_templates(&tmpls));

    // Finalize the API
    JANICE_ASSERT(janice_finalize());

    return 0;
}
