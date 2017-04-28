#include <janice_harness.h>

#include <json.hpp>

#include <fstream>
#include <future>
#include <thread>

using nlohmann::json;

void janice_cluster_files(const char * input_fname, const char * output_fname, float threshold)
{
    JaniceTemplates tmpls;
    int template_count;
    JaniceTemplateIds input_ids;
    janice_batch_read_templates(input_fname, &tmpls, &input_ids, &template_count);


    std::cout << "Finished reading templates, starting clustering" << std::endl;
    std::cout << "Acquried " << template_count << " templates" << std::endl;

    JaniceTemplateClusterItems clusters;
    uint32_t num_clusters;

    uint32_t encoded_hint;
    encoded_hint = *((uint32_t *) &threshold);


    std::cout << "Clustering templates" << std::endl;
    janice_cluster_templates((JaniceConstTemplates) tmpls, input_ids, template_count, encoded_hint, &clusters, &num_clusters);

    std::cout << "Finished clustering" << std::endl;


    std::cout << "Writing output csv file " << std::endl;

    std::ofstream fout(output_fname);
    // write header
    fout << "TEMPLATE_ID,CLUSTER_INDEX,CONFIDENCE" << std::endl;

    for (size_t i = 0; i < template_count; i++) {
        // tid, cluster_id,cluster_conf
        fout << clusters[i].tmpl_id << ','  <<  clusters[i].cluster_id << ',' << clusters[i].confidence << std::endl;
    }
    fout.close();

    
}

 


