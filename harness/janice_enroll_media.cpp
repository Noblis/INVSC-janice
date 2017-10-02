#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <fast-cpp-csv-parser/csv.h>

void print_usage()
{
    printf("Usage: janice_enroll_media sdk_path temp_path data_path input_file min_face_size detection_policy role output_path [-algorithm <algorithm>] [-threads <int>] [-gpu <int>]\n");
}

int main(int argc, char* argv[])
{
    const int min_args = 9;
    const int max_args = 15;

    if (argc < min_args || argc > max_args) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    const std::string sdk_path    = argv[1];
    const std::string temp_path   = argv[2];
    const std::string data_path   = argv[3];
    const std::string input_file  = argv[4];
    const int min_object_size     = atoi(argv[5]);
    const std::string policy_str  = argv[6];
    const std::string role_str    = argv[7];
    const std::string output_file = argv[8];

    std::string algorithm;
    int num_threads, gpu;
    if (!parse_optional_args(argc, argv, min_args, max_args, algorithm, num_threads, gpu))
        exit(EXIT_FAILURE);
    
    // Check input
    if (strcmp(get_ext(input_file), "csv") != 0) {
        printf("input_file must be \".csv\" format.\n");
        exit(EXIT_FAILURE);
    }

    // Convert detection policy
    JaniceDetectionPolicy policy;
    if      (policy_str == "All")     policy = JaniceDetectAll;
    else if (policy_str == "Largest") policy = JaniceDetectLargest;
    else if (policy_str == "Best")    policy = JaniceDetectBest;
    else {
        printf("Invalid detection policy. Valid detection policies are [All | Largest | Best]\n");
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
    double threshold = 0;
    uint32_t max_returns = 0;
    double hint = 0;

    JaniceContext context = nullptr;
    JANICE_ASSERT(janice_create_context(policy, min_object_size, role, threshold, max_returns, hint, &context))

    // Parse the metadata file
    io::CSVReader<1> metadata(input_file);
    metadata.read_header(io::ignore_extra_column, "FILENAME");

    std::vector<std::string> filenames;
    std::vector<JaniceMediaIterator> medias;

    // Load filenames into a vector
    std::string filename;
    while (metadata.read_row(filename)) {
        JaniceMediaIterator media;
        JANICE_ASSERT(janice_io_opencv_create_media_iterator((std::string(data_path) + filename).c_str(), &media))

        filenames.push_back(filename);
        medias.push_back(media);
    }

    // Convert the vector into a C-style struct
    JaniceMediaIterators media_list;
    media_list.medias = medias.data();
    media_list.length = medias.size();

    // Run batch enrollment
    JaniceTemplatesGroup tmpls_group;
    JaniceTracksGroup    tracks_group;
    JANICE_ASSERT(janice_enroll_from_media_batch(media_list, context, &tmpls_group, &tracks_group))

    // Assert we got the correct number of templates (1 list for each media)
    if (tmpls_group.length != medias.size()) {
        printf("Incorrect return value. The number of template lists should match the number of media files\n");
        exit(EXIT_FAILURE);
    }

    // Free the media objects
    for (JaniceMediaIterator& media : medias)
        JANICE_ASSERT(media->free(&media))
    
    // Write the templates to disk
    FILE* output = fopen((output_path "/templates.csv").c_str(), "w+");
    fprintf(output, "FILENAME,SOURCE,FRAME,RECT_X,RECT_Y,RECT_WIDTH,RECT_HEIGHT,CONFIDENCE\n");

    for (size_t i = 0; i < tmpls_group.length; ++i) {
        JaniceTemplates tmpls = tmpls_group[i];
        JaniceTracks tracks   = tracks_group[i];
        for (size_t j = 0; j < tmpls.length; ++j) {
            JaniceTemplate tmpl = tmpls[j];
    
            // Write the template to disk
            std::string tmpl_file = output_path + std::stoi(j) + "_" + filenames[i] + ".tmpl";
            JANICE_ASSERT(janice_write_template(tmpl, tmpl_file.c_str()))

            JaniceTrack track = tracks[j];

            for (size_t k = 0; k < track.length; ++k) {
                JaniceRect rect  = track.rects[k];
                float confidence = track.confidences[k];
                uint32_t frame   = track.frames[k];
                
                fprintf(output, "%s,%s,%u,%u,%u,%u,%u,%f\n", tmpl_file.c_str(), filenames[i].c_str(), frame, rect.x, rect.y, rect.width, rect.height, confidence);
            }

            // Free the track
            JANICE_ASSERT(janice_clear_track(&track))
        }
    }

    // Free the detections
    JANICE_ASSERT(janice_clear_detections_group(&detections))

    // Finalize the API
    JANICE_ASSERT(janice_finalize())

    return 0;
}
