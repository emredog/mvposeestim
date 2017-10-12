#ifndef ESTIMATEYRERRORCNN_H
#define ESTIMATEYRERRORCNN_H

#include <map>
#include <string>
#include <vector>



#ifdef HUMANEVA
#define KEY_COLS 4
#define CSV_COLUMNS 30 //FIXME: it's actually KEY_COLS + NUMPARTS
#elif defined(UMPM)
#define KEY_COLS 3
#define CSV_COLUMNS 29 //FIXME: it's actually KEY_COLS + NUMPARTS
#endif


class EstimateYrErrorCNN
{
public:
    EstimateYrErrorCNN(const char *csvFileName);
#ifdef HUMANEVA
    std::vector<FPTYPE> getErrors(std::string subject, std::string action, std::string view, int frame); // TODO
#elif defined(UMPM)
    std::vector<FPTYPE> getErrors(std::string action, std::string view, int frame);
#endif
protected:
    void init(const char *csvFileName);

private:
    std::map<std::string, std::vector<FPTYPE> > errorBase;
};

#endif // ESTIMATEYRERRORCNN_H
