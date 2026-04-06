#include "STDEventViewer.h"

ClassImp(STDEventViewer);

STDEventViewer::STDEventViewer()
{
    fName = "STDEventViewer";
}

bool STDEventViewer::Init()
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
    for(int i=0; i<4; i++){
        hSumADC[i] = new TH1D(Form("hSumADC%i", i),"", 1000, 0, 10000);
    }
    hSumADC[1] -> SetLineColor(kRed);
    hSumADC[2] -> SetLineColor(kGreen+2);
    hSumADC[3] -> SetLineColor(kYellow+2);

    hHitNum = new TH1I("hHitNum","",30, 0, 30);

    return true;
}

void STDEventViewer::Exec(Option_t *option)
{
    hPoly -> ClearBinContents();
    
    double maxW = 0.;
    int maxSection = -1;
    int maxLayer = 0.;
    int maxRow = 0.;
    int maxHitIdx = -1;
    int hitNum = fHitArray -> GetEntries();

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
       
        if(maxW < w){
            maxW = w;
            maxHitIdx = hit;
            maxLayer = layer;
            maxRow = row;
            maxSection = fPadPlane -> GetSectionID(padID);
        }
    }

    for(int i=0; i<768; i++){
        double x = fPadPlane -> GetX(i);
        double y = fPadPlane -> GetY(i);
        hPoly -> Fill(x, y, 1.);
    }

    // if(maxW > 3500 || maxW < 50){return;}

    // double hitWMatrix[3][5];
    // memset(&hitWMatrix, 0., sizeof(hitWMatrix));
    // hitWMatrix[1][2] = maxW;

    // int MaxRowIdxByLayer[3];
    // double tmpMaxWByLayer[3];
    // fill_n(&MaxRowIdxByLayer[0], 3, -1);
    // memset(&tmpMaxWByLayer, 0., sizeof(tmpMaxWByLayer));

    // int neighberHitNum = -1; // except the max hit
    // for(int hit=0; hit<hitNum; hit++){
    //     fHit = (LKHit*)fHitArray -> UncheckedAt(hit);
    //     int layer = fHit -> GetLayer();
    //     int row = fHit -> GetRow();
    //     int maxCenteredLayer = 1 + (layer - maxLayer);
    //     int maxCenteredRow = 2 + (row - maxRow);

    //     if(abs(maxCenteredLayer) <= 2 && abs(maxCenteredRow) < 5){
    //         double w = fHit -> W();
    //         hitWMatrix[maxCenteredLayer][maxCenteredRow] = w;
    //         neighberHitNum++;

    //         for(int layer=0; layer<3; layer++){
    //             if(maxCenteredLayer == layer && tmpMaxWByLayer[layer] < w){
    //                 tmpMaxWByLayer[layer] = w;
    //                 MaxRowIdxByLayer[layer] = maxCenteredRow;
    //             }
    //         }
    //     }
    // }

    // cout << " ================================ " << endl;
    // for(int l=2; l>=0; l--){
    //     for(int r=0; r<5; r++){
    //         int w = hitWMatrix[l][r];
    //         TString wstring = Form("%i", w);
    //         int wStringSize = wstring.Sizeof();
    //         wstring = "";
    //         for(int i=0; i<5-wStringSize; i++){
    //             wstring += " ";
    //         }
    //         cout << w << Form("%s, ", wstring.Data());
    //     }
    //     cout << endl;
    // }
    // cout << " ================================ " << endl;

    // double sumADC = 0.;
    // for(int layer=0; layer<3; layer++){
    //     int maxRowIdx = MaxRowIdxByLayer[layer];
    //     if(maxRowIdx < 0 || abs(maxRowIdx - MaxRowIdxByLayer[1]) > 1){continue;}

    //     cout << " layer: " << layer << " row: " << maxRowIdx << "| sumADC += " << hitWMatrix[layer][maxRowIdx] << endl;
    //     sumADC += hitWMatrix[layer][maxRowIdx];

    //     for(int step=0; step<2; step++){
    //         for(int side=0; side<2; side++){
    //             int stepSign = (side==0)? -1 : +1;
    //             int currIdx = maxRowIdx + stepSign*step;
    //             int prevIdx = currIdx + stepSign;

    //             double currStepW = hitWMatrix[layer][currIdx];
    //             double prevStepW = hitWMatrix[layer][prevIdx];

    //             if(currStepW > prevStepW){
    //                 if(prevStepW < 50){continue;}
    //                 sumADC += prevStepW;
    //                 // cout << " layer " << layer << " step " << step << " side " << side << " | " << " maxRowByLayer: " << maxRowIdx  << " idx: "  << currIdx << " " << prevIdx << endl;
    //                 cout << " OK ! " <<  currStepW << " > " << prevStepW  << endl;
    //             }
    //         }
    //     }
    // }
    
    // if(2 <= neighberHitNum){
    //     lk_info << "number of neighber Hit: " << neighberHitNum << " Sum of ADC: " << sumADC << endl;
    //     hSumADC[0] -> Fill(sumADC);
    //     hSumADC[maxSection+1] -> Fill(sumADC);
    // }
    
    // hHitNum -> Fill(hitNum);

    // event viwer 
    // cEvent -> cd();
    // // gPad -> SetLogz();
    // hBoundary -> Draw("");
    // hPoly -> Draw("colz, same");
    // cEvent -> Draw();
    // cEvent -> SaveAs(Form("./alphaFigure/alpha_fullCoBo_Event%i.pdf", int(fRun -> GetCurrentEventID()) ));
}

bool STDEventViewer::EndOfRun()
{
    TCanvas* c1 = new TCanvas("","",1200,1200);
    c1 -> Divide(2,2);

    c1 -> cd(1);
    hPoly -> Draw("colz");

    c1 -> cd(2);
    hHitNum -> Draw();

    c1 -> cd(3);
    hSumADC[0] -> Draw();
    for(int i=0; i<3; i++){
        hSumADC[i+1] -> Draw("same");
    }


    c1 -> Draw();
    c1 -> SaveAs("./eventTotal.pdf");

    outFile = new TFile(Form("hitHisto_%s.root",runNum.Data()), "recreate");
    outFile -> cd();
    for(int i=0; i<4; i++){
        hSumADC[i] -> Write();
    }
    outFile -> Close();

    return true;
}