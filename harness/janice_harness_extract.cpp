#include <janice_harness.h>

#include <json.hpp>

#include <fstream>

#include <future>
#include <thread>

using nlohmann::json;


std::vector<JaniceTemplate> batch_extract_templates(const std::vector<std::string> & fnames, const std::vector<JaniceRect> & detections, int start_idx, int count)
{
    assert(fnames.size() == detections.size());
    std::vector<JaniceTemplate> out;
    for(int i = start_idx; i < (start_idx + count) && i < fnames.size(); i++) {
        JaniceMediaIterator it;

        if (i >= fnames.size())
            std::cerr << "Outo f bounds index: "  << i << " vs. fnames size: " << fnames.size() << std::endl;

        JaniceError ferr = janice_file_get_iterator(fnames[i].c_str(), &it);
        if (ferr != JANICE_SUCCESS) {
            std::cerr << "failed to create file iterator for index " << i << " name: " << fnames[i] << std::endl;
            out.push_back(nullptr);
            continue;
        }

        JaniceDetection detection;
        if (i >= detections.size() )
            std::cerr << "Out of bounds index: " << i << " vs. detections size " << detections.size() << std::endl;


        JaniceError err = janice_create_detection(it, detections[i], 0, &detection);

        if (err != JANICE_SUCCESS) {
            std::cerr << "Failed to create detection for index: " << i << " fname: " << fnames[i] << std::endl;
            std::cerr << janice_error_to_string(err) << std::endl;
            out.push_back(nullptr);
            janice_free_media_iterator(&it);
            continue;
        }

        JaniceTemplate tmpl;
        JaniceError t_err = janice_create_template((JaniceConstDetections) &detection, 1, JaniceCluster, &tmpl);
        if (t_err != JANICE_SUCCESS) {
            std::cerr << "Create template failed! " << std::endl;
            tmpl = nullptr;
        }

        janice_free_media_iterator(&it);
        janice_free_detection(&detection);

        out.push_back(tmpl);
    }

    return out;
}


void janice_batch_extract_templates(const char * input_fname, const char * output_fname, int nThreads)
{
    // parse input json file
    std::ifstream fin;
    fin.open(input_fname, std::ios::in | std::ios::binary);

    std::string str((std::istreambuf_iterator<char>(fin)),
                        std::istreambuf_iterator<char>());
 
//    json metadata = json::parse(str).front();
    json metadata = json::parse(str);
   
    // we need filenames, template IDs (for evaluation), and detection
    // locations for each entry in the input file.
    std::vector<std::string> fnames;
    std::vector<long unsigned> template_ids;
    std::vector<JaniceRect> detections;

    std::map<long unsigned, std::string> tids_to_fnames;

    for (const json &md : metadata) {
        
        template_ids.push_back(md["template_id"]);
        auto sightings = md["sightings"][0];
        detections.push_back(JaniceRect());

        detections.back().x = sightings["x"];
        detections.back().y = sightings["y"];
        detections.back().width =  sightings["width"];
        detections.back().height = sightings["height"];

        fnames.push_back(sightings["url"]);
        tids_to_fnames[template_ids.back()] = fnames.back();

    }

    int thread_count = nThreads;
    int block_size = ceil(float(fnames.size()) / float(thread_count));
    

    std::vector<JaniceTemplate> templates;
    std::vector<std::future<std::vector<JaniceTemplate> > > futures;

    std::cout << "Firing " << thread_count << " jobs" << std::endl;

    int start_idx = 0;
    // Generated complete temlpates prior to clustering
    for (int i=0; i < thread_count; i++) {
        futures.push_back(std::async(std::launch::async, batch_extract_templates, fnames, detections, start_idx, block_size));
        start_idx = start_idx + block_size;
    }

    std::cout << "Waiting for results..." << std::endl;
    for (int i=0; i < futures.size(); i++) {
        std::vector<JaniceTemplate> sub_tmpl = futures[i].get();
        for (int j=0; j < sub_tmpl.size();j++) {
            templates.push_back(sub_tmpl[j]);
        }
    }
    std::cout << "Acquired total of: " << templates.size() << " templates, vs. nominal " << fnames.size() << std::endl;
    std::cout << "Finished extraction, writing output" << std::endl;


    FILE *fout = fopen(output_fname, "wb");
    if (!fout) {
        std::cerr << "Failed to open output file: " << output_fname << std::endl;
        return;
    }

    int null_count = 0;
    for (int i=0; i < templates.size(); i++) {
        if (!templates[i]) {
            null_count++;
            int temp = -1;
            fwrite(&temp, sizeof(int), 1, fout);
            fwrite(&(template_ids[i]), sizeof(long unsigned), 1, fout);

            continue;
        }

        JaniceBuffer data;
        size_t len;
        janice_serialize_template(templates[i], &data, &len);

        int alt_size = len;
        fwrite(&alt_size, sizeof(int), 1, fout);

        fwrite(&(template_ids[i]), sizeof(long unsigned), 1, fout);

        fwrite(data, len, 1, fout);
        
        delete [] data;
    }

    std::cerr << "Passed serialization, found: " << null_count << " null tempaltes" << std::endl;

    fclose(fout);

}



void janice_batch_read_templates(const char * input_file, JaniceTemplates *outputs, JaniceTemplateIds *tids, int * output_count)
{
    std::vector<JaniceTemplate>   tmpls;
    std::vector<JaniceTemplateId> template_ids;

    FILE *fin = fopen(input_file, "rb");
    if (!fin) {
        std::cerr << "Failed to input input file: " << fin << " for reading" << std::endl;
        return;
    }

    int tmpl_size = -1;
    while (fread(&tmpl_size, 1, sizeof(int), fin) == sizeof(int)) {
        long unsigned tid;

        fread(&tid, sizeof(long unsigned), 1, fin);

        template_ids.push_back(tid);

        if (tmpl_size == -1) {
            tmpls.push_back(nullptr);

            continue;
        }

        uint8_t *buffer = new uint8_t[tmpl_size];
        size_t read = fread(buffer, 1, tmpl_size, fin);
        if (read != tmpl_size) {
            std::cerr << "Failed to read template, got: "<< read << " expected: " << tmpl_size << std::endl;
            tmpls.push_back(nullptr);
            break;
        }


        JaniceTemplate tmpl;
        janice_deserialize_template(buffer, tmpl_size, &tmpl);
        tmpls.push_back(tmpl);
        delete [] buffer;
    }
    fclose(fin);

    std::cout << "Got output size: " << tmpls.size() << std::endl;
    *output_count =  tmpls.size();
   
    (*outputs) = new JaniceTemplate[tmpls.size()];
    *tids    = new JaniceTemplateId[tmpls.size()];

    for (int i=0; i < tmpls.size();i++) {
        (*outputs)[i] = tmpls[i];
        (*tids)[i] = template_ids[i];
    }
    std::cout << "Finished assigning outputs" << std::endl;
}
