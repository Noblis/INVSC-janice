#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <iostream>
#include <fstream>

#include <fast-cpp-csv-parser/csv.h>

void print_usage()
{
    printf("Usage: janice_cluster_templates sdk_path temp_path data_path input_file output_file clustering_hint [-algorithm <algorithm>] [-threads <int>] [-gpu <int>]\n");
}

int main(int argc, char* argv[])
{
    const int min_args = 7;
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
    const std::string s_hint      = argv[6];

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
    uint32_t min_object_size = 0;
    double threshold = 0;
    uint32_t max_returns = 0;
    double hint = atof(s_hint.c_str());
    JaniceEnrollmentType role = JaniceCluster;

    JaniceContext context = nullptr;
    JANICE_ASSERT(janice_create_context(policy, min_object_size, role, threshold, max_returns, hint, &context))

    // Parse the metadata file
    io::CSVReader<2> metadata(input_file);
    metadata.read_header(io::ignore_extra_column, "FILENAME","TEMPLATE_ID");

    std::vector<std::string> filenames;
    std::vector<size_t> template_ids;

    // Load filenames into a vector
    std::string filename;
    size_t template_id;
    while (metadata.read_row(filename, template_id)) {
        filenames.push_back(filename);
        template_ids.push_back(template_id);
    }

    // need to build: JaniceTemplates tmpls 
    JaniceTemplates tmpls;
    tmpls.length = filenames.size();
    tmpls.tmpls = new JaniceTemplate[tmpls.length];

    for (size_t i=0; i < filenames.size(); i++) {
        JANICE_ASSERT(janice_read_template(filenames[i].c_str(), &tmpls.tmpls[i]));
    }

    JaniceClusterIds cluster_ids;
    JaniceClusterConfidences cluster_confidences;

    JANICE_ASSERT(janice_cluster_templates(tmpls, context, &cluster_ids, &cluster_confidences));

    if (cluster_ids.length != tmpls.length) {
        std::cerr << "Output cluster assignments did not match input templates length!" << std::endl;
        return -1;
    }
    if (cluster_confidences.length != tmpls.length) {
        std::cerr << "Output cluster confidences did not match input templates length!" << std::endl;
        return -1;
    }

    std::ofstream fout(output_file.c_str());
    if (!fout) {
        std::cerr << "Failed to open output file: "<< output_file << std::endl;
        return -1;
    }

    fout << "TEMPLATE_ID,CLUSTER_INDEX,CONFIDENCE" << std::endl;
    // output csv containing cluster assignments/confidences for each template
    for(size_t i=0; i < cluster_ids.length; i++) {
        fout << template_ids[i] <<"," << cluster_ids.ids[i] << "," << cluster_confidences.confidences[i] << std::endl;
    }
    fout.close();

    // clear output variables from janice_cluster_templtes
    JANICE_ASSERT(janice_clear_cluster_ids(&cluster_ids));
    JANICE_ASSERT(janice_clear_cluster_confidences(&cluster_confidences));

    // Free the templates, this was partially allocated by us, so 
    // can't just call janice_clear_templates
    for (size_t i =0; i < tmpls.length; i++) {
        JANICE_ASSERT(janice_free_template(&tmpls.tmpls[i]));
    }
    delete [] tmpls.tmpls;

    // Finalize the API
    JANICE_ASSERT(janice_finalize())

    return 0;
}
