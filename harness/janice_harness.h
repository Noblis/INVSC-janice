#ifndef JANICE_HARNESS_H
#define JANICE_HARNESS_H


#include "janice.h"
#include "janice_io.h"

#if defined JANICE_HARNESS_LIBRARY
#  if defined _WIN32 || defined __CYGWIN__
#    define JANICE_HARNESS_EXPORT __declspec(dllexport)
#  else
#    define JANICE_HARNESS_EXPORT __attribute__((visibility("default")))
#  endif
#else
#  if defined _WIN32 || defined __CYGWIN__
#    define JANICE_HARNESS_EXPORT __declspec(dllimport)
#  else
#    define JANICE_HARNESS_EXPORT
#  endif
#endif


JANICE_HARNESS_EXPORT void janice_cluster_files(const char * input_fname, const char * output_fname, float aggressiveness);

JANICE_HARNESS_EXPORT void janice_batch_extract_templates(const char * input_fname, const char * output_fname, int num_threads);

JANICE_HARNESS_EXPORT void janice_batch_read_templates(const char * input_file, JaniceTemplates *output_file, JaniceTemplateIds * tids, int * output_count);


#endif
 
