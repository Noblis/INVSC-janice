#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <arg_parser/args.hpp>
#include <fast-cpp-csv-parser/csv.h>
#include <boost/filesystem.hpp>

#include <iostream>
#include <chrono>

int main(int argc, char* argv[])
{
    args::ArgumentParser parser("Run a series of 1-1 comparisons.");
    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    args::Positional<std::string> reference_file(parser, "reference_file", "A path to a template file. The file should list the templates to enroll. Both `janice_enroll_media` and `janice_enroll_detection` produce suitable files for this function.");
    args::Positional<std::string> reference_path(parser, "reference_path", "A prefix path to append to all reference templates before loading them");
    args::Positional<std::string> verification_file(parser, "verification_file", "A path to a template file. The file should list the templates to enroll. Both `janice_enroll_media` and `janice_enroll_detection` produce suitable files for this function.");
    args::Positional<std::string> verification_path(parser, "verification_path", "A prefix path to append to all verification templates before loading them");
    args::Positional<std::string> matches_file(parser, "matches_file", "A path to a list of matches to run verification with.");
    args::Positional<std::string> results_file(parser, "output_file", "A path to a candidate file. A file will be created if it doesn't already exist. The file location must be writable.");

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

    if (!reference_file
         || !reference_path
         || !verification_file
         || !verification_path
         || !results_file) {
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

    // Load the reference set
    io::CSVReader<1> reference_metadata(args::get(reference_file));
    reference_metadata.read_header(io::ignore_extra_column, "TEMPLATE_ID");

    std::unordered_map<uint64_t, JaniceTemplate> reference_tmpls;

    {
        uint64_t template_id;
        while (reference_metadata.read_row(template_id)) {
            JaniceTemplate tmpl = nullptr;
            boost::filesystem::path tmpl_file(args::get(reference_path));
            tmpl_file /= (std::to_string(template_id) + ".tmpl");
            JANICE_ASSERT(janice_read_template(tmpl_file.string().c_str(), &tmpl), ignored_errors);
            reference_tmpls[template_id] = tmpl;
        }
    }

    io::CSVReader<1> verification_metadata(args::get(verification_file));
    verification_metadata.read_header(io::ignore_extra_column, "TEMPLATE_ID");

    std::unordered_map<uint64_t, JaniceTemplate> verification_tmpls;

    {
        uint64_t template_id;
        while (verification_metadata.read_row(template_id)) {
            JaniceTemplate tmpl = nullptr;
            boost::filesystem::path tmpl_file(args::get(verification_path));
            tmpl_file /= (std::to_string(template_id) + ".tmpl");
            JANICE_ASSERT(janice_read_template(tmpl_file.string().c_str(), &tmpl), ignored_errors);
            verification_tmpls[template_id] = tmpl;
        }
    }

    std::vector<std::pair<uint64_t, uint64_t>> matches;

    io::CSVReader<2> matches_metadata(args::get(matches_file));

    {
        uint64_t left, right;
        while (matches_metadata.read_row(left, right)) {
            matches.push_back(std::make_pair(left, right));
        }
    }

    // Open the candidate list file
    FILE* results = fopen(args::get(results_file).c_str(), "w+");
    fprintf(results, "TEMPLATE_ID1,TEMPLATE_ID2,ERROR_CODE,SCORE,BATCH_IDX,VERIFY_TIME\n");

    int num_batches = matches.size() / args::get(batch_size) + 1;

    int pos = 0;
    for (int batch_idx = 0; batch_idx < num_batches; ++batch_idx) {
        int current_batch_size = std::min(args::get(batch_size), int(matches.size()) - pos);

        JaniceTemplates references;
        references.tmpls = new JaniceTemplate[current_batch_size];
        references.length = current_batch_size;

        JaniceTemplates verifications;
        verifications.tmpls = new JaniceTemplate[current_batch_size];
        verifications.length = current_batch_size;

        for (int batch_idx = 0; batch_idx < current_batch_size; ++batch_idx) {
            references.tmpls[batch_idx] = reference_tmpls[matches[pos + batch_idx].first];
            verifications.tmpls[batch_idx] = verification_tmpls[matches[pos + batch_idx].second];
        }

        JaniceSimilarities scores;
        JaniceErrors batch_errors;
        memset(&batch_errors, '\0', sizeof(batch_errors));

        auto start = std::chrono::high_resolution_clock::now();
        JaniceError ret = janice_verify_batch(&references, &verifications, &context, &scores, &batch_errors);
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

        for (int tmpl_idx = 0; tmpl_idx < current_batch_size; ++tmpl_idx) {
            fprintf(results, "%llu,%llu,0,%f,%d,%f\n", matches[pos + tmpl_idx].first, matches[pos + tmpl_idx].second, scores.similarities[tmpl_idx], batch_idx, elapsed);
        }

        JANICE_ASSERT(janice_clear_similarities(&scores), ignored_errors);

        delete[] references.tmpls;
        delete[] verifications.tmpls;

        pos += current_batch_size;
    }

    for (auto entry : reference_tmpls) {
        JANICE_ASSERT(janice_free_template(&entry.second), ignored_errors);
    }

    for (auto entry : verification_tmpls) {
        JANICE_ASSERT(janice_free_template(&entry.second), ignored_errors);
    }

    JANICE_ASSERT(janice_finalize(), ignored_errors);

    return 0;
}
