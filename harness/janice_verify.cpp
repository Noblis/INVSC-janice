#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <arg_parser/args.hpp>
#include <fast-cpp-csv-parser/csv.h>

#include <iostream>

int main(int argc, char* argv[])
{
    args::ArgumentParser parser("Search a gallery with a set of probes.");
    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    args::Positional<std::string> reference_file(parser, "reference_file", "A path to a template file. The file should list the templates to enroll. Both `janice_enroll_media` and `janice_enroll_detection` produce suitable files for this function.");
    args::Positional<std::string> verification_file(parser, "verification_file", "A path to a JanICE gallery saved on disk.");
    args::Positional<std::string> results_file(parser, "output_file", "A path to a candidate file. A file will be created if it doesn't already exist. The file location must be writable.");

    args::ValueFlag<std::string> sdk_path(parser, "string", "The path to the SDK of the implementation", {'s', "sdk_path"}, "./");
    args::ValueFlag<std::string> temp_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'t', "temp_path"}, "./");
    args::ValueFlag<std::string> reference_path(parser, "string", "A path to prepend to reference template files before loading them", {'d', "data_path"}, "./");
    args::ValueFlag<std::string> verification_path(parser, "string", "A path to prepend to all verification template files before loading them", {'d', "data_path"}, "./");
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

    if (!reference_file || !verification_file || !results_file) {
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

    // Load the reference set
    io::CSVReader<2> reference_metadata(args::get(reference_file));
    reference_metadata.read_header(io::ignore_extra_column, "FILENAME", "TEMPLATE_ID");

    std::vector<std::string> reference_filenames;
    std::vector<JaniceTemplateId> reference_template_ids;

    {
        std::string filename;
        int template_id;
        while (reference_metadata.read_row(filename, template_id)) {
            reference_filenames.push_back(args::get(reference_path) + "/" + filename);
            reference_template_ids.push_back(template_id);
        }
    }

    io::CSVReader<2> verification_metadata(args::get(verification_file));
    verification_metadata.read_header(io::ignore_extra_column, "FILENAME", "TEMPLATE_ID");

    std::vector<std::string> verification_filenames;
    std::vector<JaniceTemplateId> verification_template_ids;

    {
        std::string filename;
        int template_id;
        while (verification_metadata.read_row(filename, template_id)) {
            verification_filenames.push_back(args::get(verification_path) + "/" + filename);
            verification_template_ids.push_back(template_id);
        }
    }

    if (reference_filenames.size() != verification_filenames.size()) {
        printf("Verify requires the same number of reference and verificaton templates\n");
        exit(EXIT_FAILURE);
    }

    // Open the candidate list file
    FILE* results = fopen(args::get(results_file).c_str(), "w+");
    fprintf(results, "REFERENCE_ID,VERIFICATION_ID,SCORE\n");

    int num_batches = reference_filenames.size() / args::get(batch_size) + 1;

    int pos = 0;
    for (int i = 0; i < num_batches; ++i) {
        int current_batch_size = std::min(args::get(batch_size), (int) reference_filenames.size() - pos);

        JaniceTemplates references;
        references.tmpls = new JaniceTemplate[current_batch_size];
        references.length = current_batch_size;

        JaniceTemplates verifications;
        verifications.tmpls = new JaniceTemplate[current_batch_size];
        verifications.length = current_batch_size;

        for (int batch_idx = 0; batch_idx < current_batch_size; ++batch_idx) {
            JANICE_ASSERT(janice_read_template((reference_filenames[pos + batch_idx]).c_str(), &references.tmpls[batch_idx]));
            JANICE_ASSERT(janice_read_template((verification_filenames[pos + batch_idx]).c_str(), &verifications.tmpls[batch_idx]));
        }

        JaniceSimilarities scores;
        JANICE_ASSERT(janice_verify_batch(references, verifications, &scores));

        for (int batch_idx = 0; batch_idx < current_batch_size; ++batch_idx)
            fprintf(results, "%zu,%zu,%f\n", reference_template_ids[pos + batch_idx], verification_template_ids[pos + batch_idx], scores.similarities[batch_idx]);

        JANICE_ASSERT(janice_clear_similarities(&scores));

        for (int batch_idx = 0; batch_idx < current_batch_size; ++batch_idx) {
            JANICE_ASSERT(janice_free_template(&references.tmpls[batch_idx]));
            JANICE_ASSERT(janice_free_template(&verifications.tmpls[batch_idx]));
        }

        delete[] references.tmpls;
        delete[] verifications.tmpls;
    }

    JANICE_ASSERT(janice_finalize());

    return 0;
}
