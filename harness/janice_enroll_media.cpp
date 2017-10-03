#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <fast-cpp-csv-parser/csv.h>

#include <cstring>

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
    const std::string output_path = argv[8];

    std::string algorithm;
    int num_threads, gpu;
    if (!parse_optional_args(argc, argv, min_args, max_args, algorithm, num_threads, gpu))
        exit(EXIT_FAILURE);
    
    // Check input
    if (get_ext(input_file) != "csv") {
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
    JANICE_ASSERT(janice_initialize(sdk_path.c_str(), temp_path.c_str(), algorithm.c_str(), num_threads, &gpu, 1))

    // Unused defaults for context parameters
    double threshold = 0;
    uint32_t max_returns = 0;
    double hint = 0;

    JaniceContext context = nullptr;
    JANICE_ASSERT(janice_create_context(policy, min_object_size, role, threshold, max_returns, hint, &context))

    // Parse the metadata file
    io::CSVReader<1> metadata(input_file);
    metadata.read_header(io::ignore_extra_column, "file");

    std::vector<std::string> filenames;
    std::vector<JaniceMediaIterator> media;

    // Load filenames into a vector
    std::string filename;
    while (metadata.read_row(filename)) {
        JaniceMediaIterator it;
        JANICE_ASSERT(janice_io_opencv_create_media_iterator((std::string(data_path) + filename).c_str(), &it))

        filenames.push_back(filename);
        media.push_back(it);
    }

    // Convert the vector into a C-style struct
    JaniceMediaIterators media_list;
    media_list.media = media.data();
    media_list.length = media.size();

    // Run batch enrollment
    JaniceTemplatesGroup tmpls_group;
    JaniceTracksGroup    tracks_group;
    JANICE_ASSERT(janice_enroll_from_media_batch(media_list, context, &tmpls_group, &tracks_group))

    // Assert we got the correct number of templates (1 list for each media)
    if (tmpls_group.length != media.size()) {
        printf("Incorrect return value. The number of template lists should match the number of media files\n");
        exit(EXIT_FAILURE);
    }

    // Free the media objects
    for (JaniceMediaIterator& it : media)
        JANICE_ASSERT(it->free(&it))
    
    // Write the templates to disk
    FILE* output = fopen((output_path + "/templates.csv").c_str(), "w+");
    fprintf(output, "file,source,frame,Face_X,Face_Y,Face_Width,Face_Height,Confidence\n");

    for (size_t i = 0; i < tmpls_group.length; ++i) {
        const JaniceTemplates& tmpls = tmpls_group.group[i];
        const JaniceTracks& tracks   = tracks_group.group[i];
        for (size_t j = 0; j < tmpls.length; ++j) {
            JaniceTemplate tmpl = tmpls.tmpls[j];
    
            // Write the template to disk
            std::string tmpl_file = output_path + "/" + std::to_string(j) + "_" + filenames[i] + ".tmpl";
            JANICE_ASSERT(janice_write_template(tmpl, tmpl_file.c_str()))

            JaniceTrack track = tracks.tracks[j];

            for (size_t k = 0; k < track.length; ++k) {
                JaniceRect rect  = track.rects[k];
                float confidence = track.confidences[k];
                uint32_t frame   = track.frames[k];
                
                fprintf(output, "%s,%s,%u,%u,%u,%u,%u,%f\n", tmpl_file.c_str(), filenames[i].c_str(), frame, rect.x, rect.y, rect.width, rect.height, confidence);
            }
        }
    }

    // Clean up
    JANICE_ASSERT(janice_clear_templates_group(&tmpls_group))
    JANICE_ASSERT(janice_clear_tracks_group(&tracks_group))

    JANICE_ASSERT(janice_free_context(&context))

    // Finalize the API
    JANICE_ASSERT(janice_finalize())

    return 0;
}
