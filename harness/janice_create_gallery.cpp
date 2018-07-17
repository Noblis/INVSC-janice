#include <janice.h>
#include <janice_harness.h>

#include <arg_parser/args.hpp>
#include <fast-cpp-csv-parser/csv.h>

#include <iostream>
#include <chrono>

int main(int argc, char* argv[])
{
    args::ArgumentParser parser("Create a gallery from a list of templates.");
    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    args::Positional<std::string> template_file(parser, "template_file", "A path to a template file. The file should list the templates to enroll. Both `janice_enroll_media` and `janice_enroll_detection` produce suitable files for this function.");
    args::Positional<std::string> template_path(parser, "template_path", "A prefix path to prepend to all template files before loading them.");
    args::Positional<std::string> gallery_file(parser, "gallery_file", "A path to a gallery file. A file will be created if it doesn't already exist. The file location must be writable.");
    args::Positional<std::string> output_file(parser, "output_file", "A path to an output file. A file will be created if it doesn't already exist. The file location must be writable.");

    args::ValueFlag<std::string> sdk_path(parser, "string", "The path to the SDK of the implementation", {'s', "sdk_path"}, "./");
    args::ValueFlag<std::string> temp_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'t', "temp_path"}, "./");
    args::ValueFlag<std::string> log_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'l', "log_path"}, "./");
    args::ValueFlag<std::string> algorithm(parser, "string", "Optional additional parameters for the implementation. The format and content of this string is implementation defined.", {'a', "algorithm"}, "");
    args::ValueFlag<int>         num_threads(parser, "int", "The number of threads the implementation should use while running detection.", {'j', "num_threads"}, 1);
    args::ValueFlag<int>         batch_size(parser, "int", "The size of a single batch. A larger batch size may run faster but will use more CPU resources.", {'b', "batch_size"}, 128);
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

    if (!template_file || !template_path || !gallery_file) {
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

    context.batch_policy = JaniceFlagAndFinish;

    // Parse the media file
    io::CSVReader<1> metadata(args::get(template_file));
    metadata.read_header(io::ignore_extra_column, "TEMPLATE_ID");

    std::vector<std::string> filenames;
    std::vector<uint64_t> template_ids;

    {
        int template_id;
        while (metadata.read_row(template_id)) {
            filenames.push_back(args::get(template_path) + "/" + std::to_string(template_id) + ".tmpl");
            template_ids.push_back(template_id);
        }
    }

    JaniceTemplates tmpls;
    tmpls.tmpls = new JaniceTemplate[args::get(batch_size)]; // Pre-allocate
    tmpls.length = 0; // Set to 0 to create an empty gallery

    JaniceTemplateIds ids;
    ids.ids = new uint64_t[args::get(batch_size)]; // Pre-allocate
    ids.length = 0; // Set to 0 to create an empty gallery

    JaniceGallery gallery;
    JANICE_ASSERT(janice_create_gallery(&tmpls, &ids, &gallery), ignored_errors);

    auto start = std::chrono::high_resolution_clock::now();
    JANICE_ASSERT(janice_gallery_reserve(gallery, filenames.size()), ignored_errors);
    double reserve_time = 10e-3 * std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();

    // Open the candidate list file
    FILE* output = fopen(args::get(output_file).c_str(), "w+");
    fprintf(output, "RESERVE_TIME,TEMPLATE_ID,BATCH_IDX,INSERT_TIME\n");

    int num_batches = filenames.size() / args::get(batch_size) + 1;

    int pos = 0;
    for (int batch_idx = 0; batch_idx < num_batches; ++batch_idx) {
        int current_batch_size = std::min(args::get(batch_size), (int) filenames.size() - pos);

        tmpls.length = current_batch_size;
        ids.length = current_batch_size;

        for (int tmpl_idx = 0; tmpl_idx < current_batch_size; ++tmpl_idx) {
            JANICE_ASSERT(janice_read_template(filenames[pos + tmpl_idx].c_str(), &tmpls.tmpls[tmpl_idx]), ignored_errors);
            ids.ids[tmpl_idx] = template_ids[pos + tmpl_idx];
        }

        JaniceErrors batch_errors;

        auto start = std::chrono::high_resolution_clock::now();
        JaniceError ret = janice_gallery_insert_batch(gallery, &tmpls, &ids, &context, &batch_errors);
        double elapsed = 10e-3 * std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();

        if (ret == JANICE_BATCH_FINISHED_WITH_ERRORS) {
            for (size_t err_idx = 0; err_idx < batch_errors.length; ++err_idx) {
                JaniceError e = batch_errors.errors[err_idx];
                if (e != JANICE_SUCCESS) {
                    std::cerr << "Janice batch function failed!\n" << std::endl
                              << "    Error: " << janice_error_to_string(e) << std::endl
                              << "    Batch index: " << err_idx << std::endl
                              << "    Location: " << __FILE__ << ":" << __LINE__ << std::endl;

                    if (ignored_errors.find(e) != ignored_errors.end()) {
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }

        for (int tmpl_idx = 0; tmpl_idx < current_batch_size; ++tmpl_idx) {
            fprintf(output, "%f,%llu,%d,%f\n", reserve_time, ids.ids[tmpl_idx], batch_idx, elapsed);
        }

        for (int tmpl_idx = 0; tmpl_idx < current_batch_size; ++tmpl_idx) {
            JANICE_ASSERT(janice_free_template(&tmpls.tmpls[tmpl_idx]), ignored_errors);
        }

        pos += current_batch_size;
    }

    delete[] tmpls.tmpls;
    delete[] ids.ids;

    JANICE_ASSERT(janice_write_gallery(gallery, args::get(gallery_file).c_str()), ignored_errors);
    JANICE_ASSERT(janice_free_gallery(&gallery), ignored_errors);

    JANICE_ASSERT(janice_finalize(), ignored_errors);

    return 0;
}
