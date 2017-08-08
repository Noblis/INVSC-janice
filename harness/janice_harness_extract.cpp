#include <janice_harness.h>

#include <limits>

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

        JaniceError ferr = janice_io_opencv_create_media_iterator(fnames[i].c_str(), &it);
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
	    it->free(&it);
            continue;
        }

        JaniceTemplate tmpl;
        JaniceError t_err = janice_create_template((JaniceConstDetections) &detection, 1, JaniceCluster, &tmpl);
        if (t_err != JANICE_SUCCESS) {
            std::cerr << "Create template failed! " << std::endl;
            tmpl = nullptr;
        }

	it->free(&it);
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

void batch_dae_templates(std::vector<std::string> & fnames, std::vector<size_t> & sids, std::vector<JaniceTemplate> & templates, size_t start_idx, size_t count)
{
    std::cout << "Entered batch dae tempaltes, start: " << start_idx << " end: " << start_idx + count << std::endl;
    for(int i = start_idx; i < (start_idx + count) && i < fnames.size(); i++) {    
        std::cout<< "Reading file: " << fnames[i] << " index: " << i << std::endl;
        JaniceMediaIterator it;

        if (i >= fnames.size())
            std::cerr << "Outo f bounds index: "  << i << " vs. fnames size: " << fnames.size() << std::endl;

        JaniceError ferr = janice_io_opencv_create_media_iterator(fnames[i].c_str(), &it);
        if (ferr != JANICE_SUCCESS) {
            std::cerr << "failed to create file iterator for index " << i << " name: " << fnames[i] << std::endl;
            continue;
        }
        std::cout << "Created media iterator successfully" << std::endl;

        JaniceDetections detections;
        uint32_t detection_count;
        uint32_t min_object_size = 20;

        JaniceError err = janice_detect(it, min_object_size, &detections, &detection_count);
        if (err != JANICE_SUCCESS) {
            std::cerr << "Failed to detect for index: " << i << " fname: " << fnames[i] << std::endl;
            std::cerr << janice_error_to_string(err) << std::endl;
	    it->free(&it);
            continue;
        }

        JaniceDetection selected = nullptr;

        float max_conf = -std::numeric_limits<float>::max();
        int max_idx = -1;
        std::cout << "Got total of " << detection_count << " detections for image" << std::endl;
        // got 1 or more detections, find one with highest confidence (this is annoying)
        for (int det_idx = 0; det_idx < detection_count; det_idx++) {
            std::cout << "Checking detection: " << det_idx << " of " << detection_count << std::endl;
            JaniceDetectionIterator dit;
            JaniceError dit_rc = janice_create_detection_it(detections[det_idx], &dit);
            if (dit_rc != JANICE_SUCCESS) {
                std::cerr << "failed to create detection iterator from valid detection, bailing" << std::endl;
                return ;
            }
            std::cout << "Got detection it, attempting to read" << std::endl;
            uint32_t fnum = -1;
            float conf = -1;
            JaniceRect rect;
            JaniceError next_rc = janice_detection_it_next(dit, &rect, &fnum, &conf);

            if (next_rc != JANICE_SUCCESS && next_rc != JANICE_MEDIA_AT_END) {
                std::cerr << "Failed to get detection iterator" << std::endl;
                std::cerr << "rc is: " << next_rc << std::endl;
            }

            std::cout << "got conf value: " << conf << std::endl;
            if (conf > max_conf) {
                max_conf = conf;
                max_idx = det_idx;
            }

            JaniceError free_dit_rc = janice_free_detection_iterator(&dit);
            if (free_dit_rc != JANICE_SUCCESS) {
                std::cerr << "Failed to free detection iterator, definitely shoudln't have happened" << std::endl;
            }
        }


        if (max_idx == -1) {
            // no valid detections, just continue
            std::cerr << "No apparent valid detections, continuing" << std::endl;
            continue;
        }
            
        selected = detections[max_idx];

        std::cout << "Calling create template with detection at idx: " << max_idx << std::endl;
        JaniceTemplate tmpl;
        JaniceError t_err = janice_create_template((JaniceConstDetections) &selected, 1, Janice11Verification, &tmpl);
        std::cout << "Cleared create templte" << std::endl;
        if (t_err != JANICE_SUCCESS) {
            std::cerr << "Create template failed! " << std::endl;
            tmpl = nullptr;
        }

	it->free(&it);
        janice_free_detections(&detections, detection_count);

        templates[i] = tmpl;
    }
}

void janice_file_extract(const char * input_file, const char * output_file, int thread_count)
{
    std::ifstream fin(input_file);
    if (!fin)
        return;
    
    uint32_t linenum = 0;

    std::vector<std::string> fname_list;
    std::vector<size_t> sid_list;

    while (fin) {
        std::string aLine;
        std::getline(fin,aLine);

        linenum++;

        if (!fin)
            break;
        if (aLine.empty() )
            continue;
        
        std::istringstream lineStream(aLine);
        std::string fname;
        size_t sid;

        lineStream >> fname;
        if (!lineStream) {
            std::cerr << "Failed to read filename at line: " << linenum << std::endl;
            continue;
        }
        lineStream >> sid;
        if (!lineStream) {
            std::cerr << "Failed to read sid at line: " << linenum << std::endl;
        }

        fname_list.push_back(fname);
        sid_list.push_back(sid);
    }

    std::cout << "Finished reading file, got " << fname_list.size() << " total file names" << std::endl;

    int block_size = ceil(float(fname_list.size()) / float(thread_count));

    // output -- vector of extractred templates 
    std::vector<JaniceTemplate> templates(fname_list.size(), nullptr);
    std::vector<std::future<void> > futures;

    std::cout << "Firing " << thread_count << " jobs" << std::endl;

    int start_idx = 0;
    // Generated complete temlpates prior to clustering
    for (int i=0; i < thread_count; i++) {
        futures.push_back(std::async(std::launch::async, batch_dae_templates, std::ref(fname_list), std::ref(sid_list), std::ref(templates), start_idx, block_size));
        start_idx = start_idx + block_size;
    }

    std::cout << "waiting for jobs..." << std::endl;
    // wait for jobs to finish
    for (int i=0; i < futures.size(); i++) {
        futures[i].get();
    }
    
    FILE * fout = fopen(output_file, "wb");
    if (!fout) {
        std::cerr << "Failed to open output file: " << output_file << " for write" << std::endl;
        return;
    }

    for (int i=0; i < templates.size();i++) {
        if (templates[i] == nullptr) {
            int temp = -1;
            fwrite(&temp, sizeof(int), 1, fout);
            fwrite(&(sid_list[i]), sizeof(long unsigned), 1, fout);
            continue;
        }

        JaniceBuffer data;
        size_t len;
        janice_serialize_template(templates[i], &data, &len);

        int alt_size = len;
        fwrite(&alt_size, sizeof(int), 1, fout);

        fwrite(&(sid_list[i]), sizeof(long unsigned), 1, fout);
        fwrite(data, len, 1, fout);
        
        delete [] data;
    }
}



void janice_file_compare(const char * probe_file, const char * gallery_file, const char * output_file)
{
//void janice_batch_read_templates(const char * input_file, JaniceTemplates *outputs, JaniceTemplateIds *tids, int * output_count)
    std::cout << "reading probe templates file: " << probe_file << std::endl;
    JaniceTemplates probeTemplates;
    JaniceTemplateIds probeSIDs;
    int nProbe;
    janice_batch_read_templates(probe_file, &probeTemplates, &probeSIDs, &nProbe);

    std::cout << "Read total of: " << nProbe << " probe templates" << std::endl;

    std::cout << " readign gallery templates " << std::endl;
    JaniceTemplates galleryTemplates;
    JaniceTemplateIds gallerySIDs;
    int nGallery;
    janice_batch_read_templates(gallery_file, &galleryTemplates, &gallerySIDs, &nGallery);
    
    std::cout << "Read total of: " << nGallery << " gallery templates" << std::endl;

    std::ofstream fout(output_file);
    if (!fout) {
        std::cerr << "Failed to open output file " << output_file << std::endl;
        return;
    }
    fout << "Probe";
    for (int i=0; i < nGallery; i++) {
        fout << "," << gallerySIDs[i];
    }
    fout << std::endl;

    std::cout << "Opened outptu file, starting comparisons" << std::endl;
    float placeholder_value = -2;
    for (int i=0; i < nProbe; i++) {
        std::cout << "Comparisons for row: " << i << std::endl;
        fout << probeSIDs[i];
        for (int j=0; j < nGallery;j++) {
            std::string match = gallerySIDs[j] == probeSIDs[i] ? std::string("(G)") : std::string("(I)");

            // output placeholder value if either template is fte 
            if (!probeTemplates[i] || !galleryTemplates[j]) {
                fout << "," << placeholder_value << match;
                continue;
            }
            
            JaniceSimilarity score;
            JaniceError rc = janice_verify(probeTemplates[i], galleryTemplates[j], &score);
            if (rc != JANICE_SUCCESS) {
                std::cout << "Verify failed with rc: "<< rc << std::endl;
                fout << "," << placeholder_value << match;
                continue;
            }
            fout << "," << score << match;

        }
        fout << std::endl;
    }
}
