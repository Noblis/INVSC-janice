#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <fast-cpp-csv-parser/csv.h>

#include <unordered_map>

void print_usage()
{
  printf("Usage: janice_create_gallery sdk_path temp_path templates_list_file gallery_file [-algorithm <algorithm>] [-threads <int>] [-gpu <int>]\n");
}

int main(int argc, char* argv[])
{
  const int min_args = 5;
  const int max_args = 11;

  if (argc < min_args || argc > max_args) {
    print_usage();
    exit(EXIT_FAILURE);
  }

  const std::string sdk_path       = argv[1];
  const std::string temp_path      = argv[2];
  const std::string templates_list_file = argv[3];
  const std::string gallery_file   = argv[4];

  std::string algorithm;
  int num_threads;
  int gpu;
  if (!parse_optional_args(argc, argv, min_args, max_args, algorithm, num_threads, gpu))
    exit(EXIT_FAILURE);

  // Check input
  if (get_ext(templates_list_file) != "csv") {
    printf("templates_list_file must be \".csv\" format.\n");
    exit(EXIT_FAILURE);
  }

  // Initialize the API
  // TODO: Right now we only allow a single GPU to be used
  JANICE_ASSERT(janice_initialize(sdk_path.c_str(), temp_path.c_str(), algorithm.c_str(), num_threads, &gpu, 1));

  JaniceError res;

  JaniceGallery gallery = nullptr;

  // Create an empty gallery
  JaniceTemplates tmpls;
  tmpls.tmpls = nullptr;
  tmpls.length = 0;
  
  JaniceTemplateIds ids;
  ids.ids = nullptr;
  ids.length = 0;
  JANICE_ASSERT(janice_create_gallery(tmpls, ids, &gallery));

  // Load the gallery
  io::CSVReader<3> gallery_metadata(templates_list_file);
  gallery_metadata.read_header(io::ignore_extra_column, "FILENAME", "TEMPLATE_ID", "SUBJECT_ID");
  
  std::unordered_map<int, int> subject_id_lut;
  
  // Load template into a gallery
  std::string filename;
  int template_id, subject_id;
  while (gallery_metadata.read_row(filename, template_id, subject_id)) {
    JaniceTemplate tmpl;
    JANICE_ASSERT(janice_read_template(filename.c_str(), &tmpl));
    JANICE_ASSERT(janice_gallery_insert(gallery, tmpl, template_id));
    JANICE_ASSERT(janice_free_template(&tmpl));
    subject_id_lut[template_id] = subject_id;
  }

  JANICE_ASSERT(janice_gallery_prepare(gallery));
  
  JANICE_ASSERT(janice_write_gallery(gallery, gallery_file.c_str()));
  
  JANICE_ASSERT(janice_free_gallery(&gallery));
  
  JANICE_ASSERT(janice_finalize());
  
  return 0;
}
