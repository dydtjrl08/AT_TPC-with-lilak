#include "STDMCViewer.h"

ClassImp(STDMCViewer);

STDMCViewer::STDMCViewer()
{
    fName = "STDMCViewer";
}

bool STDMCViewer::Init()
{
    fDetector = (TPCDrum *) fRun -> GetDetector();
    fPadPlane = (STDPadPlane*) fDetector -> GetDetectorPlane();
    fHitArray = fRun -> GetBranchA("Hit");
    fTrackArray = fRun -> GetBranchA("MCTrack");
    fStepArray = fRun -> GetBranchA("MCStepTPCDrum");


    hPoly = fPadPlane -> GetPadPlanePoly();
    hPoly -> GetZaxis() -> SetRangeUser(1., 4000);

    double x[4] = {-100., -100., 100., 100.};
    double y[4] = {-34., 166., 166., -34.};

    hBoundary = new TH2Poly();
    hBoundary -> SetStats(0);
    hBoundary -> SetTitle("; x [mm]; y [mm]");
    hBoundary -> AddBin(4, x, y);

    cEvent = new TCanvas("cEvent", "", 600, 600);

    hHitNum = new TH1I("hHitNum","",50, 0, 50);
    hRowHitNum = new TH1I("hRowHitNum","",10, 0, 10);
    hSumADC = new TH1D("hHitNum","",100, 0, 25000);

    gTrack.clear();
    for(int i=0; i<10; i++){
        gTrack.push_back(new TGraph());
        gTrack[i] -> SetLineColor(kRed);
        gTrack[i] -> SetLineWidth(2.);
    }

    return true;
}

void STDMCViewer::Exec(Option_t *option)
{
    hPoly -> ClearBinContents();
    
    double maxW = 0.;
    int maxSection = -1;
    int maxLayer = 0.;
    int maxRow = 0.;
    int maxHitIdx = -1;
    int hitNum = fHitArray -> GetEntries();

    double sumADC = 0.;
    double adc[12][64];
    memset(adc, 0, sizeof(adc));
    for(int hit=0; hit<hitNum; hit++){
        fHit = (LKHit*)fHitArray -> UncheckedAt(hit);
        int padID = fHit -> GetPadID();
        int layer = fHit -> GetLayer();
        int row = fHit -> GetRow();
        double x = fHit -> X();
        double y = fHit -> Y();
        double w = fHit -> W();
        double tb = fHit -> GetTb();

        hPoly -> Fill(x, y, w);

        sumADC += w;

        adc[layer][row] = w;
    }

    if(hitNum > 0){
        hHitNum -> Fill(hitNum);
        hSumADC -> Fill(sumADC);
    }

    for(int i=0; i<768; i++){
        double x = fPadPlane -> GetX(i);
        double y = fPadPlane -> GetY(i);
        hPoly -> Fill(x, y, 1.);
    }

    for(int i=0; i<10; i++){
        gTrack[i] -> Set(0);
    }

    int stepNum = fStepArray -> GetEntries();
    for(int i=0; i<stepNum; i++){
        fStep = (LKMCStep*)fStepArray -> UncheckedAt(i);
        double x = fStep -> GetX();
        double y = fStep -> GetY();
        double z = fStep -> GetZ();
        int trackId = fStep -> GetTrackID() -1;
        bool isIn = fDetector -> IsInBoundary(x, y, z);
        if(!isIn){continue;}
        gTrack[trackId] -> SetPoint(gTrack[trackId]->GetN(), x, y);
    }

    cEvent -> cd();
    hBoundary -> Draw("");
    hPoly -> Draw("colz, same");
    for(int i=0; i<10; i++){
        gTrack[i] -> Draw("same, l");
    }
    cEvent -> Update();
    cEvent -> SaveAs(Form("./figure/Event%i.pdf", int(fRun -> GetCurrentEventID()) ));
}

bool STDMCViewer::EndOfRun()
{
    return true;
}