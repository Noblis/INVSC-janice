#include <janice.h>
#include <janice_harness.h>

#include <arg_parser/args.hpp>
#include <fast-cpp-csv-parser/csv.h>

#include <iostream>
#include <chrono>

int main(int argc, char* argv[])
{
    args::ArgumentParser parser("Run detection on a set of media.");
    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    args::Positional<std::string> template_file(parser, "template_file", "A path to a template file. The file should list the templates to enroll. Both `janice_enroll_media` and `janice_enroll_detection` produce suitable files for this function.");
    args::Positional<std::string> template_path(parser, "template_path", "A prefix path to prepend to all template files before loading them.");
    args::Positional<std::string> gallery_file(parser, "gallery_file", "A path to a gallery file. A file will be created if it doesn't already exist. The file location must be writable.");
    args::Positional<std::string> output_file(parser, "output_file", "A path to an output file. A file will be created if it doesn't already exist. The file location must be writable.");

    args::ValueFlag<std::string> sdk_path(parser, "string", "The path to the SDK of the implementation", {'s', "sdk_path"}, "./");
    args::ValueFlag<std::string> temp_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'t', "temp_path"}, "./");
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

    if (!template_file || !template_path || !gallery_file) {
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

    // Parse the media file
    io::CSVReader<1> metadata(args::get(template_file));
    metadata.read_header(io::ignore_extra_column, "TEMPLATE_ID");

    std::vector<std::string> filenames;
    std::vector<JaniceTemplateId> template_ids;

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
    ids.ids = new JaniceTemplateId[args::get(batch_size)]; // Pre-allocate
    ids.length = 0; // Set to 0 to create an empty gallery

    JaniceGallery gallery;
    JANICE_ASSERT(janice_create_gallery(tmpls, ids, &gallery));

    auto start = std::chrono::high_resolution_clock::now();
    JANICE_ASSERT(janice_gallery_reserve(gallery, filenames.size()));
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
            JANICE_ASSERT(janice_read_template(filenames[pos + tmpl_idx].c_str(), &tmpls.tmpls[tmpl_idx]));
            ids.ids[tmpl_idx] = template_ids[pos + tmpl_idx];
        }

        auto start = std::chrono::high_resolution_clock::now();
        JANICE_ASSERT(janice_gallery_insert_batch(gallery, tmpls, ids));
        double elapsed = 10e-3 * std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();

        for (int tmpl_idx = 0; tmpl_idx < current_batch_size; ++tmpl_idx) {
            fprintf(output, "%f,%zu,%d,%f\n", reserve_time, ids.ids[tmpl_idx], batch_idx, elapsed);
        }

        for (int tmpl_idx = 0; tmpl_idx < current_batch_size; ++tmpl_idx) {
            JANICE_ASSERT(janice_free_template(&tmpls.tmpls[tmpl_idx]));
        }

        pos += current_batch_size;
    }

    delete[] tmpls.tmpls;
    delete[] ids.ids;

    JANICE_ASSERT(janice_write_gallery(gallery, args::get(gallery_file).c_str()));
    JANICE_ASSERT(janice_free_gallery(&gallery));

    JANICE_ASSERT(janice_finalize());

    return 0;
}
