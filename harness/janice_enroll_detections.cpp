#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <arg_parser/args.hpp>
#include <fast-cpp-csv-parser/csv.h>

#include <unordered_map>
#include <iostream>

int main(int argc, char* argv[])
{
    args::ArgumentParser parser("Run feature extraction on a set of media with detections.");
    args::HelpFlag help(parser, "help", "Display this help menu.", {'h', "help"});

    args::Positional<std::string> media_file(parser, "media_file", "A path to an IJB-C compliant csv file. The IJB-C file format is defined at https://noblis.github.io/janice/harness.html#fileformat");
    args::Positional<std::string> dst_path(parser, "dst_path", "A path to an existing directory where the enrolled templates will be written. The directory must be writable.");
    args::Positional<std::string> output_file(parser, "output_file", "A path to an output file. A file will be created if it doesn't already exist. The file location must be writable.");

    args::ValueFlag<std::string> sdk_path(parser, "string", "The path to the SDK of the implementation", {'s', "sdk_path"}, "./");
    args::ValueFlag<std::string> temp_path(parser, "string", "An existing directory on disk where the caller has read / write access.", {'t', "temp_path"}, "./");
    args::ValueFlag<std::string> data_path(parser, "string", "A path to prepend to all image files before loading them", {'d', "data_path"}, "./");
    args::ValueFlag<std::string> role(parser, "string", "The enrollment role the algorithm should use. Options are [Reference11 | Verification11 | Probe1N | Gallery1N | Cluster]", {'r', "role"}, "Probe1N");
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

    if (!media_file || !dst_path || !output_file) {
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

    if      (args::get(role) == "Reference11")    context.role = Janice11Reference;
    else if (args::get(role) == "Verification11") context.role = Janice11Verification;
    else if (args::get(role) == "Probe1N")        context.role = Janice1NProbe;
    else if (args::get(role) == "Gallery1N")      context.role = Janice1NGallery;
    else if (args::get(role) == "Cluster")        context.role = JaniceCluster;
    else {
        printf("Invalid enrollment role. Valid enrollment role are [Reference11, Verification11, Probe1N, Gallery1N, Cluster]\n");
        exit(EXIT_FAILURE);
    }

    // Parse the metadata file
    io::CSVReader<8> metadata(args::get(media_file));
    metadata.read_header(io::ignore_extra_column, "FILENAME", "TEMPLATE_ID", "SUBJECT_ID", "SIGHTING_ID", "FACE_X", "FACE_Y", "FACE_WIDTH", "FACE_HEIGHT");

    std::unordered_map<int, std::vector<std::pair<std::string, JaniceRect>>> sighting_id_filename_lut;
    std::unordered_map<int, JaniceTemplateId> sighting_id_template_id_lut;
    std::unordered_map<JaniceTemplateId, int> template_id_subject_id_lut;

    {
        std::string filename;
        JaniceTemplateId template_id;
        int subject_id;
        int sighting_id;
        JaniceRect rect;

        while (metadata.read_row(filename, template_id, subject_id, sighting_id, rect.x, rect.y, rect.width, rect.height)) {
            sighting_id_filename_lut[sighting_id].push_back(std::make_pair(args::get(data_path) + filename, rect));
            sighting_id_template_id_lut[sighting_id] = template_id;
            template_id_subject_id_lut[template_id] = subject_id;
        }
    }

    std::unordered_map<JaniceTemplateId, std::vector<std::pair<JaniceMediaIterator, JaniceDetection>>> template_id_media_it_lut;

    for (auto entry : sighting_id_filename_lut) {
        JaniceMediaIterator it;
        JaniceDetection detection;

        if (entry.second.size() == 1) {
            JANICE_ASSERT(janice_io_opencv_create_media_iterator(entry.second[0].first.c_str(), &it));
            JANICE_ASSERT(janice_create_detection_from_rect(it, entry.second[0].second, 0, &detection));
            JANICE_ASSERT(it->reset(it));
        } else {
            const char** filenames = new const char*[entry.second.size()];

            JaniceTrack track;
            track.rects = new JaniceRect[entry.second.size()];
            track.confidences = new float[entry.second.size()];
            track.frames = new uint32_t[entry.second.size()];
            track.length = entry.second.size();

            for (size_t i = 0; i < entry.second.size(); ++i) {
                filenames[i] = entry.second[i].first.c_str();

                track.rects[i] = entry.second[i].second;
                track.confidences[i] = 1.0;
                track.frames[i] = i;
            }

            JANICE_ASSERT(janice_io_opencv_create_sparse_media_iterator(filenames, track.length, &it));
            JANICE_ASSERT(janice_create_detection_from_track(it, track, &detection));
            JANICE_ASSERT(it->reset(it));

            delete[] filenames;
            delete[] track.rects;
            delete[] track.confidences;
            delete[] track.frames;
        }

        template_id_media_it_lut[sighting_id_template_id_lut[entry.first]].push_back(std::make_pair(it, detection));
    }

    int num_batches = template_id_media_it_lut.size() / args::get(batch_size) + 1;

    FILE* output = fopen(args::get(output_file).c_str(), "w+");
    fprintf(output, "FILENAME,TEMPLATE_ID,SUBJECT_ID\n");

    auto it = template_id_media_it_lut.begin();
    int pos = 0;
    for (int i = 0; i < num_batches; ++i) {
        int current_batch_size = std::min(args::get(batch_size), (int) template_id_media_it_lut.size() - pos);

        JaniceMediaIteratorsGroup media_group;
        media_group.group = new JaniceMediaIterators[current_batch_size];
        media_group.length = current_batch_size;

        JaniceDetectionsGroup detections_group;
        detections_group.group = new JaniceDetections[current_batch_size];
        detections_group.length = current_batch_size;

        std::vector<int> batch_template_ids;
        for (int batch_idx = 0; batch_idx < current_batch_size; ++batch_idx) {
            const JaniceTemplateId& template_id = it->first;
            const std::vector<std::pair<JaniceMediaIterator, JaniceDetection>>& d_info = it->second;

            media_group.group[batch_idx].media = new JaniceMediaIterator[d_info.size()];
            media_group.group[batch_idx].length = d_info.size();

            detections_group.group[batch_idx].detections = new JaniceDetection[d_info.size()];
            detections_group.group[batch_idx].length     = d_info.size();

            for (int detection_idx = 0; detection_idx < d_info.size(); ++detection_idx) {
                media_group.group[batch_idx].media[detection_idx] = d_info[detection_idx].first;
                detections_group.group[batch_idx].detections[detection_idx] = d_info[detection_idx].second;
            }

            batch_template_ids.push_back(template_id);
            ++it;
        }

        JaniceTemplates tmpls;
        JANICE_ASSERT(janice_enroll_from_detections_batch(media_group, detections_group, &context, &tmpls));

        // Assert we got the correct number of templates (1 tmpl per detection subgroup)
        if (tmpls.length != current_batch_size) {
            printf("Incorrect return value. The number of templates should match the current batch size\n");
            exit(EXIT_FAILURE);
        }

        for (int tmpl_idx = 0; tmpl_idx < tmpls.length; ++tmpl_idx) {
            std::string tmpl_file = args::get(dst_path) + "/" + std::to_string(batch_template_ids[tmpl_idx]) + ".tmpl";
            JANICE_ASSERT(janice_write_template(tmpls.tmpls[tmpl_idx], tmpl_file.c_str()));

            fprintf(output, "%s,%d,%d\n", tmpl_file.c_str(), batch_template_ids[tmpl_idx], template_id_subject_id_lut[batch_template_ids[i]]);
        }

        // Cleanup detections group
        for (int batch_idx = 0; batch_idx < current_batch_size; ++batch_idx) {
            delete[] media_group.group[batch_idx].media;
            delete[] detections_group.group[batch_idx].detections;
        }

        delete[] media_group.group;
        delete[] detections_group.group;

        JANICE_ASSERT(janice_clear_templates(&tmpls));

        pos += current_batch_size;
    }

    for (auto& entry : template_id_media_it_lut) {
        for (std::pair<JaniceMediaIterator, JaniceDetection>& tmpl : entry.second) {
            JANICE_ASSERT(tmpl.first->free(&tmpl.first));
            JANICE_ASSERT(janice_free_detection(&tmpl.second));
        }
    }

    // Finalize the API
    JANICE_ASSERT(janice_finalize());

    return 0;
}
