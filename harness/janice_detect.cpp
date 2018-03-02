#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <arg_parser/args.hpp>
#include <fast-cpp-csv-parser/csv.h>

#include <iostream>
#include <chrono>

int main(int argc, char* argv[])
{
    args::ArgumentParser parser("Run detection on a set of media.");
    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    args::Positional<std::string> media_file(parser, "media_file", "A path to an IJB-C compliant csv file. The IJB-C file format is defined at https://noblis.github.io/janice/harness.html#fileformat");
    args::Positional<std::string> media_path(parser, "media_path", "A prefix path to append to all media before loading them");
    args::Positional<std::string> output_file(parser, "output_file", "A path to an output file. A file will be created if it doesn't already exist. The file location must be writable.");

    args::ValueFlag<std::string> sdk_path(parser, "string", "The path to the SDK of the implementation", {'s', "sdk_path"}, "./");
    args::ValueFlag<std::string> temp_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'t', "temp_path"}, "./");
    args::ValueFlag<int>         min_object_size(parser, "int", "The minimum sized object that should be detected", {'m', "min_object_size"}, -1);
    args::ValueFlag<std::string> policy(parser, "string", "The detection policy the algorithm should use. Options are '[All | Largest | Best]'", {'p', "policy"}, "All");
    args::ValueFlag<std::string> algorithm(parser, "string", "Optional additional parameters for the implementation. The format and content of this string is implementation defined.", {'a', "algorithm"}, "");
    args::ValueFlag<int>         num_threads(parser, "int", "The number of threads the implementation should use while running detection.", {'j', "num_threads"}, 1);
    args::ValueFlag<int>         batch_size(parser, "int", "The size of a single batch. A larger batch size may run faster but will use more CPU resources.", {'b', "batch_size"}, 128);
    args::ValueFlag<std::vector<int>, GPUReader> gpus(parser, "int,int,int", "The GPU indices of the CUDA-compliant GPU cards the implementation should use while running detection", {'g', "gpus"}, std::vector<int>());

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    } catch (args::ValidationError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    if (!media_file || !media_path || !output_file) {
        std::cout << parser;
        return 1;
    }

    // Initialize the API
    JANICE_ASSERT(janice_initialize(args::get(sdk_path).c_str(), 
                                    args::get(temp_path).c_str(), 
                                    args::get(algorithm).c_str(), 
                                    args::get(num_threads), 
                                    args::get(gpus).data(), 
                                    args::get(gpus).size()));

    JaniceContext context;
    JANICE_ASSERT(janice_init_default_context(&context));

    if (args::get(policy) == "All") {
        context.policy = JaniceDetectAll;
    } else if (args::get(policy) == "Largest") {
        context.policy = JaniceDetectLargest;
    } else if (args::get(policy) == "Best") {
        context.policy = JaniceDetectBest;
    } else {
        printf("Invalid detection policy. Valid detection policies are [All | Largest | Best]\n");
        exit(EXIT_FAILURE);
    }

    context.min_object_size = args::get(min_object_size);

    // Parse the media file
    io::CSVReader<1> metadata(args::get(media_file));
    metadata.read_header(io::ignore_extra_column, "FILENAME");

    std::vector<std::string> filenames;
    std::vector<JaniceMediaIterator> media;

    { // Load filenames into a vector
        std::string filename;
        while (metadata.read_row(filename)) {
            JaniceMediaIterator it;
            JANICE_ASSERT(janice_io_opencv_create_media_iterator((args::get(media_path) + "/" + filename).c_str(), &it));

            filenames.push_back(filename);
            media.push_back(it);
        }
    }

    int num_batches = media.size() / args::get(batch_size) + 1;

    FILE* output = fopen(args::get(output_file).c_str(), "w+");
    fprintf(output, "TEMPLATE_ID,FILENAME,FRAME_NUM,FACE_X,FACE_Y,FACE_WIDTH,FACE_HEIGHT,CONFIDENCE,BATCH_IDX,DETECTION_TIME\n");

    int template_id = 0, pos = 0;
    for (int batch_idx = 0; batch_idx < num_batches; ++batch_idx) {
        int current_batch_size = std::min(args::get(batch_size), (int) media.size() - pos);

        JaniceMediaIterators media_list;
        media_list.media  = media.data() + pos;
        media_list.length = current_batch_size;

        // Run batch detection
        JaniceDetectionsGroup detections_group;

        auto start = std::chrono::high_resolution_clock::now();
        JANICE_ASSERT(janice_detect_batch(media_list, &context, &detections_group));
        double elapsed = 10e-3 * std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();

        // Assert we got the correct number of detections (1 list for each media)
        if (detections_group.length != current_batch_size) {
            printf("Incorrect return value. The number of detection lists should match the number of media files\n");
            exit(EXIT_FAILURE);
        }


        // Write the detection files to disk
        for (size_t group_idx = 0; group_idx < detections_group.length; ++group_idx) {
            JaniceDetections detections = detections_group.group[group_idx];
            for (size_t detection_idx = 0; detection_idx < detections.length; ++detection_idx) {
                JaniceTrack track;
                JANICE_ASSERT(janice_detection_get_track(detections.detections[detection_idx], &track));

                const std::string filename = filenames[pos + group_idx];
                for (size_t track_idx = 0; track_idx < track.length; ++track_idx) {
                    JaniceRect rect  = track.rects[track_idx];
                    float confidence = track.confidences[track_idx];
                    uint32_t frame   = track.frames[track_idx];
                
                    fprintf(output, "%d,%s,%u,%u,%u,%u,%u,%f,%d,%f\n", template_id++, filename.c_str(), frame, rect.x, rect.y, rect.width, rect.height, confidence, batch_idx, elapsed);
                }

                // Free the track
                JANICE_ASSERT(janice_clear_track(&track));
            }
        }

        // Free the detections
        JANICE_ASSERT(janice_clear_detections_group(&detections_group));

        pos += current_batch_size;
    }

    // Free the media iterators
    for (size_t i = 0; i < media.size(); ++i) {
        JANICE_ASSERT(media[i]->free(&media[i]));
    }

    // Finalize the API
    JANICE_ASSERT(janice_finalize());

    return 0;
}
