#ifndef STDDECODER_HH
#define STDDECODER_HH

#include "TClonesArray.h"
#include "LKLogger.h"
#include "LKParameterContainer.h"
#include "LKRun.h"
#include "LKTask.h"
#include "GETChannel.h"
#include "TPCDrum.h"
#include "STDPadPlane.h"

#include "SejongDAQFlow.h"
#include "GETDecoder.h"
#include "GETBasicFrame.h"

class STDDecoder : public LKTask
{
    public:
        STDDecoder();
        virtual ~STDDecoder(){}

        bool Init();
        bool Clear();
        void Run(Long64_t numEvents=-1);
        bool EndOfRun();

        void SetRunFile(vector<TString> fileList);

        void SetEventNumber(int event); // set the limit of number of event by runs
        Int_t GetTotalEventNumber(); // get the number of event after running

    private:
        Int_t FileOpen(int asadIdx);
        Int_t AddFiles();
        Int_t CheckEvent(int asadIdx);

        Int_t ReadHeader(int asadIdx);
        Int_t ReadItem(int asadIdx);

        Int_t FillData();

        TPCDrum* fDetector = nullptr;
        STDPadPlane* fDetectorPlane = nullptr;

        int fEventIdx = 0;
        int fEventNum = -1;

        int fAsAdNum = 0;

        TClonesArray* fEventHeaderArray = nullptr;
        TClonesArray* fChannelArray = nullptr;

        GETDecoder* fDecoder = nullptr;
        GETChannel* fChannel = nullptr;

        DAQList fDAQList;
        GETBasicFrame *fFrame[ASADNUM];

        // HeaderFrame mHeader;
        // ItemFrame mItem;

    ClassDef(STDDecoder,1);
};

#endif
