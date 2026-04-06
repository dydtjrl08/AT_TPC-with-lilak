#include "STDPadPlane.h"
#define LOG_TRACE std::cout << ">>> [" << __PRETTY_FUNCTION__ << "] line " << __LINE__ << std::endl
#define debug_cout std::cout << "[" << __func__ << ":" << __LINE__ << "] "

#include <iostream>
ClassImp(STDPadPlane);

STDPadPlane::STDPadPlane()
{
    //LOG_TRACE;
    fName = "STDPadPlane";
    if (fChannelArray==nullptr)
        fChannelArray = new TObjArray();
}

bool STDPadPlane::Init()
{
    lk_info << "Initializing STDPadPlane" << std::endl;
    //LOG_TRACE;
    InitPadPlaneGeometry();
    InitPadMapping();
    InitChannelArray();
    
    return true;
}

void STDPadPlane::Clear(Option_t *option)
{
        //LOG_TRACE;
	LKDetectorPlane::Clear(option);
}

void STDPadPlane::Print(Option_t *option) const
{
        //LOG_TRACE;
	lk_info << "STDPadPlane" << std::endl;
}

bool STDPadPlane::IsInBoundary(Double_t x, Double_t y)
{
    //LOG_TRACE;  
    double extraGap = 2.; // [mm]
    double activePadPlaneWidth = GetType1PadNum()*(GetType1PadWidth()+GetPadGap()) + GetType2PadNum()*(GetType2PadWidth()+GetPadGap()) + extraGap;
    double activePadPlaneUpperBoundary = double(GetLayerNum()-1)*(GetPadHeight()+GetPadGap())+ (GetPadHeight()+GetPadGap())/2. + extraGap;
    double activePadPlaneLowerBoundary = (GetPadHeight()+GetPadGap())/2. + extraGap;

    if(-activePadPlaneWidth/2. <= x && x <= activePadPlaneWidth/2.){
        if(-activePadPlaneLowerBoundary <= y && y <= activePadPlaneUpperBoundary){
            return true;
        }
    }

    return false;
}

Int_t STDPadPlane::FindPadID(Double_t x, Double_t y)
{
    //LOG_TRACE;
    int padID = fPadPlanePoly -> FindBin(x, y);
    if(padID < 1){return -1;}
    return padID-1;
}

Int_t STDPadPlane::GetSectionID(int padID)
{
    //LOG_TRACE;
    Int_t row = GetRowID(padID);
    if(row < 0 || fRowNum <= row){return -1;}
    if(row < 27){return 0;}
    if(27 <= row && row < (27+10)){return 1;}
    if((27+10) <= row){return 2;}
    return -1;
}

Int_t STDPadPlane::GetSectionID(int layer, int row)
{
    //LOG_TRACE;
    int padId = GetPadID(layer, row);
    return GetSectionID(padId);
}

Int_t STDPadPlane::GetLayerID(int padID){return int(padID/64);}
Int_t STDPadPlane::GetLayerID(int asad, int aget, int chan){return fPadMap.find(make_tuple(asad, aget, chan))->second.first;}
Int_t STDPadPlane::GetRowID(int padID){return int(padID%64);}
Int_t STDPadPlane::GetRowID(int asad, int aget, int chan){return fPadMap.find(make_tuple(asad, aget, chan))->second.second;}
Int_t STDPadPlane::GetPadID(int layer, int row)
{
    //LOG_TRACE;
    if(layer < 0 || fLayerNum <= layer){return -1;}
    if(row < 0 || fRowNum <= row){return -1;}
    return layer * fRowNum + row;
}
Int_t STDPadPlane::GetPadID(int asad, int aget, int chan)
{
    //LOG_TRACE;
    int layer = GetLayerID(asad, aget, chan);
    int row = GetRowID(asad, aget, chan);
    return GetPadID(layer, row);
}

Int_t STDPadPlane::GetAsAdID(int padID)
{
    //LOG_TRACE;
    int layer = GetLayerID(padID);
    int row = GetRowID(padID);
    for(auto it = fPadMap.begin(); it != fPadMap.end(); ++it){
        if((it->second).first == layer && (it->second).second == row){
            return get<0>(it->first);
        }
    }
    return -1;
}

Int_t STDPadPlane::GetAgetID(int padID)
{
    //LOG_TRACE;
    int layer = GetLayerID(padID);
    int row = GetRowID(padID);
    for(auto it = fPadMap.begin(); it != fPadMap.end(); ++it){
        if((it->second).first == layer && (it->second).second == row){
            return get<1>(it->first);
        }
    }
    return -1;
}

Int_t STDPadPlane::GetChanID(int padID)
{
    //LOG_TRACE;
    int layer = GetLayerID(padID);
    int row = GetRowID(padID);
    for(auto it = fPadMap.begin(); it != fPadMap.end(); ++it){
        if((it->second).first == layer && (it->second).second == row){
            return get<2>(it->first);
        }
    }
    return -1;
}

Double_t STDPadPlane::GetX(int layer, int row){return fPadPosMap_lr.find(make_pair(layer, row))->second.first;}
Double_t STDPadPlane::GetX(int padID){return fPadPosMap_padIdx.find(padID)->second.first;}
Double_t STDPadPlane::GetY(int layer, int row){return fPadPosMap_lr.find(make_pair(layer, row))->second.second;}
Double_t STDPadPlane::GetY(int padID){return fPadPosMap_padIdx.find(padID)->second.second;}

Int_t STDPadPlane::GetFPNChannelID(int chan)
{
    //LOG_TRACE;
    
    Int_t fpnID = -1;

    if(0 <= chan && chan < 17) {fpnID = 11;} 
    else if(chan < 34){fpnID = 22;} 
    else if(chan < 51){fpnID = 45;} 
    else if(chan < 68){fpnID = 56;}
    
    std::cout << "Ch No. " << chan << " , FPN Channel " << fpnID << std::endl;
    
    return fpnID;
}

Int_t STDPadPlane::GetAsAdNum(){return fAsAdNum;}
Int_t STDPadPlane::GetAGETNum(){return fAGETNum;}
Int_t STDPadPlane::GetChanNum(){return fChanNum;}

Int_t STDPadPlane::GetPadNum(){return fPadNum;}
Int_t STDPadPlane::GetLayerNum(){return fLayerNum;}
Int_t STDPadPlane::GetRowNum(){return fRowNum;}

Int_t STDPadPlane::GetType1PadNum(){return GetType1LeftPadNum()+GetType1RightPadNum();}
Int_t STDPadPlane::GetType1LeftPadNum(){return fLeftType1PadNum;}
Int_t STDPadPlane::GetType1RightPadNum(){return fRightType1PadNum;}
Int_t STDPadPlane::GetType2PadNum(){return fType2PadNum;}

Double_t STDPadPlane::GetPadHeight(){return fPadHeight;}
Double_t STDPadPlane::GetType1PadWidth(){return fType1PadWidth;}
Double_t STDPadPlane::GetType2PadWidth(){return fType2PadWidth;}
Double_t STDPadPlane::GetPadGap(){return fPadGap;}

TH2Poly* STDPadPlane::GetPadPlanePoly(){return fPadPlanePoly;}

TH2* STDPadPlane::GetHist(Option_t *option)
{
    //LOG_TRACE;
    return (TH2D *) nullptr;
}

void STDPadPlane::DrawFrame(Option_t *option)
{
}

void STDPadPlane::InitPadPlaneGeometry()
{
    fPadPlanePoly = new TH2Poly();
    fPadPlanePoly -> SetStats(0);
    fPadPlanePoly -> SetTitle(";x [mm]; y [mm]");
    //LOG_TRACE;
    double boundaryX[5];
    double boundaryY[5];

    // correct value of pad plane origin X position, origin position X is center of pad plane
    double leftType1PadSizeX = double(fLeftType1PadNum) * fType1PadWidth + double(fLeftType1PadNum - 1) * fPadGap - fType1PadWidth/2.;
    double type2PadHalfSizeX = double(fType2PadNum / 2) * fType2PadWidth + double(fType2PadNum/2 - 1) * fPadGap;
    double addtionalPadGap = fPadGap * 1.5;
    double padPosXOffset = leftType1PadSizeX + type2PadHalfSizeX + addtionalPadGap;

    int padIdx = 0; // pad index will be increasing as a layer and row number
    for(int l=0; l<fLayerNum; l++){
        double padPosY = double(l) * (fPadHeight + fPadGap);
        double padPosX = -1. * padPosXOffset;
        double padWidth = 0.;

        for(int r=0; r<fRowNum; r++){
            // for Type1 and Type2 width 
            if(fLeftType1PadNum <= r && r < (fLeftType1PadNum + fType2PadNum)){padWidth = fType2PadWidth;}
            else{padWidth = fType1PadWidth;}

            fPadPosMap_lr.insert({make_pair(l, r), make_pair(padPosX, padPosY)});
            fPadPosMap_padIdx.insert({padIdx, make_pair(padPosX, padPosY)});

            // for PadPlane Poly boundary
            for(int i=0; i<5; i++){
                double xSign = (i<2 || i==4)? -1. : +1.;
                double ySign = (i==1 || i==2)? -1. : +1.;
                boundaryX[i] = padPosX + xSign * padWidth/2.;
                boundaryY[i] = padPosY + ySign * (fPadHeight + fPadGap)/2.;
            }
            fPadPlanePoly -> AddBin(5, boundaryX, boundaryY);

            // for crossing index of type1 and type2
            if(r == fLeftType1PadNum-1 || r == (fLeftType1PadNum + fType2PadNum -1)){padPosX += ((fType1PadWidth/2.) + (fType2PadWidth/2.) + fPadGap);}
            else{padPosX += (padWidth + fPadGap);}

            padIdx++;
        }
    }
}
// omic 4
// junction 16
// aget 하나에 fpn  4개 포함하면 68개
// 48개는 아무것도 없는 채널 단, 노이즈는 존재.
// fpn은 보드 자체에 있음. 
// si 디텍터 하나 당 fpn 채널 1개 
void STDPadPlane::InitPadMapping()
{
    //LOG_TRACE;
    //mapping
    int connecterIdx = 12;
    
    // Si 디텍터가 연결된 AsAd, AGET 인덱스 알아야 한다
    const int si_asad = 0;
    const int ohm_aget = 0;
    const int junc_aget = 3;
    
    const int LAYER_SI_OHMIC = 100;
    const int LAYER_SI_JUNCTION = 101;

    for(int asad=0; asad<fAsAdNum; asad++){
        for(int aget=0; aget<fAGETNum; aget++){
            
            // 1. Si detector 매핑 전용 (Ohmic)
            if(asad == si_asad && aget == ohm_aget){
                for (int chan = 0; chan < fChanNum; chan++){
                    if(IsFPNChannel(chan)) continue;

                    if (chan < 4) {
                        int layer = LAYER_SI_OHMIC;
                        int row = chan; 
                        fPadMap.insert({make_tuple(asad,aget,chan), make_pair(layer,row)});

                        std::cout << "Mapped Si Ohmic: AsAd" << asad << " AGet" << aget
                              << " Ch" << chan << " -> L" << layer << " R" << row << std::endl;
                    }
                    // 나머지 채널은 매핑 안 함
                }
            }
            
            // 2. Si Detector - Junction Side 매핑 (AsAd 0, AGet 3)
            else if (asad == si_asad && aget == junc_aget) {
                for (int chan = 0; chan < fChanNum; chan++){
                    if(IsFPNChannel(chan)) continue;
                
                    if (chan < 16) {
                        int layer = LAYER_SI_JUNCTION;
                        int row = chan; // 0 ~ 15
                        fPadMap.insert({make_tuple(asad, aget,chan), make_pair(layer,row)});

                        std::cout << "Mapped Si Junction: AsAd" << asad << " AGet" << aget 
                              << " Ch" << chan << " -> L" << layer << " R" << row << std::endl;
                    }
                }
            }

            // 3. 일반 TPC 매핑 (위의 두 경우가 아닐 때 실행)
            else {
                int tmpBaseRowIdx = (connecterIdx%4 == 0)? 0 : (4 - connecterIdx%4)* 64/4;
                int tmpBaseLayerIdx = 4 * abs(ceil((connecterIdx-1)/4) - 2);
                int pinIdx = 0;
                
                for(int chan=0; chan<fChanNum; chan++){
                    if(IsFPNChannel(chan)){continue;}
                    pinIdx++;

                    int row = tmpBaseRowIdx + (16-1) - (pinIdx - (pinIdx%4)) / 4;
                    if(pinIdx%4 == 0){row += 1;}

                    int layer = -1;
                    if(pinIdx%4 == 0){layer = tmpBaseLayerIdx + 2;}
                    else if(pinIdx%4 == 1){layer = tmpBaseLayerIdx;}
                    else if(pinIdx%4 == 2){layer = tmpBaseLayerIdx + 3;}
                    else if(pinIdx%4 == 3){layer = tmpBaseLayerIdx + 1;}

                    fPadMap.insert({make_tuple(asad, aget, chan), make_pair(layer, row)});
                }
            }

            // [중요] 여기가 핵심입니다!
            // Si든 TPC든 루프가 끝나면 무조건 인덱스를 하나 줄여야 다음 위치가 맞습니다.
            connecterIdx--; 
        }
    }
}




void STDPadPlane::InitChannelArray()
{
    LOG_TRACE;
    fChannelArray = new TObjArray();
    fMCTagArray = new TObjArray();
  
    debug_cout << "Pad Number : " << GetPadNum() << std::endl;



    for(int i=0; i<GetPadNum(); i++){
        GETChannel* channel = new GETChannel();
        channel -> SetPadID(i);
        channel -> SetAsad(GetAsAdID(i));
        channel -> SetAget(GetAgetID(i));
        channel -> SetChan(GetChanID(i));
        fChannelArray -> Add(channel);

        LKMCTag* mcTag = new LKMCTag();
        fMCTagArray -> Add(mcTag);
    }
}

bool STDPadPlane::IsFPNChannel(int chan)
{
    LOG_TRACE;
    std::cout << "IsFPNChannel ? ch No. " << chan << std::endl;

    if(chan == 11 || chan == 22 || chan == 45 || chan == 56){return true;}
    return false;
}
