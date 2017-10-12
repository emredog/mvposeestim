#include "nms.h"

#include "myarray.hpp"
#include "logging.h"

#include <algorithm>
#include <float.h> // for DBL_MIN and DBL_MAX


#define CANDIDATE_MAXNBR  1000  // max number of boxes to keep

#define X1(d, p)  detections[d].line[4*(p) + 0]
#define Y1(d, p)  detections[d].line[4*(p) + 1]
#define X2(d, p)  detections[d].line[4*(p) + 2]
#define Y2(d, p)  detections[d].line[4*(p) + 3]

#ifdef MV_PTC

struct Detection
{
	const FPTYPE *line; // boxes corners + component id + score
	FPTYPE score;
	bool enabled;
    const int *partTypes;
};

/*
 * Comparison function for detection sorting.
 */
bool compare_detections(const Detection& d1, const Detection& d2)
{
	return d1.score > d2.score;
}

/*
 * Sort detections in decreasing score order (highest score first).
 */
struct Detection* sort_boxes(const std::vector<FPTYPE> *boxes, const std::vector<int> inPartTypes, int detectionsNbr, int numparts, int line_length)
{
	struct Detection *detections = new Detection[detectionsNbr];
	const FPTYPE *boxes_data = &((*boxes)[0]);
    const int *partType_data = &(inPartTypes[0]);
	for(int d = 0; d < detectionsNbr; d++)
	{
		detections[d].line = boxes_data + d*line_length;
		detections[d].score = detections[d].line[line_length-1];
		detections[d].enabled = true;
        detections[d].partTypes = partType_data + d*(numparts);
	}
	std::sort(&detections[0], &detections[detectionsNbr], compare_detections);

	return detections;
}

std::vector<FPTYPE>* not_nms_pick_best(const std::vector<FPTYPE> *boxes, int numparts, /*INPUT*/const std::vector<int>& inPartTypes, /*OUTPUT*/ std::vector<int>* &outPartTypes)
{
    std::vector<FPTYPE> *boxes_nms = new std::vector<FPTYPE>;
    if (outPartTypes) // clear & delete if not provided null
    {
        outPartTypes->clear();
        delete outPartTypes;
    }
    outPartTypes = new std::vector<int>();
    
    if( boxes->empty() )
        return boxes_nms; //if there are no detections, we're supposed to return an empty vector for outPartTypes as well (it's expected outside)
    

    

    
    int line_length = 4*numparts+2; // nbr of elements in a line
    int detectionsNbr = boxes->size() / line_length;
    
    
    // throw away boxes if the number of candidates are too many
    // (sort by score and later consider only the Nth first)
    struct Detection *detections = sort_boxes(boxes, inPartTypes, detectionsNbr, numparts, line_length);
    
    //just take the best one
    for(int i = 0; i < line_length; i++)
    {
        boxes_nms->push_back(detections[0].line[i]);
    }
    
    for (int p = 0; p<numparts; p++)
        outPartTypes->push_back(detections[0].partTypes[p]);
    
    
    
    // release memory
    delete[] detections;
    
    return boxes_nms;
}

std::vector<FPTYPE> *not_nms_pick_best_N(const std::vector<double> *boxes, int numparts, int bestNToReturn, const std::vector<int> &inPartTypes, std::vector<int> *&outPartTypes)
{
    std::vector<FPTYPE> *boxes_nms = new std::vector<FPTYPE>;
    if (outPartTypes) // clear & delete if not provided null
    {
        outPartTypes->clear();
        delete outPartTypes;
    }
    outPartTypes = new std::vector<int>();

    if( boxes->empty() )
        return boxes_nms; //if there are no detections, we're supposed to return an empty vector for outPartTypes as well (it's expected outside)





    int line_length = 4*numparts+2; // nbr of elements in a line
    int detectionsNbr = boxes->size() / line_length;


    // throw away boxes if the number of candidates are too many
    // (sort by score and later consider only the Nth first)
    struct Detection *detections = sort_boxes(boxes, inPartTypes, detectionsNbr, numparts, line_length);

    //just take the best N
    for (int d=0; d<bestNToReturn; d++)
    {
        for(int i = 0; i < line_length; i++)
        {
            boxes_nms->push_back(detections[d].line[i]);
        }
    }

    for (int d=0; d<bestNToReturn; d++)
    {
        for (int p = 0; p<numparts; p++)
            outPartTypes->push_back(detections[d].partTypes[p]);
    }



    // release memory
    delete[] detections;

    return boxes_nms;
}


std::vector<FPTYPE>* nms(const std::vector<FPTYPE> *boxes, FPTYPE overlap, int numparts, /*INPUT*/const std::vector<int>& inPartTypes, /*OUTPUT*/ std::vector<int>* &outPartTypes)
{
	std::vector<FPTYPE> *boxes_nms = new std::vector<FPTYPE>;
    if (outPartTypes) // clear & delete if not provided null
    {
        outPartTypes->clear();
        delete outPartTypes;
    }
    outPartTypes = new std::vector<int>();

	if( boxes->empty() )
		return boxes_nms;

	int line_length = 4*numparts+2; // nbr of elements in a line
	int detectionsNbr = boxes->size() / line_length;

	// throw away boxes if the number of candidates are too many
	// (sort by score and later consider only the Nth first)
	struct Detection *detections = sort_boxes(boxes, inPartTypes, detectionsNbr, numparts, line_length);
	int betterDetectionsNbr = std::min(detectionsNbr, CANDIDATE_MAXNBR);

	myArray<FPTYPE> area(betterDetectionsNbr, numparts+1);
	myArray<FPTYPE> rx1(betterDetectionsNbr, 1);
	myArray<FPTYPE> ry1(betterDetectionsNbr, 1);
	myArray<FPTYPE> rx2(betterDetectionsNbr, 1);
	myArray<FPTYPE> ry2(betterDetectionsNbr, 1);

	// compute boxes area
	for(int d = 0; d < betterDetectionsNbr; d++)
	{
		rx1.set(DBL_MAX, d); // init for a min search
		ry1.set(DBL_MAX, d); // init for a min search
		rx2.set(DBL_MIN, d); // init for a max search
		ry2.set(DBL_MIN, d); // init for a max search

		for(int p = 0; p < numparts; p++)
		{
			// area of the box for part p of detection d
			area.set((X2(d,p) - X1(d,p) + 1)*(Y2(d,p) - Y1(d,p) + 1), d, p);

			rx1.set(std::min(X1(d,p), rx1.get(d)), d); // min over p
			ry1.set(std::min(Y1(d,p), ry1.get(d)), d); // min over p
			rx2.set(std::max(X2(d,p), rx2.get(d)), d); // max over p
			ry2.set(std::max(Y2(d,p), ry2.get(d)), d); // max over p
		}

		// compute the biggest boxes covering detection
		FPTYPE rarea = (rx2.get(d) - rx1.get(d) + 1.0)*(ry2.get(d) - ry1.get(d) + 1.0);
		area.set(rarea, d, numparts);
	}

#if LOGGING_ON
	writeLog("nms rx1=");
	writeLog(&rx1);
	writeLog("nms ry1=");
	writeLog(&ry1);
	writeLog("nms rx2=");
	writeLog(&rx2);
	writeLog("nms ry2=");
	writeLog(&ry2);
	writeLog("nms [area rarea]=");
	writeLog(&area);
#endif

	myArray<FPTYPE> xx1(betterDetectionsNbr, numparts+1);
	myArray<FPTYPE> yy1(betterDetectionsNbr, numparts+1);
	myArray<FPTYPE> xx2(betterDetectionsNbr, numparts+1);
	myArray<FPTYPE> yy2(betterDetectionsNbr, numparts+1);
	myArray<FPTYPE> w(betterDetectionsNbr, numparts+1);
	myArray<FPTYPE> h(betterDetectionsNbr, numparts+1);
	myArray<FPTYPE> inter(betterDetectionsNbr, numparts+1);
	myArray<FPTYPE> o(betterDetectionsNbr, numparts+1);
	myArray<FPTYPE> omax(betterDetectionsNbr, 1);

	// keep non overlapping boxes ?

#if LOGGING_ON
	bool firstIteration = true;
#endif

	while( true )
	{
		int best = betterDetectionsNbr; // invalid value

		// search the highest scored detection not yet processed
		for(int d = 0; d < betterDetectionsNbr; d++)
		{
			if( detections[d].enabled )
			{
				best = d;
				break;
			}
		}

		if( best == betterDetectionsNbr ) // invalid value
			break;
		// no more good detection, exit

		for(int i = 0; i < line_length; i++)
        {
            boxes_nms->push_back(detections[best].line[i]);
        }
        
        for (int p = 0; p < numparts; p++)
            outPartTypes->push_back(detections[best].partTypes[p]);

		// disable the detections that are similar (aka that overlap a lot) to the chosen one
		for(int d = 0; d < betterDetectionsNbr; d++)
		{
			if( ! detections[d].enabled )
				continue;

			for(int p = 0; p < numparts; p++)
			{
				xx1.set(std::max(X1(best,p), X1(d,p)), d, p); // max over p
				yy1.set(std::max(Y1(best,p), Y1(d,p)), d, p); // max over p
				xx2.set(std::min(X2(best,p), X2(d,p)), d, p); // min over p
				yy2.set(std::min(Y2(best,p), Y2(d,p)), d, p); // min over p
			}
			xx1.set(std::max(rx1.get(best), rx1.get(d)), d, numparts);
			yy1.set(std::max(ry1.get(best), ry1.get(d)), d, numparts);
			xx2.set(std::min(rx2.get(best), rx2.get(d)), d, numparts);
			yy2.set(std::min(ry2.get(best), ry2.get(d)), d, numparts);

			omax.set(DBL_MIN, d);
			for(int p = 0; p < numparts+1; p++)
			{
				w.set(std::max(xx2.get(d, p) - xx1.get(d, p) + 1.0, 0.0), d, p);
				h.set(std::max(yy2.get(d, p) - yy1.get(d, p) + 1.0, 0.0), d, p);
				inter.set(w.get(d, p) * h.get(d, p), d, p);
				o.set(inter.get(d, p) * 1.0 / area.get(best, p), d, p);
				omax.set(std::max(o.get(d, p), omax.get(d)), d);
			}

			if( omax.get(d) > overlap )
			{
				detections[d].enabled = false;
			}
		}

#if LOGGING_ON
		if( firstIteration )
		{
			writeLog("nms xx1=");
			writeLog(&xx1);
			writeLog("nms yy1=");
			writeLog(&yy1);
			writeLog("nms xx2=");
			writeLog(&xx2);
			writeLog("nms yy2=");
			writeLog(&yy2);
			writeLog("nms w=");
			writeLog(&w);
			writeLog("nms h=");
			writeLog(&h);
			writeLog("nms inter=");
			writeLog(&inter);
			writeLog("nms o=");
			writeLog(&o);
			firstIteration = false;
		}
#endif
	}

#if LOGGING_ON
	writeLog("nms top=");
	writeLog(boxes_nms, boxes_nms->size()/line_length, line_length);
#endif

	// release memory
	delete[] detections;

	return boxes_nms;
}

//------------------------------------------------------------------------------------------------------------
#else //    following block is compiled if MV_PTC is not defined
//------------------------------------------------------------------------------------------------------------

struct Detection
{
    const FPTYPE *line; // boxes corners + component id + score
    FPTYPE score;
    bool enabled;
};

/*
 * Comparison function for detection sorting.
 */
bool compare_detections(const Detection& d1, const Detection& d2)
{
    return d1.score > d2.score;
}

/*
 * Sort detections in decreasing score order (highest score first).
 */
struct Detection* sort_boxes(const std::vector<FPTYPE> *boxes, int detectionsNbr, int line_length)
{
    struct Detection *detections = new Detection[detectionsNbr];
    const FPTYPE *boxes_data = &((*boxes)[0]);
    for(int d = 0; d < detectionsNbr; d++)
    {
        detections[d].line = boxes_data + d*line_length;
        detections[d].score = detections[d].line[line_length-1];
        detections[d].enabled = true;
    }
    std::sort(&detections[0], &detections[detectionsNbr], compare_detections);
    
    return detections;
}

std::vector<FPTYPE> *not_nms_pick_best_N(const std::vector<double> *boxes, int numparts, int bestNToReturn)
{
    std::vector<FPTYPE> *boxes_nms = new std::vector<FPTYPE>;

    if( boxes->empty() )
        return boxes_nms;

    int line_length = 4*numparts+2; // nbr of elements in a line
    int detectionsNbr = boxes->size() / line_length;


    // throw away boxes if the number of candidates are too many
    // (sort by score and later consider only the Nth first)
    struct Detection *detections = sort_boxes(boxes, detectionsNbr, line_length);

    //just take the first N best
    for (int d=0; d<bestNToReturn; d++)
    {
        for(int i = 0; i < line_length; i++)
            boxes_nms->push_back(detections[d].line[i]);
    }

    // release memory
    delete[] detections;

    return boxes_nms;
}

std::vector<FPTYPE>* not_nms_pick_best(const std::vector<FPTYPE> *boxes, int numparts)
{
    std::vector<FPTYPE> *boxes_nms = new std::vector<FPTYPE>;
    
    if( boxes->empty() )
        return boxes_nms;
    
    int line_length = 4*numparts+2; // nbr of elements in a line
    int detectionsNbr = boxes->size() / line_length;
    
    
    // throw away boxes if the number of candidates are too many
    // (sort by score and later consider only the Nth first)
    struct Detection *detections = sort_boxes(boxes, detectionsNbr, line_length);
    
    //just take the best one
    for(int i = 0; i < line_length; i++)
        boxes_nms->push_back(detections[0].line[i]);
    
    // release memory
    delete[] detections;
    
    return boxes_nms;
}


std::vector<FPTYPE>* nms(const std::vector<FPTYPE> *boxes, FPTYPE overlap, int numparts)
{
    std::vector<FPTYPE> *boxes_nms = new std::vector<FPTYPE>;
    
    if( boxes->empty() )
        return boxes_nms;
    
    int line_length = 4*numparts+2; // nbr of elements in a line
    int detectionsNbr = boxes->size() / line_length;
    
    // throw away boxes if the number of candidates are too many
    // (sort by score and later consider only the Nth first)
    struct Detection *detections = sort_boxes(boxes, detectionsNbr, line_length);
    int betterDetectionsNbr = std::min(detectionsNbr, CANDIDATE_MAXNBR);
    
    myArray<FPTYPE> area(betterDetectionsNbr, numparts+1);
    myArray<FPTYPE> rx1(betterDetectionsNbr, 1);
    myArray<FPTYPE> ry1(betterDetectionsNbr, 1);
    myArray<FPTYPE> rx2(betterDetectionsNbr, 1);
    myArray<FPTYPE> ry2(betterDetectionsNbr, 1);
    
    // compute boxes area
    for(int d = 0; d < betterDetectionsNbr; d++)
    {
        rx1.set(DBL_MAX, d); // init for a min search
        ry1.set(DBL_MAX, d); // init for a min search
        rx2.set(DBL_MIN, d); // init for a max search
        ry2.set(DBL_MIN, d); // init for a max search
        
        for(int p = 0; p < numparts; p++)
        {
            // area of the box for part p of detection d
            area.set((X2(d,p) - X1(d,p) + 1)*(Y2(d,p) - Y1(d,p) + 1), d, p);
            
            rx1.set(std::min(X1(d,p), rx1.get(d)), d); // min over p
            ry1.set(std::min(Y1(d,p), ry1.get(d)), d); // min over p
            rx2.set(std::max(X2(d,p), rx2.get(d)), d); // max over p
            ry2.set(std::max(Y2(d,p), ry2.get(d)), d); // max over p
        }
        
        // compute the biggest boxes covering detection
        FPTYPE rarea = (rx2.get(d) - rx1.get(d) + 1.0)*(ry2.get(d) - ry1.get(d) + 1.0);
        area.set(rarea, d, numparts);
    }
    
#if LOGGING_ON
    writeLog("nms rx1=");
    writeLog(&rx1);
    writeLog("nms ry1=");
    writeLog(&ry1);
    writeLog("nms rx2=");
    writeLog(&rx2);
    writeLog("nms ry2=");
    writeLog(&ry2);
    writeLog("nms [area rarea]=");
    writeLog(&area);
#endif
    
    myArray<FPTYPE> xx1(betterDetectionsNbr, numparts+1);
    myArray<FPTYPE> yy1(betterDetectionsNbr, numparts+1);
    myArray<FPTYPE> xx2(betterDetectionsNbr, numparts+1);
    myArray<FPTYPE> yy2(betterDetectionsNbr, numparts+1);
    myArray<FPTYPE> w(betterDetectionsNbr, numparts+1);
    myArray<FPTYPE> h(betterDetectionsNbr, numparts+1);
    myArray<FPTYPE> inter(betterDetectionsNbr, numparts+1);
    myArray<FPTYPE> o(betterDetectionsNbr, numparts+1);
    myArray<FPTYPE> omax(betterDetectionsNbr, 1);
    
    // keep non overlapping boxes ?
    
#if LOGGING_ON
    bool firstIteration = true;
#endif
    
    while( true )
    {
        int best = betterDetectionsNbr; // invalid value
        
        // search the highest scored detection not yet processed
        for(int d = 0; d < betterDetectionsNbr; d++)
        {
            if( detections[d].enabled )
            {
                best = d;
                break;
            }
        }
        
        if( best == betterDetectionsNbr ) // invalid value
            break;
        // no more good detection, exit
        
        for(int i = 0; i < line_length; i++)
            boxes_nms->push_back(detections[best].line[i]);
        
        // disable the detections that are similar (aka that overlap a lot) to the chosen one
        for(int d = 0; d < betterDetectionsNbr; d++)
        {
            if( ! detections[d].enabled )
                continue;
            
            for(int p = 0; p < numparts; p++)
            {
                xx1.set(std::max(X1(best,p), X1(d,p)), d, p); // max over p
                yy1.set(std::max(Y1(best,p), Y1(d,p)), d, p); // max over p
                xx2.set(std::min(X2(best,p), X2(d,p)), d, p); // min over p
                yy2.set(std::min(Y2(best,p), Y2(d,p)), d, p); // min over p
            }
            xx1.set(std::max(rx1.get(best), rx1.get(d)), d, numparts);
            yy1.set(std::max(ry1.get(best), ry1.get(d)), d, numparts);
            xx2.set(std::min(rx2.get(best), rx2.get(d)), d, numparts);
            yy2.set(std::min(ry2.get(best), ry2.get(d)), d, numparts);
            
            omax.set(DBL_MIN, d);
            for(int p = 0; p < numparts+1; p++)
            {
                w.set(std::max(xx2.get(d, p) - xx1.get(d, p) + 1.0, 0.0), d, p);
                h.set(std::max(yy2.get(d, p) - yy1.get(d, p) + 1.0, 0.0), d, p);
                inter.set(w.get(d, p) * h.get(d, p), d, p);
                o.set(inter.get(d, p) * 1.0 / area.get(best, p), d, p);
                omax.set(std::max(o.get(d, p), omax.get(d)), d);
            }
            
            if( omax.get(d) > overlap )
            {
                detections[d].enabled = false;
            }
        }
        
#if LOGGING_ON
        if( firstIteration )
        {
            writeLog("nms xx1=");
            writeLog(&xx1);
            writeLog("nms yy1=");
            writeLog(&yy1);
            writeLog("nms xx2=");
            writeLog(&xx2);
            writeLog("nms yy2=");
            writeLog(&yy2);
            writeLog("nms w=");
            writeLog(&w);
            writeLog("nms h=");
            writeLog(&h);
            writeLog("nms inter=");
            writeLog(&inter);
            writeLog("nms o=");
            writeLog(&o);
            firstIteration = false;
        }
#endif
    }
    
#if LOGGING_ON
    writeLog("nms top=");
    writeLog(boxes_nms, boxes_nms->size()/line_length, line_length);
#endif
    
    // release memory
    delete[] detections;
    
    return boxes_nms;
}
#endif // MV_PTC
