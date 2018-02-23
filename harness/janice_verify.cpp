#include <janice.h>
#include <janice_io_opencv.h>
#include <janice_harness.h>

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <fast-cpp-csv-parser/csv.h>

void print_usage()
{
    printf("Usage: janice_verify sdk_path temp_path templates_list_file_a templates_list_file_b scores_file [-algorithm <algorithm>] [-threads <int>] [-gpu <int>]\n");
}

static JaniceError janice_load_template_map_from_file(const std::string &templates_list_file, std::map<uint32_t,JaniceTemplate> &templates, std::vector<uint32_t> &template_ids, std::vector<uint32_t> &subject_ids)
{
  clock_t start;

  std::ifstream templates_list_stream(templates_list_file.c_str());
  std::string line;

  while (getline(templates_list_stream, line)) {
    std::stringstream row(line);
    std::string template_id, subject_id, template_file;
    getline(row, template_id, ',');
    getline(row, subject_id, ',');
    getline(row, template_file, ',');

    std::stringstream ss;
    ss << template_id;
    uint32_t tid;
    ss >> tid;

    template_ids.push_back(tid);
    subject_ids.push_back(atoi(subject_id.c_str()));

    if (templates.find(tid) == templates.end()) {
      // Load the serialized template from disk
      std::cout << "[" << __func__ << "] deserializing " << template_id << ": " << template_file << std::endl;
      JaniceTemplate tmpl;
      JANICE_ASSERT(janice_read_template(template_file.c_str(), &tmpl));
      templates[tid] = tmpl;
    }
  }
  templates_list_stream.close();

  return JANICE_SUCCESS;
}

int main(int argc, char* argv[])
{
  const int min_args = 6;
  const int max_args = 12;

  if (argc < min_args || argc > max_args) {
    print_usage();
    exit(EXIT_FAILURE);
  }

  const std::string sdk_path       = argv[1];
  const std::string temp_path      = argv[2];
  const std::string templates_a_path = argv[3];
  const std::string templates_b_path = argv[4];
  const std::string scores         = argv[5];

  std::string algorithm;
  int num_threads;
  int gpu;
  if (!parse_optional_args(argc, argv, min_args, max_args, algorithm, num_threads, gpu)) {
    exit(EXIT_FAILURE);
  }

  std::map<uint32_t, JaniceTemplate> templates_a;
  std::map<uint32_t, JaniceTemplate> templates_b;
  std::vector<uint32_t> template_ids_a;
  std::vector<uint32_t> template_ids_b;
  std::vector<uint32_t> subject_ids_a;
  std::vector<uint32_t> subject_ids_b;

  JANICE_ASSERT(janice_initialize(sdk_path.c_str(), temp_path.c_str(), algorithm.c_str(), num_threads, &gpu, 1));

  JANICE_ASSERT(janice_load_template_map_from_file(templates_a_path, templates_a, template_ids_a, subject_ids_a));
  JANICE_ASSERT(janice_load_template_map_from_file(templates_b_path, templates_b, template_ids_b, subject_ids_b));
    
  assert(templates.a.size() == templates.b.size());

  std::ofstream scores_stream(scores.c_str(), std::ios::out | std::ios::ate);
  scores_stream.precision(15);
  scores_stream << "TEMPLATE_ID1,TEMPLATE_ID2,ERROR_CODE,SCORE,VERIFY_TIME" << std::endl;
  for (size_t i = 0; i < template_ids_a.size(); ++i) {
    uint32_t tid_a = template_ids_a[i];
    uint32_t tid_b = template_ids_b[i];
      
    JaniceSimilarity similarity;
    JaniceTemplate template_a = templates_a[tid_a];
    JaniceTemplate template_b = templates_b[tid_b];

    JaniceError res = janice_verify(template_a, template_b, &similarity);
    scores_stream << tid_a << ","
                  << tid_b << ","
                  << res << ","
                  << similarity << ","
                  << "0.0" << std::endl;
  }
  scores_stream.close();
  for (auto iter = templates_a.begin(); iter != templates_a.end(); ++iter) {
    JaniceTemplate tmpl = (*iter).second;
    janice_free_template(&tmpl);
  }
  for (auto iter = templates_b.begin(); iter != templates_b.end(); ++iter) {
    JaniceTemplate tmpl = (*iter).second;
    janice_free_template(&tmpl);
  }
    
  JANICE_ASSERT(janice_finalize());
  exit(1);
}
