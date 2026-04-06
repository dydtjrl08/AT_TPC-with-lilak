void InputCutInfomation(TCutG*& cutData){
    cutData = new TCutG("cutData",15);
    //this is for data cut
    cutData->SetPoint( 0, 180,   17*1000);
    cutData->SetPoint( 1, 205,   17*1000);
    cutData->SetPoint( 2, 212, 20.5*1000);
    cutData->SetPoint( 3, 235, 63.5*1000);
    cutData->SetPoint( 4, 260,   90*1000);
    cutData->SetPoint( 5, 270,  113*1000);
    cutData->SetPoint( 6, 272,  133*1000);
    cutData->SetPoint( 7, 260,  154*1000);
    cutData->SetPoint( 8, 220,  138*1000);
    cutData->SetPoint( 9, 205,  120*1000);
    cutData->SetPoint(10, 197,  102*1000);
    cutData->SetPoint(11, 192,   72*1000);
    cutData->SetPoint(12, 170,   35*1000);
    cutData->SetPoint(13, 170,   35*1000);
    cutData->SetPoint(14, 180,   20*1000);


}



double tbToZ(int tb){
    double z = tb * 0.46; // in mm
    return z;
}
void trackreco_eventViewer(){
    ROOT::EnableImplicitMT(25);
    TCutG *cutData = nullptr;
    InputCutInfomation(cutData);
    TFile* filein = new TFile("./test_250818005.root","read");

    TTree* treein = new TTree();
    treein=(TTree*)filein->Get("event");
    TClonesArray *hits = nullptr;
    treein->SetBranchAddress("Hit",&hits);

    STDPadPlane* fPadPlane = new STDPadPlane();
    fPadPlane->Init();
    TH2Poly* PadPlane = fPadPlane->GetPadPlanePoly();

    int EventNum = treein->GetEntries();
    cout <<"Total Event Number : "<< EventNum << endl;

    TCanvas* c1 = new TCanvas("c1","c1",800,600);
    TGraphErrors* fitLayerGraph = new TGraphErrors();
    TF1* fit1 = new TF1("fit1","pol1",-100000,100000);
    PadPlane->Draw("colz");

    TH1D* ADC_distribution = new TH1D("ADC_distribution",";ADC [a.u.];Counts",100,0,260000);
    TH1D* TrackLength_Distribution = new TH1D("TrackLength_Distribution",";Track Length [mm];Counts",65,110,180);
    TH2D* TrackLength_ADC_Distribution = new TH2D("TrackLength_ADC_Distribution",";Track Length [mm];ADC [a.u.]",100,100,150,100,0,260000);
    TH1D* ClusteredPositionXonY0 = new TH1D("",";X position [mm];Counts",100,-25,25);
    TH2D* DataIndexVsADC = new TH2D("DataIndexVsADC",";Pad HitNum;ADC [a.u.]",100,50,350,100,0,260000);
    TH2D* TimeDistribution_YTb = new TH2D("TimeDistribution_YTb",";Y position [mm];Time Bin",12,-6,138,512,-0.5,512.5);
    TH1D* TimeBinDistribution = new TH1D("TimeBinDistribution",";Time Bin;Counts",1024,-511,511);
    TH1D* TrackLengthDiffDistribution = new TH1D("TrackLengthDiffDistribution",";Track Length Difference [mm];Counts",1024,-511,511);
    double padData[12][64][4]; //layer, row, [x,y,w,tb]
    double ClusteredData[12]; // position data for each layer ( x position )


    for(int event = 10 ; event <100000; event++){
        if(event % 1000 == 0) cout << "Processing Event : " << event << endl;

        fitLayerGraph->Set(0); // Reset the graph for each event
        memset(padData, 0, sizeof(padData));
        memset(ClusteredData, 0, sizeof(ClusteredData));
        treein->GetEntry(event);
        PadPlane->Clear();
        int padHitted = hits->GetEntriesFast();
        if(!padHitted) continue;

        int interactedLayer = 0;
        int Wtotal = 0 ;

        for(int data = 0 ; data <padHitted; data++){
            LKHit* fhit= (LKHit*)hits->UncheckedAt(data);
            int Layer = fhit->GetLayer();
            int row   = fhit->GetRow();

            double X = fhit->x();
            double Y = fhit->y();
            double W = fhit->W();
            
            if(W < 150) continue; // ADC 범위 필터링
            // if(W < 150 || W > 4000) continue; // ADC 범위 필터링
            padData[Layer][row][0] = X;
            padData[Layer][row][1] = Y;
            padData[Layer][row][2] = W;
            padData[Layer][row][3] = fhit->GetTb();
            Wtotal += W;

        }

        // total ADC sum cut 
        if(!cutData->IsInside(padHitted,Wtotal))continue;
        //

        for(int layer = 0 ; layer < 12 ; layer++){
            double layerWtotal = 0;
            double ClusteredPositionX = 0;
            for(int row = 0 ; row < 64 ; row++){
                ClusteredPositionX += padData[layer][row][0] * padData[layer][row][2];
                layerWtotal += padData[layer][row][2];
            }
            


            if(layerWtotal > 0) {
                ClusteredPositionX /= layerWtotal;
                ClusteredData[layer] = ClusteredPositionX;
                fitLayerGraph->SetPoint(fitLayerGraph->GetN(), ClusteredPositionX,layer*12);
                fitLayerGraph->SetPointError(fitLayerGraph->GetN()-1, 0, 1/sqrt(layerWtotal)); // Error as sqrt of total weight
                
                
                interactedLayer++;
            } else {
                ClusteredData[layer] = 0; // No hits in this layer
            }
        }
        if(interactedLayer<11) continue; // Skip if less than 11 layers are interacted
        double ADC_Sum = 0;
        double minTb = 512;
        double maxTb = 0;

        int layerHighest = 0;
        int layerLowest = 11;
        for(int layer = 0 ; layer < 12 ; layer++){
            double LayerADCsum = 0;
            double totaltb =0;

            
            for(int row = 0 ; row < 64 ; row++){
                double X = padData[layer][row][0];
                double Y = padData[layer][row][1];
                double W = padData[layer][row][2];
                double tb = padData[layer][row][3];
                PadPlane->Fill(X, Y, W);
                TimeDistribution_YTb->Fill(Y,tb,W);
                ADC_Sum += W;
                LayerADCsum += W;
                totaltb += tb * W;
                
            }
            double avgTb = totaltb / LayerADCsum;
            
            if(avgTb < minTb){
                layerLowest = layer;
                 minTb = avgTb;
            }
            if(avgTb > maxTb){
                maxTb = avgTb;
                layerHighest = layer;
            }


        }
        
        double tbDiff = maxTb - minTb;
        if(layerHighest-layerLowest < 0)tbDiff *= -1;
        // cout << tbDiff << endl;
        TimeBinDistribution->Fill(tbDiff);

        ADC_distribution->Fill(ADC_Sum);
        fitLayerGraph->Fit("fit1","Q");
    
        ClusteredPositionXonY0->Fill(fit1->GetX(0));
        double trackLength =0;
        for(int layer = 11 ; layer >=0 ; layer--){
            if(ClusteredData[layer] != 0) {
                trackLength = sqrt(pow(layer*12,2)+pow(fit1->GetX(layer*12)-fit1->GetX(0),2)+pow(tbToZ(tbDiff),2));
                if(trackLength> 145)TrackLengthDiffDistribution->Fill(tbDiff);
                TrackLength_Distribution->Fill(trackLength);
                break;
            }
        }
        DataIndexVsADC->Fill(padHitted, ADC_Sum);
        // cout << trackLength << endl;
        TrackLength_ADC_Distribution->Fill(trackLength, ADC_Sum);
        // fit1->DrawCopy("same");

        
        
        
        
    }

    


    // PadPlane->Draw("colz same");
    c1->Update();
    c1->SaveAs(Form("event_total123cut.png"));

    c1->Clear();
    gStyle->SetOptStat(0);
    ClusteredPositionXonY0->SetLineColor(kRed);
    ClusteredPositionXonY0->Draw();
    TF1* fitgaus = new TF1("fitgaus","gaus",-2.5,2.5);
    ClusteredPositionXonY0->Fit("fitgaus","QR");
    fitgaus->SetLineColor(kBlue);
    fitgaus->Draw("same");
    TLatex *lat = new TLatex();
    lat->SetTextSize(0.03);
    lat->DrawLatexNDC(0.15,0.85,Form("Mean : %.2f, Sigma : %.2f", fitgaus->GetParameter(1), fitgaus->GetParameter(2)));
    c1->Update();
    c1->SaveAs(Form("ClusteredPositionXonY0cut.png"));

    c1->Clear();
    gStyle->SetOptStat(0);
    ADC_distribution->SetLineColor(kRed);
    ADC_distribution->Draw();
    c1->Update();
    c1->SaveAs(Form("histo_SumRawADCcut.png"));

    c1->Clear();
    gStyle->SetOptStat(0);
    TrackLength_Distribution->SetLineColor(kRed);
    TrackLength_Distribution->Draw();
    TF1* fitgaus2 = new TF1("fitgaus2","gaus",131,142);
    
    TrackLength_Distribution->Fit("fitgaus2","QR");
    cout << fitgaus2->GetParameter(1) << " " << fitgaus2->GetParameter(2) << endl;
    fitgaus2->SetLineColor(kBlue);
    fitgaus2->Draw("same");
    c1->Update();
    c1->SaveAs(Form("TrackLength_Distributioncut.png"));

    c1->Clear();
    gStyle->SetOptStat(0);
    TrackLength_ADC_Distribution->SetLineColor(kRed);
    TrackLength_ADC_Distribution->Draw("colz");
    c1->Update();
    c1->SaveAs(Form("TrackLength_ADC_Distributioncut.png"));

    c1->Clear();
    gStyle->SetOptStat(0);
    DataIndexVsADC->SetLineColor(kRed);
    DataIndexVsADC->Draw("colz");
    cutData->Draw("same");
    c1->Update();
    c1->SaveAs(Form("DataIndexVsADCcut.png"));

    c1->Clear();
    gStyle->SetOptStat(0);
    TimeDistribution_YTb->SetLineColor(kRed);
    TimeDistribution_YTb->Draw("colz");
    c1->Update();
    c1->SaveAs(Form("TimeDistribution_YTbcut.png"));


    c1->Clear();
    gStyle->SetOptStat(0);
    TimeBinDistribution->SetLineColor(kRed);
    TimeBinDistribution->Draw();
    c1->Update();
    c1->SaveAs(Form("TimeBinDistributioncut.png"));

    c1->Clear();
    gStyle->SetOptStat(0);
    TrackLengthDiffDistribution->SetLineColor(kRed);
    TrackLengthDiffDistribution->Draw();
    c1->Update();
    c1->SaveAs(Form("TrackLengthDiffDistributioncut.png"));


}
