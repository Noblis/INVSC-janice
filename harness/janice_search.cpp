#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <fast-cpp-csv-parser/csv.h>

#include <unordered_map>

void print_usage()
{
    printf("Usage: janice_search sdk_path temp_path probe_file gallery_file threshold num_returns candidate_list [-algorithm <algorithm>] [-threads <int>] [-gpu <int>]\n");
}

int main(int argc, char* argv[])
{
    const int min_args = 8;
    const int max_args = 14;

    if (argc < min_args || argc > max_args) {
        print_usage();
        exit(EXIT_FAILURE);
    }

    const std::string sdk_path       = argv[1];
    const std::string temp_path      = argv[2];
    const std::string probe_file     = argv[3];
    const std::string gallery_file   = argv[4];
    const float threshold            = atof(argv[5]);
    const uint32_t num_returns       = atol(argv[6]);
    const std::string cand_list_file = argv[7];

    std::string algorithm;
    int num_threads;
    int gpu;
    if (!parse_optional_args(argc, argv, min_args, max_args, algorithm, num_threads, gpu))
        exit(EXIT_FAILURE);

    // Check input
    if (get_ext(probe_file) != "csv") {
        printf("probe_file must be \".csv\" format.\n");
        exit(EXIT_FAILURE);
    }

    if (get_ext(gallery_file) != "csv") {
        printf("gallery_file must be \".csv\" format.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the API
    // TODO: Right now we only allow a single GPU to be used
    JANICE_ASSERT(janice_initialize(sdk_path.c_str(), temp_path.c_str(), algorithm.c_str(), num_threads, &gpu, 1));

    JaniceDetectionPolicy policy = JaniceDetectAll; // This is ignored
    uint32_t min_object_size = 0;
    JaniceEnrollmentType role = Janice1NProbe; // This is ignored
    double hint = 0.0;

    JaniceContext context = nullptr;
    JANICE_ASSERT(janice_create_context(policy, min_object_size, role, threshold, num_returns, hint, &context));

    JaniceGallery gallery = nullptr;
    { // Create an empty gallery
        JaniceTemplates tmpls;
        tmpls.tmpls = nullptr;
        tmpls.length = 0;

        JaniceTemplateIds ids;
        ids.ids = nullptr;
        ids.length = 0;
        JANICE_ASSERT(janice_create_gallery(tmpls, ids, &gallery));
    }

    // Load the gallery
    io::CSVReader<3> gallery_metadata(gallery_file);
    gallery_metadata.read_header(io::ignore_extra_column, "file", "templateId", "subjectId");

    std::unordered_map<int, int> subject_id_lut;

    // Load template into a gallery
    std::string filename;
    int template_id, subject_id;
    while (gallery_metadata.read_row(filename, template_id, subject_id)) {
        JaniceTemplate tmpl;
        JANICE_ASSERT(janice_read_template(filename.c_str(), &tmpl));

        JANICE_ASSERT(janice_gallery_insert(gallery, tmpl, template_id));

        subject_id_lut[template_id] = subject_id;

        JANICE_ASSERT(janice_free_template(&tmpl));
    }

    // Keep
    const uint32_t k = num_returns != 0 ? std::min(num_returns, (uint32_t) subject_id_lut.size()) : (uint32_t) subject_id_lut.size();

    // Open the candidate list file
    printf("cand_list_file: %s\n", cand_list_file.c_str());
    FILE* cand_list = fopen(cand_list_file.c_str(), "w+");
    fprintf(cand_list, "0");
    for (uint32_t i = 0; i < k; ++i)
        fprintf(cand_list, ",%u", i);
    fprintf(cand_list, "\n");

    // Parse the probe file
    io::CSVReader<3> probe_metadata(probe_file);
    probe_metadata.read_header(io::ignore_extra_column, "file", "templateId", "subjectId");

    while (probe_metadata.read_row(filename, template_id, subject_id)) {
        JaniceTemplate tmpl;
        JANICE_ASSERT(janice_read_template(filename.c_str(), &tmpl));

        JaniceSimilarities similarities;
        JaniceTemplateIds ids;
        JANICE_ASSERT(janice_search(tmpl, gallery, context, &similarities, &ids));

        fprintf(cand_list, "%d", template_id);
        for (size_t i = 0; i < similarities.length; ++i) {
            std::string label = (subject_id_lut[ids.ids[i]] == subject_id ? "G" : "I");
            fprintf(cand_list, ",%f(%s)", similarities.similarities[i], label.c_str());
        }
        fprintf(cand_list, "\n");

        JANICE_ASSERT(janice_clear_similarities(&similarities));
        JANICE_ASSERT(janice_clear_template_ids(&ids));
        JANICE_ASSERT(janice_free_template(&tmpl));
    }

    JANICE_ASSERT(janice_free_gallery(&gallery));
    JANICE_ASSERT(janice_free_context(&context));

    JANICE_ASSERT(janice_finalize());

    return 0;
}
