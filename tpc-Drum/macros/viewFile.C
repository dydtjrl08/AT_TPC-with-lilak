void viewFile(){
    TFile *filein = new TFile("./FileEventViewer.root","read");
    TH2D* TD = (TH2D*)filein->Get("TBDistribution");
    TF1* fitgaus = new TF1("fitgaus","gaus",89,115);


    TCanvas *c1 = new TCanvas("c1","c1",1200,800);
    gStyle->SetOptStat(0);
    TD->SetLineColor(kBlue);
    TD->Rebin(4);
    TD->Draw("colz");
    TD->Fit("fitgaus","QR");
    c1->Update();

}