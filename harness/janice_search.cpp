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

    args::Positional<std::string> probe_file(parser, "probe_file", "A path to a template file. The file should list the templates to enroll. Both `janice_enroll_media` and `janice_enroll_detection` produce suitable files for this function.");
    args::Positional<std::string> probe_path(parser, "probe_path", "A prefix path to append to all probe templates before loading them");
    args::Positional<std::string> gallery_file(parser, "gallery_file", "A path to a JanICE gallery saved on disk.");
    args::Positional<std::string> candidate_file(parser, "candidate_file", "A path to a candidate file. A file will be created if it doesn't already exist. The file location must be writable.");

    args::ValueFlag<std::string> sdk_path(parser, "string", "The path to the SDK of the implementation", {'s', "sdk_path"}, "./");
    args::ValueFlag<std::string> temp_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'t', "temp_path"}, "./");
    args::ValueFlag<float>       threshold(parser, "float", "A score threshold for search. All returned matches will have a score over the threshold.", {'f', "threshold"}, 0.0);
    args::ValueFlag<int>         max_returns(parser, "int", "The maximum number of matches to return from a search.", {'m', "max_returns"}, 50);
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

    if (!probe_file || !gallery_file || !candidate_file) {
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

    context.threshold = args::get(threshold);
    context.max_returns = args::get(max_returns);

    JaniceGallery gallery;
    JANICE_ASSERT(janice_read_gallery(args::get(gallery_file).c_str(), &gallery));

    // Load the gallery
    io::CSVReader<1> metadata(args::get(probe_file));
    metadata.read_header(io::ignore_extra_column, "TEMPLATE_ID");

    std::vector<std::string> filenames;
    std::vector<JaniceTemplateId> template_ids;

    {
        int template_id;
        while (metadata.read_row(template_id)) {
            filenames.push_back(args::get(probe_path) + "/" + std::to_string(template_id) + ".tmpl");
            template_ids.push_back(template_id);
        }
    }

    // Open the candidate list file
    FILE* candidates = fopen(args::get(candidate_file).c_str(), "w+");
    fprintf(candidates, "SEARCH_TEMPLATE_ID,RANK,ERROR_CODE,GALLERY_TEMPLATE_ID,SCORE,SEARCH_TIME\n");

    int num_batches = filenames.size() / args::get(batch_size) + 1;

    int pos = 0;
    for (int i = 0; i < num_batches; ++i) {
        int current_batch_size = std::min(args::get(batch_size), (int) filenames.size() - pos);

        JaniceTemplates probes;
        probes.tmpls = new JaniceTemplate[current_batch_size];
        probes.length = current_batch_size;

        for (int batch_idx = 0; batch_idx < current_batch_size; ++batch_idx)
            JANICE_ASSERT(janice_read_template(filenames[pos + batch_idx].c_str(), &probes.tmpls[batch_idx]));

        JaniceSimilaritiesGroup search_scores;
        JaniceTemplateIdsGroup search_ids;
        JANICE_ASSERT(janice_search_batch(probes, gallery, &context, &search_scores, &search_ids));

        for (int batch_idx = 0; batch_idx < current_batch_size; ++batch_idx)
            for (int search_idx = 0; search_idx < search_scores.group[batch_idx].length; ++search_idx)
                fprintf(candidates, "%zu,%d,0,%zu,%f,-1\n", template_ids[pos + batch_idx], search_idx, search_ids.group[batch_idx].ids[search_idx], search_scores.group[batch_idx].similarities[search_idx]);

        JANICE_ASSERT(janice_clear_similarities_group(&search_scores));
        JANICE_ASSERT(janice_clear_template_ids_group(&search_ids));

        for (int batch_idx = 0; batch_idx < current_batch_size; ++batch_idx)
            JANICE_ASSERT(janice_free_template(&probes.tmpls[batch_idx]));

        delete[] probes.tmpls;

        pos += current_batch_size;
    }

    JANICE_ASSERT(janice_free_gallery(&gallery));

    JANICE_ASSERT(janice_finalize());

    return 0;
}
