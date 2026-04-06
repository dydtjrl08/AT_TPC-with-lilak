#include "STDRunManager.h"

ClassImp(STDRunManager);

STDRunManager::STDRunManager()
: fIsDAQStage(false), fInputRun(""), fRejectRun(""), fTotalEventNum(-1), fEventNumByRun(-1), fCurrentEventIdx(0)
{
}

bool STDRunManager::Init()
{
    if(fInputRun != ""){
        fRunList = SejongDAQFlow::GetRunList(fInputRun, fRejectRun, fIsDAQStage);
        if(fIsDAQStage){
            fDecoder = new STDDecoder();
            LKRun::SetEventTrigger(fDecoder);
        }
    }
    // else{lk_error << "STDRunManager::Init() There is no set the Input Run List!!!" << endl;}
    LKRun::SetAutoTermination(false);

    return true;
}

bool STDRunManager::Run()
{
    if(fIsDAQStage){
        for(int run=0; run<fRunList.size(); run++){
            LKRun::SetOutputFile(Form("./test_%i.root", fRunList[run].first));
            LKRun::Init();

            // for AGET Decoder
            fDecoder -> Clear();
            if(fTotalEventNum != -1){fDecoder -> SetEventNumber(fTotalEventNum-fCurrentEventIdx-1);}
            if(fEventNumByRun != -1){fDecoder -> SetEventNumber(fEventNumByRun-1);}
            fDecoder -> SetRunFile(fRunList[run].second);
        
            // Main Run process
            LKRun::Run();

            if(fTotalEventNum != -1){fCurrentEventIdx += fDecoder -> GetTotalEventNumber();}
        }
    }
    else{
        for(int run=0; run<fRunList.size(); run++){
            LKRun::AddInputFile(Form("./test_%i.root", fRunList[run].first));
        }
        LKRun::Init();
        LKRun::Run();
    }

    LKRun::Terminate(this);
    return true;
}

bool STDRunManager::EndOfRun()
{
    return true;
}

void STDRunManager::SetDatapath(TString path){fDataPath = path;}
void STDRunManager::SetRunList(TString list){fInputRun = list;}
void STDRunManager::SetRejectRun(TString list){fRejectRun = list;}

void STDRunManager::SetEventNumber(int event){fTotalEventNum = event;}
void STDRunManager::SetEventNumByRun(int event){fEventNumByRun = event;}

void STDRunManager::SetDAQStage(){fIsDAQStage = true;}
