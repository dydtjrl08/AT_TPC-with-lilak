bool BranchCut(TTree* &tree);
double TBtoZ(double tb);

void EventViewer250911()
{
    //Turn stop watch on

    time_t start = time(NULL);
    TRandom3* rd = new TRandom3();
    rd->SetSeed(0);

    // file open and tree open
    TFile* filein = new TFile("./test_250818005.root","read");
    TTree* treein = (TTree*)filein->Get("event");

    // set branch cut ( Raw data is too large to handle all branches )
    TClonesArray *hits = nullptr;
    BranchCut(treein);
    treein->SetBranchAddress("Hit",&hits);
    
    // get total event number
    int EventNum = treein->GetEntries();
    cout <<"Total Event Number : "<< EventNum << endl;

    // getting pad geometry
    STDPadPlane* fPadPlane = new STDPadPlane();
    fPadPlane->Init();
    TH2Poly* padPlane = fPadPlane->GetPadPlanePoly();


    // data array to data-pad mapping
    double padData[12][64][4]; //layer, row, [x,y,w,tb]
    double ClusteredDataX[12]; // layer에서의 cluster된 위치데이터
    double ClusteredDataTb[12]; // layer에서의 cluster된 시간데이터
    double ClusteredDataADC[12]; // layer에서의 cluster된 ADC데이터

    // TGraph* fitLayerGraphX = new TGraph();
    TGraphErrors* fitLayerGraphX = new TGraphErrors();
    // TGraph* fitLayerGraphTB = new TGraph();
    TGraphErrors* fitLayerGraphTB = new TGraphErrors();
    TF1* fitX = new TF1("fitX","pol1",-10000,10000);
    TF1* fitTb = new TF1("fitTb","pol1",-10000,10000);

    TH2D* DataIndex_TotalADC = new TH2D("DataIndex_TotalADC",";Pad HitNum;Total ADC [a.u.]",100,50,350,100,0,260000);
    TH1D* TBDistribution = new TH1D("TBDistribution",";Time Bin;Counts",1024,-511.5,512.5);
    TH2D* TB_Y = new TH2D("TB_Y",";Y position [mm];Time Bin",12,-6,138,512,-0.5,512.5);
    TH1D* AngleDistribution = new TH1D("AngleDistribution",";Track Angle [Rad];Counts",120,0,1);
    TH1D* TrackLengthDistribution = new TH1D("TrackLengthDistribution",";Track Length [mm];Counts",250,110,210);

    TCanvas* c1 = new TCanvas("c1","c1",1200,800);

    int cuttedevent = 0;
    // int wantosee =9528;
    // cout.precision(4);
    for(int event = 4 ; event < EventNum; event++){
        
        fitLayerGraphX ->Set(0); // Reset the graph for each event
        fitLayerGraphTB->Set(0); // Reset the graph for each event
        fitX ->SetParameters(0,0);
        fitTb->SetParameters(0,0);
        memset(padData, 0, sizeof(padData));
        memset(ClusteredDataX, 0, sizeof(ClusteredDataX));
        memset(ClusteredDataTb, 0, sizeof(ClusteredDataTb));
        memset(ClusteredDataADC, 0, sizeof(ClusteredDataADC));
        padPlane->Clear();
        // if(event % 1000 == 0) cout << "Processing Event : " << event << endl;
        if(event % 132 == 0 ) cout << "[ " <<fixed << setprecision(2)<< (double)event/(double)EventNum*100.0 << " % ] \r " << flush;
        treein->GetEntry(event);
        int padHitted = hits->GetEntriesFast();
        if(!padHitted) continue;

        int RawWtotal = 0;
        int InteractedLayer=0;

        //data mapping
        for(int data = 0 ; data <padHitted; data++){
            LKHit* fhit= (LKHit*)hits->UncheckedAt(data);
            int layer = fhit->GetLayer();
            int row   = fhit->GetRow();

            double ADC = fhit->W();
            // if(W < 150 || W > 4000) continue;
            if(ADC < 150) continue;

            padData[layer][row][0] = fhit->x();
            padData[layer][row][1] = fhit->y();
            padData[layer][row][2] = ADC;
            padData[layer][row][3] = fhit->GetTb();
            padPlane->Fill(fhit->x(), fhit->y(), fhit->GetTb());
            // cout << "Layer : " << Layer << " , Row : " << row << " , X : " << X << " , Y : " << Y << " , W : " << W << " , Tb : " << tb << endl;
            RawWtotal += ADC;
        }


        //layer clustering 

        for(int layer =0; layer <12; layer++){
            double layerWtotal = 0;
            double ClusteredPositionX = 0;
            double ClusteredPositionTb = 0;

            for(int row = 0; row < 64; row++){
                ClusteredPositionX += padData[layer][row][0] * padData[layer][row][2];
                ClusteredPositionTb += padData[layer][row][3] * padData[layer][row][2];
                layerWtotal += padData[layer][row][2];
            }
            if(layerWtotal > 0){
                ClusteredPositionX /= layerWtotal;
                ClusteredPositionTb /= layerWtotal;
                InteractedLayer++;
            }

            // cout << "Layer : " << layer << " , ClusteredPositionX : " << ClusteredPositionX << " , ClusteredPositionTb : " << ClusteredPositionTb << " , LayerWtotal : " << layerWtotal << endl;
            ClusteredDataX[layer] = ClusteredPositionX;
            ClusteredDataTb[layer] = ClusteredPositionTb;
            ClusteredDataADC[layer] = layerWtotal;

        }
        if(InteractedLayer < 2) continue; // skip if less than 2 layers are interacted
        if(InteractedLayer < 5) continue; // skip if less than 11 layers are interacted

        //check track continuity
        bool isContinuous = true;
        if(ClusteredDataTb[0] == 0) isContinuous = false; // check first layer
        if(ClusteredDataX[0] < -10 || ClusteredDataX[0] > 10) isContinuous = false; // check first layer X position
        for(int layer = 0; layer < InteractedLayer-1; layer++){
            if(ClusteredDataTb[layer] == 0){
                isContinuous = false;
                break;
            }
        }

        
        if(!isContinuous) continue;


        //track fitting 
        for(int layer = 0 ; layer < InteractedLayer ; layer++){
            if(ClusteredDataX[layer] != 0){
                fitLayerGraphX->SetPoint(fitLayerGraphX->GetN(), layer*12, ClusteredDataX[layer]);
                fitLayerGraphX->SetPointError(fitLayerGraphX->GetN()-1, 0, 1/sqrt(ClusteredDataADC[layer])); // Error as sqrt of total weight
            }
            if(ClusteredDataTb[layer] != 0){
                // cout<< ClusteredDataTb[layer] << endl;
                fitLayerGraphTB->SetPoint(fitLayerGraphTB->GetN(),layer*12,ClusteredDataTb[layer] );
                // cout << layer*12 << " , " << ClusteredDataTb[layer] << "    " << ClusteredDataADC[layer] << endl  ;
                TB_Y->Fill(layer*12,ClusteredDataTb[layer],ClusteredDataADC[layer]);
                // cout<< 1/sqrt(ClusteredDataADC[layer]) << endl;
                fitLayerGraphTB->SetPointError(fitLayerGraphTB->GetN()-1,0,1/sqrt(ClusteredDataADC[layer])); // Error as sqrt of total weight
            }
        }

        fitLayerGraphX->Fit("fitX","Q");
        fitLayerGraphTB->Fit("fitTb","Q");
        // cout << fitLayerGraphTB->GetN() << endl;
        
        double tbDiff = (fitTb->Eval((InteractedLayer-1)*12)) - fitTb->Eval(0);
        double XDiff = (fitX->Eval((InteractedLayer-1)*12)) - fitX->Eval(0);
        TBDistribution->Fill(tbDiff);
        double angle = atan(TBtoZ(tbDiff)/((InteractedLayer-1)*12));

        // cout << "Event : " << event << " , Interacted Layer : " << InteractedLayer*12-12 << " , XDiff : " << XDiff << " , tbDiff : " << tbDiff << endl;
        // cout << fitTb->Eval((InteractedLayer-1)*12) << " - " << fitTb->Eval(0) << " = " << tbDiff << endl;
        double TrackLength = sqrt(pow((InteractedLayer-1)*12,2)+pow(TBtoZ(tbDiff),2)+pow(XDiff,2));

        TrackLengthDistribution->Fill(TrackLength);
        AngleDistribution->Fill(cos(angle));
        DataIndex_TotalADC->Fill(padHitted, RawWtotal);

        cuttedevent++;
    }

    TLatex* lat = new TLatex();
    c1->Clear();
    gStyle->SetOptStat(0);
    TrackLengthDistribution->SetLineColor(kBlue);
    TrackLengthDistribution->SetTitle("Track length distribution ;Track Length [mm];Counts");
    TrackLengthDistribution->Draw();
    lat->SetTextSize(0.04);
    lat->DrawLatexNDC(0.55,0.80,Form("%d Events",cuttedevent));
    lat->DrawLatexNDC(0.55,0.85,Form("Run # : 250818005"));
    lat->DrawLatexNDC(0.55,0.75,Form("Field cage voltage : 1850V"));
    lat->DrawLatexNDC(0.55,0.70,Form("1 tb = 1.05cm/us"));
    c1->Update();
    c1->SaveAs(Form("TrackLengthDistribution_250911.png"));

    c1->Clear();
    TB_Y->Draw("colz");
    gStyle->SetOptStat(0);
    c1->Update();
    c1->SaveAs(Form("TB_Y_250911.png"));

    c1->Clear();
    DataIndex_TotalADC->Draw("colz");
    gStyle->SetOptStat(0);
    c1->Update();
    c1->SaveAs(Form("DataIndex_TotalADC_250911.png"));

    c1->Clear();
    //TBDistribution->Rebin(8);
    TBDistribution->SetLineColor(kRed);
    TBDistribution->SetLineWidth(3);
    TBDistribution->SetTitle("#Deltatb distribution ;#Deltatb = tb_{end} - tb_{vertex} [a.u.];Counts");
    
    TBDistribution->Draw();
    lat->SetTextSize(0.04);
    lat->DrawLatexNDC(0.15,0.80,Form("%d Events",cuttedevent));
    lat->DrawLatexNDC(0.15,0.85,Form("Run # : 250818005"));
    lat->DrawLatexNDC(0.15,0.75,Form("Field cage voltage : 1850V"));
    lat->DrawLatexNDC(0.15,0.70,Form("1 tb = 40 ns"));

    gStyle->SetOptStat(0);
    c1->Update();
    c1->SaveAs(Form("TBDistribution_250911norebin.png"));

    TFile* fileout = new TFile("./FileEventViewer.root","recreate");
    fileout->cd();
    TB_Y->Write();
    TBDistribution->Write();
    AngleDistribution->Write();
    TrackLengthDistribution->Write();
    DataIndex_TotalADC->Write();
    fileout->Close();
    filein->Close();
    


    time_t end = time(NULL);
    double diff = difftime(end, start);
    cout << "Time taken: " <<fixed<<setprecision(0)<< diff << " seconds." << endl;
    cout << "Total Cut Event : " << cuttedevent << endl;
}














bool BranchCut(TTree* &tree){

    tree->SetBranchStatus("*",0);
    tree->SetBranchStatus("Hit",1);
    tree->SetBranchStatus("Hit.fX",1);
    tree->SetBranchStatus("Hit.fY",1);
    tree->SetBranchStatus("Hit.fZ",1);
    tree->SetBranchStatus("Hit.fW",1);
    tree->SetBranchStatus("Hit.fTb",1);
    tree->SetBranchStatus("Hit.fPadID",1);
    tree->SetBranchStatus("Hit.fSection",1);
    tree->SetBranchStatus("Hit.fLayer",1);
    tree->SetBranchStatus("Hit.fRow",1);
    tree->SetBranchStatus("Hit.fUniqueID",0);
    tree->SetBranchStatus("Hit.fBits",0);
    tree->SetBranchStatus("Hit.fDX",0);
    tree->SetBranchStatus("Hit.fDY",0);
    tree->SetBranchStatus("Hit.fHitID",0);
    tree->SetBranchStatus("Hit.fTrackID",0);
    tree->SetBranchStatus("Hit.fChannelID",0);
    tree->SetBranchStatus("Hit.fAlpha",0);
    tree->SetBranchStatus("Hit.fPedestal",0);
    return true;
}

double TBtoZ(double tb){
    double driftVelocity = 0.42 ;// mm/tb ( 1.05 cm/us )
    
    return tb * driftVelocity;
}
