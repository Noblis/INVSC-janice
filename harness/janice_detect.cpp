#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <fast-cpp-csv-parser/csv.h>

void print_usage()
{
    printf("Usage: janice_detect sdk_path temp_path data_path images_file min_face_size detection_policy output_file [-algorithm <algorithm>] [-threads <int>] [-gpu <int>]\n");
}

int main(int argc, char* argv[])
{
    const int min_args = 8;
    const int max_args = 14;

    if (argc < min_args || argc > max_args) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    const std::string sdk_path    = argv[1];
    const std::string temp_path   = argv[2];
    const std::string data_path   = argv[3];
    const std::string images_file = argv[4];
    const int min_object_size     = atoi(argv[5]);
    const std::string policy_str  = argv[6];
    const std::string output_file = argv[7];

    std::string algorithm;
    int num_threads;
    int gpu;
    if (!parse_optional_args(argc, argv, min_args, max_args, algorithm, num_threads, gpu))
        exit(EXIT_FAILURE);

    // Check input
    if (get_ext(images_file) != "csv") {
        printf("images_file must be \".csv\" format.\n");
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

    // Initialize the API
    // TODO: Right now we only allow a single GPU to be used
    JANICE_ASSERT(janice_initialize(sdk_path.c_str(), temp_path.c_str(), algorithm.c_str(), num_threads, &gpu, 1))

    // Unused defaults for context parameters
    JaniceEnrollmentType role = Janice1NProbe;
    double threshold = 0;
    uint32_t max_returns = 0;
    double hint = 0;

    JaniceContext context = nullptr;
    JANICE_ASSERT(janice_create_context(policy, min_object_size, role, threshold, max_returns, hint, &context))

    // Parse the images file
    io::CSVReader<1> images(images_file);
    images.read_header(io::ignore_extra_column, "FILENAME");

    std::vector<std::string> filenames;
    std::vector<JaniceMediaIterator> media;

    // Load filenames into a vector
    std::string filename;
    while (images.read_row(filename)) {
        JaniceMediaIterator it;
        JANICE_ASSERT(janice_io_opencv_create_media_iterator((std::string(data_path) + filename).c_str(), &it))

        filenames.push_back(filename);
        media.push_back(it);
    }

    // Convert the vector into a C-style struct
    JaniceMediaIterators media_list;
    media_list.media = media.data();
    media_list.length = media.size();

    // Run batch detection
    JaniceDetectionsGroup detections_group;
    JANICE_ASSERT(janice_detect_batch(media_list, context, &detections_group))

    // Assert we got the correct number of detections (1 list for each media)
    if (detections_group.length != media.size()) {
        printf("Incorrect return value. The number of detection lists should match the number of media files\n");
        exit(EXIT_FAILURE);
    }

    // Free the media objects
    for (JaniceMediaIterator& it : media)
        JANICE_ASSERT(it->free(&it))
    
    // Write the detection files to disk
    FILE* output = fopen(output_file.c_str(), "w+");
    fprintf(output, "file,frame,Face_X,Face_Y,Face_Width,Face_Height,Confidence\n");

    for (size_t i = 0; i < detections_group.length; ++i) {
        JaniceDetections detections = detections_group.group[i];
        for (size_t j = 0; j < detections.length; ++j) {
            JaniceTrack track;
            JANICE_ASSERT(janice_detection_get_track(detections.detections[j], &track))

            const std::string filename = filenames[i];
            for (size_t k = 0; k < track.length; ++k) {
                JaniceRect rect  = track.rects[k];
                float confidence = track.confidences[k];
                uint32_t frame   = track.frames[k];
                
                fprintf(output, "%s,%u,%u,%u,%u,%u,%f\n", filename.c_str(), frame, rect.x, rect.y, rect.width, rect.height, confidence);
            }

            // Free the track
            JANICE_ASSERT(janice_clear_track(&track))
        }
    }

    // Free the detections
    JANICE_ASSERT(janice_clear_detections_group(&detections_group))

    // Finalize the API
    JANICE_ASSERT(janice_finalize())

    return 0;
}
