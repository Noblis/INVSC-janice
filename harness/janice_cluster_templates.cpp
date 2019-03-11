#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <arg_parser/args.hpp>
#include <fast-cpp-csv-parser/csv.h>

#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
    args::ArgumentParser parser("Run clustering on a set of templates.");
    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    args::Positional<std::string> template_file(parser, "template_file", "A path to an IJB-C compliant csv file. The IJB-C file format is defined at https://noblis.github.io/janice/harness.html#fileformat");
    args::Positional<std::string> template_path(parser, "template_path", "A prefix path to append to all templates before loading them");
    args::Positional<std::string> output_file(parser, "output_file", "A path to an output file. A file will be created if it doesn't already exist. The file location must be writable.");

    args::ValueFlag<std::string> sdk_path(parser, "string", "The path to the SDK of the implementation", {'s', "sdk_path"}, "./");
    args::ValueFlag<std::string> temp_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'t', "temp_path"}, "./");
    args::ValueFlag<std::string> log_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'l', "log_path"}, "./");
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

    if (!template_file || !template_path || !output_file) {
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

    context.hint            = args::get(hint);

    // Parse the metadata file
    io::CSVReader<2> metadata(args::get(template_file));
    metadata.read_header(io::ignore_extra_column, "FILENAME","TEMPLATE_ID");

    std::vector<std::string> filenames;
    std::vector<size_t> template_ids;

    // Load filenames into a vector
    std::string filename;
    size_t template_id;
    while (metadata.read_row(filename, template_id)) {
        filenames.push_back(args::get(template_path) + filename);
        template_ids.push_back(template_id);
    }

    // need to build: JaniceTemplates tmpls 
    JaniceTemplates tmpls;
    tmpls.length = filenames.size();
    tmpls.tmpls = new JaniceTemplate[tmpls.length];

    for (size_t i=0; i < filenames.size(); i++) {
        JANICE_ASSERT(janice_read_template(filenames[i].c_str(), &tmpls.tmpls[i]), ignored_errors);
    }

    JaniceClusterIds cluster_ids;
    JaniceClusterConfidences cluster_confidences;

    JANICE_ASSERT(janice_cluster_templates(&tmpls, &context, &cluster_ids, &cluster_confidences), ignored_errors);

    if (cluster_ids.length != tmpls.length) {
        std::cerr << "Output cluster assignments did not match input templates length!" << std::endl;
        return -1;
    }

    if (cluster_confidences.length != tmpls.length) {
        std::cerr << "Output cluster confidences did not match input templates length!" << std::endl;
        return -1;
    }

    std::ofstream fout(args::get(output_file).c_str());
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
    JANICE_ASSERT(janice_clear_cluster_ids(&cluster_ids), ignored_errors);
    JANICE_ASSERT(janice_clear_cluster_confidences(&cluster_confidences), ignored_errors);

    // Free the templates, this was partially allocated by us, so 
    // can't just call janice_clear_templates
    for (size_t i =0; i < tmpls.length; i++) {
        JANICE_ASSERT(janice_free_template(&tmpls.tmpls[i]), ignored_errors);
    }
    delete [] tmpls.tmpls;

    // Finalize the API
    JANICE_ASSERT(janice_finalize(), ignored_errors);

    return 0;
}
