#include "STDMCViewer.h"
#include <iostream>
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
    

    LKParameterContainer* Par = fRun -> GetParameterContainer();

    Dir = Par -> CheckPar("Figure/BaseDir")
	       ? Par -> GetParString("Figure/BaseDir")
	       : "Figure/others";


    hPoly = fPadPlane -> GetPadPlanePoly();
    hPoly -> GetZaxis() -> SetRangeUser(0, 4000);

    hPoly -> SetMinimum(1e-5);

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
    // 1. 초기화
    hPoly -> ClearBinContents();
    for(int i=0; i<gTrack.size(); i++){
        if (gTrack[i]) gTrack[i] -> Set(0);
    }

    lk_info << "Total track at " << fRun -> GetCurrentEventID() <<"th events: " << gTrack.size() << endl;

    // =======================================================================
    // Part 1. Hit 처리 (안전장치 추가)
    // =======================================================================
    if (fHitArray) {
        int hitNum = fHitArray -> GetEntries();
        double sumADC = 0.;
        
        for(int hit=0; hit<hitNum; hit++){
            // UncheckedAt 대신 At 사용 (조금 느려도 안전함)
            fHit = (LKHit*)fHitArray -> At(hit); 
            
            // [체크] fHit가 비어있으면 건너뜀
            if (fHit == nullptr) continue; 

            double x = fHit -> X();
            double y = fHit -> Y();
            double w = fHit -> W();

            hPoly -> Fill(x, y, w);
            sumADC += w;
        }
        
        if(hitNum > 0){
            hHitNum -> Fill(hitNum);
            hSumADC -> Fill(sumADC);
        }
    }

    // 경계선 그리기 (이건 안전함)
    for(int i=0; i<768; i++){
        double x = fPadPlane -> GetX(i);
        double y = fPadPlane -> GetY(i);
        hPoly -> Fill(x, y, 0);
    }

    // =======================================================================
    // Part 2. Step 처리 (여기가 충돌 지점)
    // =======================================================================
    if (fStepArray) {
        int stepNum = fStepArray -> GetEntries();
        for(int i=0; i<stepNum; i++){
            // UncheckedAt 대신 At 사용
            fStep = (LKMCStep*)fStepArray -> At(i); 
            
            // [체크 1] 포인터가 살아있는지 확인
            if (fStep == nullptr) continue; 

            // [체크 2] Track ID 계산
            int trackId = fStep -> GetTrackID() - 1;

            // [체크 3] 벡터 범위 확인 (음수이거나, 사이즈보다 크면 패스)
            // gTrack.size()를 사용하여 10이라는 하드코딩 숫자 의존 제거
            if (trackId < 0 || trackId >= gTrack.size()) {
                continue; 
            }

            // [체크 4] gTrack 내부의 그래프 객체가 진짜 있는지 확인
            if (gTrack[trackId] == nullptr) continue;

            double x = fStep -> GetX();
            double y = fStep -> GetY();
            double z = fStep -> GetZ();

	 //   fDetector->GetCoordinateGeantToPad(x, y, z);
            // Detector 경계 체크
            if (fDetector && !fDetector->IsInBoundary(x, y, z)) {
                continue;
            }

            // 모든 관문을 통과했으므로 안전하게 포인트 추가
            gTrack[trackId] -> SetPoint(gTrack[trackId]->GetN(), x, y);
        }
    }

    // =======================================================================
    // Part 3. 그리기 및 저장
    // =======================================================================
    if (cEvent) {
        cEvent -> cd();
        cEvent -> Clear();
        if(hBoundary) hBoundary -> Draw("");
        if(hPoly) hPoly -> Draw("colz, same");
        
        for(int i=0; i<gTrack.size(); i++){
            // 그래프가 있고, 점이 하나라도 있을 때만 그리기
            if (gTrack[i] && gTrack[i]->GetN() > 0) {
                gTrack[i] -> Draw("same, l");
            }
        }
        cEvent -> Modified(); 
        cEvent -> Update();
        
        // 그림 저장 (폴더가 없으면 에러날 수 있으니 확인 필요)
        //cEvent -> SaveAs(Form("./figure_one_track/Direct_decay%i.pdf", int(fRun -> GetCurrentEventID()) ));
        cEvent -> SaveAs(Form("%s/%i.pdf", Dir.Data(),int(fRun -> GetCurrentEventID()) ));
    }

    // 진행 상황 출력
    std::cout << "Event " << fRun -> GetCurrentEventID() << " Processed." << std::endl;
    
    // 자동 진행을 위해 입력 대기 제거 (테스트용이면 주석 해제)
    /*
    std::cout << ">>> Next? (q to quit): ";
    if (std::cin.get() == 'q') fRun->SignalEndOfRun();
    */
}

bool STDMCViewer::EndOfRun()
{
    return true;
}
