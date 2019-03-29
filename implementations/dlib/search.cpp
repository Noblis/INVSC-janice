#include "types.hpp"
#include "utils.hpp"

JaniceError janice_search(const JaniceTemplate probe, const JaniceGallery gallery, const JaniceContext* context, JaniceSimilarities* similarities, JaniceTemplateIds* ids)
{
    assert(probe != nullptr);
    assert(gallery != nullptr);
    assert(context != nullptr);
    assert(similarities != nullptr);
    assert(ids != nullptr);

    try {
        std::vector<std::pair<float, uint64_t>> score_ids;
        for (size_t i = 0; i < gallery->tmpls.size(); ++i) {
            float score = 1 - dlib::length(probe->fv - gallery->tmpls[i].fv);
            uint64_t id = gallery->reverse_lut[i];

            score_ids.push_back(std::make_pair(score, id));
        }

        auto compare_scores = [](const std::pair<float, uint64_t>& left, const std::pair<float, uint64_t>& right) {
            // In the case of a tie we want to ensure a consistent order
            if (left.first == right.first) {
                return left.second > right.second;
            }

            return left.first > right.first;
        };

        size_t max_returns = context->max_returns;
        if (max_returns == 0) {
            max_returns = score_ids.size();
        }

        if (max_returns < score_ids.size()) {
            std::partial_sort(score_ids.begin(), score_ids.begin() + max_returns, score_ids.end(), compare_scores);
        } else {
            std::sort(score_ids.begin(), score_ids.end(), compare_scores);
        }

        if (context->threshold > -std::numeric_limits<float>::max()) {
            auto it = std::lower_bound(score_ids.begin(), score_ids.end(), context->threshold, [](const std::pair<float, uint64_t>& p, float val) {
               return val < p.first;
            });

            size_t threshold_limit = std::distance(score_ids.begin(), it);
            max_returns = std::min(max_returns, threshold_limit);
        }

        similarities->length = max_returns;
        similarities->similarities = new double[similarities->length];

        ids->length = max_returns;
        ids->ids = new uint64_t[ids->length];

        for (size_t i = 0; i < max_returns; ++i) {
            similarities->similarities[i] = score_ids[i].first;
            ids->ids[i]                   = score_ids[i].second;
        }
    } catch (const std::exception& e) {
        std::cerr << "Search failed. Error: [" << e.what() << "]" << std::endl;

        return JANICE_UNKNOWN_ERROR;
    }

    return JANICE_SUCCESS;
}

JaniceError janice_search_batch(const JaniceTemplates* probes, const JaniceGallery gallery, const JaniceContext* context, JaniceSimilaritiesGroup* similarities_group, JaniceTemplateIdsGroup* ids_group, JaniceErrors* errors)
{
    assert(probes != nullptr);
    assert(gallery != nullptr);
    assert(context != nullptr);
    assert(similarities_group != nullptr);
    assert(ids_group != nullptr);
    assert(errors != nullptr);

    JaniceError return_code = JANICE_SUCCESS;
    try {
        similarities_group->length = probes->length;
        similarities_group->group = new JaniceSimilarities[similarities_group->length];

        ids_group->length = probes->length;
        ids_group->group = new JaniceTemplateIds[ids_group->length];

        errors->length = probes->length;
        errors->errors = new JaniceError[errors->length];

        for (size_t i = 0; i < probes->length; ++i) {
            errors->errors[i] = janice_search(probes->tmpls[i], gallery, context, &similarities_group->group[i], &ids_group->group[i]);
            if (errors->errors[i] != JANICE_SUCCESS) {
                std::cerr << "Search probe: [" << i << "] failed. Error: [" << janice_error_to_string(errors->errors[i]) << "]" << std::endl;

                if (context->batch_policy == JaniceAbortEarly) {
                    errors->length = i + 1;
                    return JANICE_BATCH_ABORTED_EARLY;
                } else {
                    return_code = JANICE_BATCH_FINISHED_WITH_ERRORS;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Batch search failed. Error: [" << e.what() << "]" << std::endl;
        return JANICE_UNKNOWN_ERROR;
    }

    return JANICE_SUCCESS;
}
