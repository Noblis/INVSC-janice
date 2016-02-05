#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <sstream>

#include "iarpa_janus.h"
#include "iarpa_janus_io.h"

using namespace std;

void printUsage()
{
    printf("Usage: janus_verify sdk_path temp_path template_dir template_pairs_file scores_file [-algorithm <algorithm>]\n");
}

int main(int argc, char *argv[])
{
    int requiredArgs = 6;

    if ((argc < requiredArgs) || (argc > 8)) {
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

    ifstream infile(argv[4]);
    ofstream outfile(argv[5]);
    string line;
    
    while (getline(infile,line)) {
	istringstream row(line);
        string enrolltempl, veriftempl;
        getline(row, enrolltempl, ',');
        getline(row, veriftempl, ',');
	janus_flat_template enroll_flat_template;
	janus_flat_template verif_flat_template;
	size_t enroll_flat_templ_size;
	size_t verif_flat_templ_size;

	JANUS_ASSERT(janus_read_flat_template((argv[3] + enrolltempl + ".flat_template").c_str(), &enroll_flat_template, &enroll_flat_templ_size))
	JANUS_ASSERT(janus_read_flat_template((argv[3] + veriftempl + ".flat_template").c_str(), &verif_flat_template, &verif_flat_templ_size))

	float similarity;
        JANUS_ASSERT(janus_verify(enroll_flat_template, enroll_flat_templ_size, verif_flat_template, verif_flat_templ_size, &similarity))	
	outfile << enrolltempl << "," << veriftempl << "," << similarity << "\n";
        JANUS_ASSERT(janus_free_flat_template(enroll_flat_template))
        JANUS_ASSERT(janus_free_flat_template(verif_flat_template))
    }        
    infile.close();
    outfile.close();
    JANUS_ASSERT(janus_finalize())

    janus_print_metrics(janus_get_metrics());
    return EXIT_SUCCESS;
}
