#include "types.hpp"
#include "utils.hpp"

#include <dlib/clustering.h>

JaniceError janice_cluster_media(const JaniceMediaIterators* its, const JaniceContext* context, JaniceClusterIdsGroup* cluster_ids_group, JaniceClusterConfidencesGroup* cluster_confidences_group, JaniceDetectionsGroup* detections_group)
{
    JaniceTemplatesGroup tmpls_group;
    JaniceErrors errors;
    JaniceError err = janice_enroll_from_media_batch(its, context, &tmpls_group, detections_group, &errors);
    if (err != JANICE_SUCCESS) {
        return err;
    }

    janice_clear_errors(&errors);

    std::vector<JaniceTemplate> flat_tmpls_group;
    for (size_t i = 0; i < tmpls_group.length; ++i) {
        for (size_t j = 0; j < tmpls_group.group[i].length; ++j) {
            flat_tmpls_group.push_back(tmpls_group.group[i].tmpls[j]);
        }
    }

    JaniceTemplates cluster_tmpls;
    cluster_tmpls.length = flat_tmpls_group.size();
    cluster_tmpls.tmpls = flat_tmpls_group.data();

    JaniceClusterIds flat_ids;
    JaniceClusterConfidences flat_confidences;
    err = janice_cluster_templates(&cluster_tmpls, context, &flat_ids, &flat_confidences);

    janice_clear_templates_group(&tmpls_group);

    if (err != JANICE_SUCCESS) {
        return err;
    }

    cluster_ids_group->length = detections_group->length;
    cluster_ids_group->group = new JaniceClusterIds[cluster_ids_group->length];

    cluster_confidences_group->length = detections_group->length;
    cluster_confidences_group->group = new JaniceClusterConfidences[cluster_confidences_group->length];

    int index = 0;
    for (size_t i = 0; i < detections_group->length; ++i) {
        cluster_ids_group->group[i].length = detections_group->group[i].length;
        cluster_ids_group->group[i].ids = new uint64_t[cluster_ids_group->group[i].length];

        cluster_confidences_group->group[i].length = detections_group->group[i].length;
        cluster_confidences_group->group[i].confidences = new double[cluster_confidences_group->group[i].length];

        for (size_t j = 0; j < detections_group->group[i].length; ++j) {
            cluster_ids_group->group[i].ids[j] = flat_ids.ids[index];
            cluster_confidences_group->group[i].confidences[j] = flat_confidences.confidences[index];

            ++index;
        }
    }

    janice_clear_cluster_ids(&flat_ids);
    janice_clear_cluster_confidences(&flat_confidences);

    return JANICE_SUCCESS;
}

JaniceError janice_cluster_templates(const JaniceTemplates* tmpls, const JaniceContext* context, JaniceClusterIds* ids, JaniceClusterConfidences* confidences)
{
    assert(tmpls != nullptr);
    assert(context != nullptr);
    assert(ids != nullptr);
    assert(confidences != nullptr);

    std::vector<dlib::sample_pair> edges;
    for (size_t i = 0; i < tmpls->length; ++i) {
        for (size_t j = i; j < tmpls->length; ++j) {
            // Faces are connected in the graph if they are close enough.  Here we check if
            // the distance between two face descriptors is less than 0.6, which is the
            // decision threshold the network was trained to use.  Although you can
            // certainly use any other threshold you find useful.
            if (dlib::length(tmpls->tmpls[i]->fv - tmpls->tmpls[j]->fv) < 0.6)
                edges.push_back(dlib::sample_pair(i,j));
        }
    }

    std::vector<unsigned long> labels;
    const unsigned long num_clusters = dlib::chinese_whispers(edges, labels);
    (void) num_clusters;

    ids->length = tmpls->length;
    ids->ids = new uint64_t[ids->length];

    confidences->length = tmpls->length;
    confidences->confidences = new double[confidences->length];

    for (size_t i = 0; i < labels.size(); ++i) {
        ids->ids[i] = labels[i];
        confidences->confidences[i] = 1.0f;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_cluster_ids(JaniceClusterIds* ids)
{
    if (ids) {
        delete[] ids->ids;
        ids->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_cluster_ids_group(JaniceClusterIdsGroup* ids_group)
{
    if (ids_group) {
        for (size_t i = 0; i < ids_group->length; ++i) {
            janice_clear_cluster_ids(&ids_group->group[i]);
        }

        delete[] ids_group->group;
        ids_group->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_cluster_confidences(JaniceClusterConfidences* confidences)
{
    if (confidences) {
        delete[] confidences->confidences;
        confidences->length = 0;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_clear_cluster_confidences_group(JaniceClusterConfidencesGroup* confidences_group)
{
    if (confidences_group) {
        for (size_t i = 0; i < confidences_group->length; ++i) {
            janice_clear_cluster_confidences(&confidences_group->group[i]);
        }

        delete[] confidences_group->group;
        confidences_group->length = 0;
    }

    return JANICE_SUCCESS;
}
