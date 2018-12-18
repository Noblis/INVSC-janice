#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <arg_parser/args.hpp>
#include <fast-cpp-csv-parser/csv.h>
#include <boost/filesystem.hpp>

#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
    args::ArgumentParser parser("Run clustering on a set of media.");
    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    args::Positional<std::string> media_file(parser, "media_file", "A path to an IJB-C compliant csv file. The IJB-C file format is defined at https://noblis.github.io/janice/harness.html#fileformat");
    args::Positional<std::string> media_path(parser, "media_path", "A prefix path to append to all media before loading them");
    args::Positional<std::string> output_file(parser, "output_file", "A path to an output file. A file will be created if it doesn't already exist. The file location must be writable.");

    args::ValueFlag<std::string> sdk_path(parser, "string", "The path to the SDK of the implementation", {'s', "sdk_path"}, "./");
    args::ValueFlag<std::string> temp_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'t', "temp_path"}, "./");
    args::ValueFlag<std::string> log_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'l', "log_path"}, "./");
    args::ValueFlag<uint32_t>    min_object_size(parser, "uint32", "The minimum sized object that should be detected", {'m', "min_object_size"}, 0);
    args::ValueFlag<std::string> policy(parser, "string", "The detection policy the algorithm should use. Options are '[All | Largest | Best]'", {'p', "policy"}, "All");
    args::ValueFlag<float>       hint(parser, "float", "The hint parameter that should be given to the clustering algorithm", {'h', "hint"}, 0.5);
    args::ValueFlag<std::string> algorithm(parser, "string", "Optional additional parameters for the implementation. The format and content of this string is implementation defined.", {'a', "algorithm"}, "");
    args::ValueFlag<int>         num_threads(parser, "int", "The number of threads the implementation should use while running detection.", {'j', "num_threads"}, 1);
    args::ValueFlag<std::vector<int>, ListReader<int>> gpus(parser, "int,int,int", "The GPU indices of the CUDA-compliant GPU cards the implementation should use while running detection", {'g', "gpus"}, std::vector<int>());
    args::ValueFlag<std::vector<std::string>, ListReader<std::string>> nonfatal_errors(parser, "JaniceError,JaniceError", "Comma-separated list of nonfatal JanusError codes", {'n', "nonfatal_errors"}, std::vector<std::string>());

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

    // Register nonfatal errors
    std::set<JaniceError> ignored_errors;
    janice_harness_register_nonfatal_errors(args::get(nonfatal_errors), ignored_errors);

    // Initialize the API
    JANICE_ASSERT(janice_initialize(args::get(sdk_path).c_str(), 
                                    args::get(temp_path).c_str(),
                                    args::get(log_path).c_str(),
                                    args::get(algorithm).c_str(), 
                                    args::get(num_threads), 
                                    args::get(gpus).data(), 
                                    args::get(gpus).size()), ignored_errors);

    JaniceContext context;
    JANICE_ASSERT(janice_init_default_context(&context), ignored_errors);

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
    context.hint            = args::get(hint);

    // Parse the metadata file
    io::CSVReader<1> metadata(input_file);
    metadata.read_header(io::ignore_extra_column, "FILENAME");

    std::vector<std::string> filenames;

    // Load filenames into a vector
    std::string filename;
    while (metadata.read_row(filename)) {
        filenames.push_back(filename);
    }

    // build media iterators as input to clustering

    JaniceMediaIterators media;
    media.length = filenames.size();
    media.media  = new JaniceMediaIterator[media.length];
    for (size_t i=0; i < media.length; i++) {
        boost::filesystem::path filename(data_path);
        filename /= filenames[i];
        JANICE_ASSERT(janice_io_opencv_create_media_iterator(filename.string().c_str(), &media.media[i]), ignored_errors);
    }

    JaniceClusterIdsGroup cluster_ids;
    JaniceClusterConfidencesGroup cluster_confidences;
    JaniceTracksGroup tracks;

    JANICE_ASSERT(janice_cluster_media(media, context, &cluster_ids, &cluster_confidences, &tracks), ignored_errors);

    if (cluster_ids.length != media.length) {
        std::cerr << "Output cluster assignments did not match input templates size!" << std::endl;
        return -1;
    }

    if (cluster_confidences.length != media.length) {
        std::cerr << "Output cluster confidences did not match input templates size!" << std::endl;
        return -1;
    }

    if (tracks.length != media.length) {
        std::cerr << "Output tracks did not match input media size" << std::endl;
        return -1;
    }

    std::ofstream fout(output_file.c_str());
    if (!fout) {
        std::cerr << "Failed to open output file: "<< output_file << std::endl;
        return -1;
    }

    fout << "FILENAME,FACE_X,FACE_Y,FACE_WIDTH,FACE_HEIGHT,SIGHTING_ID,CLUSTER_ID,CLUSTER_CONFIDENCE" << std::endl;
    size_t global_sighting_idx = 0;
    // output csv containing cluster assignments/confidences for each template
    for(size_t media_idx=0; media_idx < tracks.length; media_idx++) {
        if (tracks.group[media_idx].length != cluster_ids.group[media_idx].length) {
            std::cerr << "# tracks per media " << filenames[media_idx] << " did not match number of clustering assignments" << std::endl;
            return -1;
        }

        if (tracks.group[media_idx].length != cluster_confidences.group[media_idx].length) {
            std::cerr << "# tracks per media " << filenames[media_idx] << " did not match number of clustering confidences" << std::endl;
            return -1;
        }

        for (size_t track_idx=0; track_idx < tracks.group[media_idx].length; track_idx++) {
            size_t track_cluster_assignment = cluster_ids.group[media_idx].ids[track_idx];
            
            JaniceClusterConfidence current_confidence = cluster_confidences.group[media_idx].confidences[track_idx];

            for (size_t detection_idx = 0; detection_idx < tracks.group[media_idx].tracks[track_idx].length; detection_idx++) {
                JaniceRect current_rect = tracks.group[media_idx].tracks[track_idx].rects[detection_idx];
                
                fout << filenames[media_idx] << ',' << current_rect.x << ',' << current_rect.y << ',' << current_rect.width << ',' << current_rect.height << ',' << global_sighting_idx << ',' << track_cluster_assignment << "," << current_confidence << std::endl;

            }
            global_sighting_idx++;
        }
    }
    fout.close();

    // clear output variables from janice_cluster_templtes
    JANICE_ASSERT(janice_clear_cluster_ids_group(&cluster_ids), ignored_errors);
    JANICE_ASSERT(janice_clear_cluster_confidences_group(&cluster_confidences), ignored_errors);
    JANICE_ASSERT(janice_clear_tracks_group(&tracks), ignored_errors);

    for( size_t i=0; i < media.length; i++) {
        JANICE_ASSERT(media.media[i]->free(&media.media[i]), ignored_errors);
    }
    delete [] media.media;

    // Finalize the API
    JANICE_ASSERT(janice_finalize(), ignored_errors);

    return 0;
}
