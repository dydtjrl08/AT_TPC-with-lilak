void test()
{
    TFile* file = new TFile("alphaTrkPar.root","read");
    TTree* tree = (TTree*)file -> Get("event");

    double trkPar[3];
    tree -> SetBranchAddress("trkPar", &trkPar);


    STDPadPlane* fPadPlane = new STDPadPlane();
    fPadPlane -> Init();

    TH2Poly* hPoly = fPadPlane -> GetPadPlanePoly();
    hPoly -> GetZaxis() -> SetRangeUser(1., 4000);


    double xWorld[4] = {-100., -100., 100., 100.};
    double yWorld[4] = {-120., 166., 166., -120.};
    TH2Poly* world = new TH2Poly();
    world -> SetStats(0);
    world -> SetTitle("; x [mm]; y [mm]");
    world -> AddBin(4, xWorld, yWorld);

    double x[4] = {-100., -100., 100., 100.};
    double y[4] = {-34., 166., 166., -34.};
    TH2Poly* hBoundary = new TH2Poly();
    hBoundary -> SetStats(0);
    hBoundary -> SetTitle("; x [mm]; y [mm]");
    hBoundary -> AddBin(4, x, y);

    for(int i=0; i<768; i++){
        double x = fPadPlane -> GetX(i);
        double y = fPadPlane -> GetY(i);
        hPoly -> Fill(x, y, 1.);
    }

    TCanvas* c1 = new TCanvas();
    c1 -> cd();
    world -> Draw();
    hBoundary -> Draw("same");
    hPoly -> Draw("same, colz");

    // vector<TF1*> trk;

    // TF1* trk = new TF1("trk", "pol1", 100, -100.);

    
    double boundY[2] = {-60., -75.};


    const int eventNum = tree -> GetEntries();
    
    int tmpIdx = 0;
    for(double y = boundY[0]; y>boundY[1]; y-=0.25){
        tmpIdx++;
    }
    
    const int testingY = tmpIdx;
    double trkXByY[testingY][eventNum];

    for(int event=0; event<eventNum; event++){
        tree -> GetEntry(event);

        double x0 = (-130. - trkPar[0])/trkPar[1];
        double x1 = (trkPar[2] - trkPar[0])/trkPar[1];

        double tmpX = 0.;
        double y0 = -130.;
        double y1 = trkPar[2];
        if(x0 > x1){
            tmpX = x0;
            x0 = x1;
            x1 = tmpX;
            y0 = trkPar[2];
            y1 = -130.;
        }

        int idx=0;
        for(double y = boundY[0]; y>boundY[1]; y-=0.25){
            double x = (y-trkPar[0])/trkPar[1];
            trkXByY[idx][event] = x;
            idx++;
        }

        TGraph* gTrk = new TGraph();
        gTrk -> SetPoint(0, x0, y0);
        gTrk -> SetPoint(1, x1, y1);
        gTrk -> SetLineColor(kRed);
        gTrk -> SetLineWidth(2);

        gTrk -> Draw("pl");
    }
    c1 -> Draw();

    double minSigma = 100.;
    double optY = 0.;
    for(int i=0; i<testingY; i++){
        double mean=0.;
        for(int trk=0; trk<eventNum; trk++){
            mean += trkXByY[i][trk];
        }
        mean /= double(eventNum);

        double sigma = 0.;
        for(int trk=0; trk<eventNum; trk++){
            sigma = (trkXByY[i][trk]-mean)*(trkXByY[i][trk]-mean);
        }
        sigma /= double(eventNum-1);

        sigma = sqrt(sigma);

        double y = boundY[0] - i*0.25;

        if(minSigma > sigma){
            minSigma = sigma;
            optY = y;
        }

        cout << sigma << " " << y << endl;
    }


    TH1D* alphaLength = new TH1D("length", "", 60, 40., 140.);

    for(int event=0; event<eventNum; event++){
        tree -> GetEntry(event);

        double y0 = optY;
        double y1 = trkPar[2];
        double x0 = (optY - trkPar[0])/trkPar[1];
        double x1 = (trkPar[2] - trkPar[0])/trkPar[1];

        
        if(y1 > 40){continue;}
        double r = sqrt((x0-x1)*(x0-x1) + (y0-y1)*(y0-y1));
        alphaLength -> Fill(r);
    }

    TCanvas* c2 = new TCanvas("c2","c2", 600, 600);
    alphaLength -> SetTitle("Alpha trk length; [mm]; Counts");
    alphaLength -> Draw();

    c2 -> Draw();



}