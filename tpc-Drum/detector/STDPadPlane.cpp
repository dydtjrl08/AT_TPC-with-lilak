#include "STDPadPlane.h"

ClassImp(STDPadPlane);

STDPadPlane::STDPadPlane()
{
    fName = "STDPadPlane";
    if (fChannelArray==nullptr)
        fChannelArray = new TObjArray();
}

bool STDPadPlane::Init()
{
    lk_info << "Initializing STDPadPlane" << std::endl;

    InitPadPlaneGeometry();
    InitPadMapping();
    InitChannelArray();
    
    return true;
}

void STDPadPlane::Clear(Option_t *option)
{
    LKDetectorPlane::Clear(option);
}

void STDPadPlane::Print(Option_t *option) const
{
    lk_info << "STDPadPlane" << std::endl;
}

bool STDPadPlane::IsInBoundary(Double_t x, Double_t y)
{
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
    int padID = fPadPlanePoly -> FindBin(x, y);
    if(padID < 1){return -1;}
    return padID-1;
}

Int_t STDPadPlane::GetSectionID(int padID)
{
    Int_t row = GetRowID(padID);
    if(row < 0 || fRowNum <= row){return -1;}
    if(row < 27){return 0;}
    if(27 <= row && row < (27+10)){return 1;}
    if((27+10) <= row){return 2;}
    return -1;
}

Int_t STDPadPlane::GetSectionID(int layer, int row)
{
    int padId = GetPadID(layer, row);
    return GetSectionID(padId);
}

Int_t STDPadPlane::GetLayerID(int padID){return int(padID/64);}
Int_t STDPadPlane::GetLayerID(int asad, int aget, int chan){return fPadMap.find(make_tuple(asad, aget, chan))->second.first;}
Int_t STDPadPlane::GetRowID(int padID){return int(padID%64);}
Int_t STDPadPlane::GetRowID(int asad, int aget, int chan){return fPadMap.find(make_tuple(asad, aget, chan))->second.second;}
Int_t STDPadPlane::GetPadID(int layer, int row)
{
    if(layer < 0 || fLayerNum <= layer){return -1;}
    if(row < 0 || fRowNum <= row){return -1;}
    return layer * fRowNum + row;
}
Int_t STDPadPlane::GetPadID(int asad, int aget, int chan)
{
    int layer = GetLayerID(asad, aget, chan);
    int row = GetRowID(asad, aget, chan);
    return GetPadID(layer, row);
}

Int_t STDPadPlane::GetAsAdID(int padID)
{
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
    if(0 <= chan && chan < 17) {return 11;} 
    else if(chan < 34){return 22;} 
    else if(chan < 51){return 45;} 
    else if(chan < 68){return 56;}
    return -1;
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

void STDPadPlane::InitPadMapping()
{
    //mapping
    int connecterIdx = 12;
    for(int asad=0; asad<fAsAdNum; asad++){
        for(int aget=0; aget<fAGETNum; aget++){
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
            connecterIdx--;
        }
    }
}

void STDPadPlane::InitChannelArray()
{
    fChannelArray = new TObjArray();
    fMCTagArray = new TObjArray();

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
    if(chan == 11 || chan == 22 || chan == 45 || chan == 56){return true;}
    return false;
}