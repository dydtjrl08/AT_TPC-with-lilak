#ifndef LKRUNMANAGER_HH
#define LKRUNMANAGER_HH

#include <iostream>

#include "TClonesArray.h"
#include "LKLogger.h"
#include "LKParameterContainer.h"
#include "LKRun.h"

#include "SejongDAQFlow.h"
#include "STDDecoder.h"

// run files clustering and managing, to be updated
// multi-processing mode with this manager will update... 

class STDRunManager : public LKRun
{
    public:
        STDRunManager();
        virtual ~STDRunManager(){}

        bool Init();
        bool Run();
        bool EndOfRun();

        void Print();

        // Avaliable input run list format: 
        // 1. Text files ex) runlist.list .lis. .text .txt
        // 2. Single run number ex) 240101001
        // 3. List of run number ex) 240101001, 240101002, 240101003
        void SetDatapath(TString path);
        void SetRunList(TString list); 
        void SetRejectRun(TString list);

        void SetEventNumber(int event);
        void SetEventNumByRun(int event);

        void SetDAQStage(); // Only convert mode from DAQ data (graw files) to LILAK format ROOT files 

    private:
        STDDecoder* fDecoder = nullptr;

        bool fIsDAQStage;
        TString fDataPath;
        TString fInputRun;
        TString fRejectRun;
        RunList fRunList;

        int fTotalEventNum;
        int fEventNumByRun;
        int fCurrentEventIdx;

    ClassDef(STDRunManager,1);
};

#endif