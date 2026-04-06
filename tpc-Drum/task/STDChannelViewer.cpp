#include "STDChannelViewer.h"

ClassImp(STDChannelViewer);

STDChannelViewer::STDChannelViewer()
{
    fName = "STDChannelViewer";
}

bool STDChannelViewer::Init()
{
    fDetector = (TPCDrum *) fRun -> GetDetector();
    fPadPlane = (STDPadPlane*) fDetector -> GetDetectorPlane();
    fHitArray = fRun -> GetBranchA("Hit");

    hPoly = fPadPlane -> GetPadPlanePoly();
    hPoly -> GetZaxis() -> SetRangeUser(1., 4000);

    double x[4] = {-100., -100., 100., 100.};
    double y[4] = {-34., 166., 166., -34.};

    hBoundary = new TH2Poly();
    hBoundary -> SetStats(0);
    hBoundary -> SetTitle("; x [mm]; y [mm]");
    hBoundary -> AddBin(4, x, y);

    cEvent = new TCanvas("cEvent", "", 600, 600);

    gCluster = new TGraph();
    gCluster -> SetMarkerStyle(20);
    gCluster -> SetMarkerSize(0.7);
    gCluster -> SetMarkerColor(kRed);

    hFitter = new TF1("fitter", "pol1", -100, 200.);

    hHitNum = new TH1I("hHitNum","",50, 0, 50);
    hRowHitNum = new TH1I("hRowHitNum","",10, 0, 10);
    hSumADC = new TH1D("hHitNum","",100, 0, 25000);
    hHitADC = new TH2D("hHitNum","", 50, 0, 50., 100, 0., 25000);

    outFile = new TFile("alphaTrkPar.root","recreate");
    outTree = new TTree("event", "event");
    outTree -> Branch("trkPar", fit, "fit[3]/D");

    return true;
}

void STDChannelViewer::Exec(Option_t *option)
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
        hHitADC -> Fill(hitNum, sumADC);
    }

    // for(int i=0; i<768; i++){
    //     double x = fPadPlane -> GetX(i);
    //     double y = fPadPlane -> GetY(i);
    //     hPoly -> Fill(x, y, 1.);
    // }

    // hFitter -> SetParameters(0., 0.);
    // gCluster -> Set(0);

    // double maxY = 0.;
    // for(int layer=0; layer<12; layer++){
    //     double y = fPadPlane -> GetY(layer, 30);
    //     double sumADC = 0.;
    //     double weightedPos = 0.;

    //     int rowNum = 0;
    //     for(int row=0; row<64; row++){
    //         double x = fPadPlane -> GetX(layer, row);
    //         double w = adc[layer][row];
    //         if(w < 40.){continue;}

    //         weightedPos += (w*x);
    //         sumADC += w;
    //         rowNum++;
    //     }
    //     weightedPos /= sumADC;

    //     if(rowNum > 0){hRowHitNum -> Fill(rowNum);}

    //     if(rowNum < 2){continue;}
    //     gCluster -> SetPoint(gCluster->GetN(), weightedPos, y);
    //     if(maxY < y){maxY = y;}
    // }


    // event viwer 
    // memset(fit, 0, sizeof(fit));
    // if(gCluster->GetN() < 3){return;}
    // gCluster -> Fit(hFitter, "Q");

    // cout << hFitter->GetChisquare() << endl;

    // if(hFitter->GetChisquare() > 50){return;}
    // fit[0] = hFitter->GetParameter(0); 
    // fit[1] = hFitter->GetParameter(1); 
    // fit[2] = maxY;
    // outTree -> Fill();

    if(hitNum < 6 || sumADC < 10000.){return;}

    cEvent -> cd();
    hBoundary -> Draw("");
    hPoly -> Draw("colz, same");
    // gCluster -> Draw("same, p");
    cEvent -> Update();
    cEvent -> SaveAs(Form("./figure/Event%i.pdf", int(fRun -> GetCurrentEventID()) ));
}

bool STDChannelViewer::EndOfRun()
{
    TCanvas* c1 = new TCanvas("","",1200,1200);
    c1 -> Divide(2,2);

    c1 -> cd(1);
    hHitNum -> SetStats(0);
    hHitNum -> SetTitle("; Hit Num; Counts");
    hHitNum -> Draw();

    c1 -> cd(2);
    hSumADC -> SetStats(0);
    hSumADC -> SetTitle("; Sum of ADC; Counts");
    hSumADC -> Draw();

    c1 -> cd(3);
    hHitADC -> SetStats(0);
    hHitADC -> SetTitle("; Row Hit num; Counts");
    hHitADC -> Draw("colz");

    c1 -> Draw();
    c1 -> SaveAs("./alphaSummary.pdf");

    outFile -> cd();
    outTree -> Write();
    outFile -> Close();

    return true;
}