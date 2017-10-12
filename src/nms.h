#ifndef NMS_H
#define NMS_H

#include <vector>

/*
 * Non-maximum suppression.
 * Select high-scoring detections and skip detections
 * that are significantly covered by a previously selected detection.
 */
#ifdef MV_PTC
std::vector<FPTYPE>* nms(const std::vector<FPTYPE> *boxes, FPTYPE overlap, int numparts, /*INPUT*/const std::vector<int>& inPartTypes, /*OUTPUT*/ std::vector<int>* &outPartTypes);

std::vector<FPTYPE>* not_nms_pick_best(const std::vector<FPTYPE> *boxes, int numparts, /*INPUT*/const std::vector<int>& inPartTypes, /*OUTPUT*/ std::vector<int>* &outPartTypes);

std::vector<FPTYPE>* not_nms_pick_best_N(const std::vector<FPTYPE> *boxes, int numparts, int bestNToReturn, /*INPUT*/const std::vector<int>& inPartTypes, /*OUTPUT*/ std::vector<int>* &outPartTypes);

#else //not MV_PTC
std::vector<FPTYPE>* nms(const std::vector<FPTYPE> *boxes, FPTYPE overlap, int numparts);

std::vector<FPTYPE>* not_nms_pick_best(const std::vector<FPTYPE> *boxes, int numparts);

std::vector<FPTYPE>* not_nms_pick_best_N(const std::vector<FPTYPE> *boxes, int numparts, int bestNToReturn);

#endif //MV_PTC


#endif // NMS_H
