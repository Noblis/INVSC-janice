#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <iostream>
#include <fstream>

#include <fast-cpp-csv-parser/csv.h>

void print_usage()
{
    printf("Usage: janice_cluster_media sdk_path temp_path data_path input_file output_file [-algorithm <algorithm>] [-threads <int>] [-gpu <int>]\n");
}

int main(int argc, char* argv[])
{
    const int min_args = 6;
    const int max_args = 15;

    if (argc < min_args || argc > max_args) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    const std::string sdk_path    = argv[1];
    const std::string temp_path   = argv[2];
    const std::string data_path   = argv[3];
    const std::string input_file  = argv[4];
    const std::string output_file = argv[5];

    std::string algorithm;
    int num_threads, gpu;
    if (!parse_optional_args(argc, argv, min_args, max_args, algorithm, num_threads, gpu))
        exit(EXIT_FAILURE);
    
    // Check input
    if (get_ext(input_file) != std::string("csv")) {
        printf("input_file must be \".csv\" format.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the API
    // TODO: Right now we only allow a single GPU to be used
    JANICE_ASSERT(janice_initialize(sdk_path.c_str(), temp_path.c_str(), algorithm.c_str(), num_threads, &gpu, 1))

    // Unused defaults for context parameters
    JaniceDetectionPolicy policy;
    uint32_t min_object_size = 20;
    double threshold = 0;
    uint32_t max_returns = 0;
    double hint = 1.0f;
    JaniceEnrollmentType role = JaniceCluster;

    JaniceContext context = nullptr;
    JANICE_ASSERT(janice_create_context(policy, min_object_size, role, threshold, max_returns, hint, &context))

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
    for( size_t i=0; i < media.length; i++) {
        JANICE_ASSERT(janice_io_opencv_create_media_iterator((std::string(data_path) + "/" + filenames[i]).c_str(), &media.media[i]));
    }

    JaniceClusterIdsGroup cluster_ids;
    JaniceClusterConfidencesGroup cluster_confidences;
    JaniceTracksGroup tracks;

    JANICE_ASSERT(janice_cluster_media(media, context, &cluster_ids, &cluster_confidences, &tracks));

    if (cluster_ids.length != media.length) {
        std::cerr << "Output cluster assignments did not match input templates size!" << std::endl;
        return -1;
    }

    if (cluster_confidences.length != media.length) {
        std::cerr << "Output cluster confidences did not match input templates size!" << std::endl;
        return -1;
    }

    if(tracks.length != media.length) {
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
    JANICE_ASSERT(janice_clear_cluster_ids_group(&cluster_ids));
    JANICE_ASSERT(janice_clear_cluster_confidences_group(&cluster_confidences));
    JANICE_ASSERT(janice_clear_tracks_group(&tracks));

    for( size_t i=0; i < media.length; i++) {
        media.media[i]->free(&media.media[i]);
    }
    delete [] media.media;

    // Finalize the API
    JANICE_ASSERT(janice_finalize())

    return 0;
}
