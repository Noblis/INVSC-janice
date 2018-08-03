#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <arg_parser/args.hpp>
#include <fast-cpp-csv-parser/csv.h>

#include <iostream>
#include <chrono>

int main(int argc, char* argv[])
{
    args::ArgumentParser parser("Search a gallery with a set of probes.");
    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    args::Positional<std::string> probe_file(parser, "probe_file", "A path to a template file. The file should list the templates to enroll. Both `janice_enroll_media` and `janice_enroll_detection` produce suitable files for this function.");
    args::Positional<std::string> probe_path(parser, "probe_path", "A prefix path to append to all probe templates before loading them");
    args::Positional<std::string> gallery_file(parser, "gallery_file", "A path to a JanICE gallery saved on disk.");
    args::Positional<std::string> candidate_file(parser, "candidate_file", "A path to a candidate file. A file will be created if it doesn't already exist. The file location must be writable.");

    args::ValueFlag<std::string> sdk_path(parser, "string", "The path to the SDK of the implementation", {'s', "sdk_path"}, "./");
    args::ValueFlag<std::string> temp_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'t', "temp_path"}, "./");
    args::ValueFlag<std::string> log_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'l', "log_path"}, "./");
    args::ValueFlag<float>       threshold(parser, "float", "A score threshold for search. All returned matches will have a score over the threshold.", {'f', "threshold"}, 0.0);
    args::ValueFlag<int>         max_returns(parser, "int", "The maximum number of matches to return from a search.", {'m', "max_returns"}, 50);
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

    if (!probe_file || !gallery_file || !candidate_file) {
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

    context.threshold = args::get(threshold);
    context.max_returns = args::get(max_returns);
    context.batch_policy = JaniceFlagAndFinish;

    JaniceGallery gallery;
    JANICE_ASSERT(janice_read_gallery(args::get(gallery_file).c_str(), &gallery), ignored_errors);
    JANICE_ASSERT(janice_gallery_prepare(gallery), ignored_errors);

    io::CSVReader<1> metadata(args::get(probe_file));
    metadata.read_header(io::ignore_extra_column, "TEMPLATE_ID");

    std::vector<std::string> filenames;
    std::vector<uint64_t> template_ids;

    {
        uint64_t template_id;
        while (metadata.read_row(template_id)) {
            filenames.push_back(args::get(probe_path) + "/" + std::to_string(template_id) + ".tmpl");
            template_ids.push_back(template_id);
        }
    }

    // Open the candidate list file
    FILE* candidates = fopen(args::get(candidate_file).c_str(), "w+");
    fprintf(candidates, "SEARCH_TEMPLATE_ID,RANK,ERROR_CODE,GALLERY_TEMPLATE_ID,SCORE,BATCH_IDX,SEARCH_TIME\n");

    int num_batches = filenames.size() / args::get(batch_size) + 1;

    int pos = 0;
    for (int batch_idx = 0; batch_idx < num_batches; ++batch_idx) {
        int current_batch_size = std::min(args::get(batch_size), (int) filenames.size() - pos);

        JaniceTemplates probes;
        probes.tmpls = new JaniceTemplate[current_batch_size];
        probes.length = current_batch_size;

        for (int probe_idx = 0; probe_idx < current_batch_size; ++probe_idx) {
            JANICE_ASSERT(janice_read_template(filenames[pos + probe_idx].c_str(), &probes.tmpls[probe_idx]), ignored_errors);
        }

        JaniceSimilaritiesGroup search_scores;
        JaniceTemplateIdsGroup search_ids;
        JaniceErrors batch_errors;
        memset(&batch_errors, '\0', sizeof(batch_errors));

        auto start = std::chrono::high_resolution_clock::now();
        JaniceError ret = janice_search_batch(&probes, gallery, &context, &search_scores, &search_ids, &batch_errors);
        double elapsed = 10e-3 * std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();

        if (ret == JANICE_BATCH_FINISHED_WITH_ERRORS) {
            bool doExit = false;
            for (size_t err_idx = 0; err_idx < batch_errors.length; ++err_idx) {
                JaniceError e = batch_errors.errors[err_idx];
                if (e != JANICE_SUCCESS) {
                    std::cerr << "Janice batch function failed!" << std::endl
                              << "    Error: " << janice_error_to_string(e) << std::endl
                              << "    Batch index: " << err_idx << std::endl
                              << "    Location: " << __FILE__ << ":" << __LINE__ << std::endl;

                    if (ignored_errors.find(e) != ignored_errors.end()) {
                        doExit = true;
                    }
                }
            }
            if (doExit) {
                exit(EXIT_FAILURE);
            }
        }

        janice_clear_errors(&batch_errors);

        for (int probe_idx = 0; probe_idx < current_batch_size; ++probe_idx) {
            for (int search_idx = 0; search_idx < search_scores.group[probe_idx].length; ++search_idx) {
                fprintf(candidates, "%llu,%d,0,%llu,%f,%d,%f\n", template_ids[pos + probe_idx], search_idx, search_ids.group[probe_idx].ids[search_idx], search_scores.group[probe_idx].similarities[search_idx], batch_idx, elapsed);
            }
        }

        JANICE_ASSERT(janice_clear_similarities_group(&search_scores), ignored_errors);
        JANICE_ASSERT(janice_clear_template_ids_group(&search_ids), ignored_errors);

        for (int batch_idx = 0; batch_idx < current_batch_size; ++batch_idx) {
            JANICE_ASSERT(janice_free_template(&probes.tmpls[batch_idx]), ignored_errors);
        }

        delete[] probes.tmpls;

        pos += current_batch_size;
    }

    JANICE_ASSERT(janice_free_gallery(&gallery), ignored_errors);

    JANICE_ASSERT(janice_finalize(), ignored_errors);

    return 0;
}
