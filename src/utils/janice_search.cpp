#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>

#include "iarpa_janus.h"
#include "iarpa_janus_io.h"
using namespace std;

const char *get_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

void printUsage()
{
    printf("Usage: janus_evaluate_search sdk_path temp_path flat_gallery_file templates_dir probe_flat_templates_file num_returns candidate_lists_file [-algorithm <algorithm>]\n");
}

int main(int argc, char *argv[])
{
    int requiredArgs = 8;

    if ((argc < requiredArgs) || (argc > 9)) {
        printUsage();
        return 1;
    }

    char *algorithm = NULL;
    for (int i=0; i<argc-requiredArgs; i++)
        if (strcmp(argv[requiredArgs+i],"-algorithm") == 0)
            algorithm = argv[requiredArgs+(++i)];
        else {
            fprintf(stderr, "Unrecognized flag: %s\n", argv[requiredArgs+i]);
            return 1;
        }

    JANUS_ASSERT(janus_initialize(argv[1], argv[2], algorithm, 0))
    int num_requested_returns = atoi(argv[6]);
    janus_flat_gallery flat_gallery;
    size_t flat_gallery_size;

    JANUS_ASSERT(janus_read_flat_gallery(argv[3], &flat_gallery, &flat_gallery_size))
    
    ifstream probesfile(argv[5]);
    ofstream candlistfile(argv[7]);
    string line;
  
    while (getline(probesfile,line)) {
    	istringstream row(line);
 	string templ_id, template_file;
	getline(row, templ_id, ',');
	getline(row, template_file, ',');

	janus_flat_template flat_template;
	size_t flat_template_size;
	janus_template_id *template_ids = new janus_template_id[num_requested_returns];
	float* similarities = new float[num_requested_returns];	
	int num_actual_returns;

	JANUS_ASSERT(janus_read_flat_template((argv[4] + template_file).c_str(), &flat_template, &flat_template_size))
	JANUS_ASSERT(janus_search(flat_template, flat_template_size, flat_gallery, flat_gallery_size, num_requested_returns, template_ids, similarities, &num_actual_returns))	
	JANUS_ASSERT(janus_free_flat_template(flat_template));
	
	for(int i=0; i < num_actual_returns; i++) {
		candlistfile << templ_id << "," << i << "," << template_ids[i] << "," << similarities[i] << "\n";	
	}

	delete[] template_ids;
	delete[] similarities;
    }    

    probesfile.close();
    candlistfile.close();
    JANUS_ASSERT(janus_free_flat_gallery(flat_gallery))
    JANUS_ASSERT(janus_finalize())

    return EXIT_SUCCESS;
}
