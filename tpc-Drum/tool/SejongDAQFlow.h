#ifndef SEJONGDAQFLOW_HH
#define SEJONGDAQFLOW_HH

#include "TSystem.h"
#include "TSystemFile.h"
#include "TSystemDirectory.h"
#include "TString.h"
#include "TMath.h"
#include "TVector3.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TObject.h"

#include <iostream>
#include <fstream>
#include <queue>
#include <vector>

using namespace std;

typedef vector<pair<Int_t, vector<TString>>> RunList;
typedef vector<queue<TString>> DAQList;

static TString kDataBasePath = "/data/public_data/";

#define ASADNUM 4
#define AGETNUM 4
#define CHANNUM 68  
#define TIMEBUCKET 512

class SejongDAQFlow
{
    public:
        SejongDAQFlow();
        virtual ~SejongDAQFlow(){}

        // parsing function
        static TString AddDash(TString path);
        static RunList GetRunList(TString input, TString rejectRun, bool isDAQStage);
        static DAQList GetDAQList(vector<TString> fileList); // Note!!! this function mainly used in only AGETDecoder, it must process for only CoBo format naming files
        static vector<TString> GetDAQFiles(TString path); // For only online mode
        static vector<Int_t> GetRunsFromString(TString run);

    private:

        // parsing function
        static Bool_t CheckRunIDFormat(TString run);
        static RunList RemovedRejectRun(RunList runList, vector<Int_t> rejectRun);        
        static RunList GetLines(TString file, bool isDAQStage);
        static vector<TString> GetDataBaseFile(TString run, bool isDAQStage);
        static vector<TString> GetDataBaseFile(int runID, bool isDAQStage);
        static double GetFileTime(TString fileName); // [sec]
};

#endif
