// =====================================================
// *           Sejong TPC-Drum Main Pld Plane          *
// *           Author: Seunghwan Lee                   *
// =====================================================

#ifndef STDPADPALNE_HH
#define STDPADPALNE_HH

#include "LKDetectorPlane.h"
#include "LKLogger.h"

#include <map>
#include <tuple>
#include <vector>
#include "TH2Poly.h"

#include "GETChannel.h"
#include "LKMCTag.h"

class STDPadPlane : public LKDetectorPlane
{
    public:
        STDPadPlane();
        virtual ~STDPadPlane() { ; }

        bool Init();
        void Clear(Option_t *option="");
        void Print(Option_t *option="") const;

        bool SetDataFromBranch(){return true;}
        void FillDataToHist(){;}

        bool IsInBoundary(Double_t x, Double_t y); // coordinate on the pad plane surface 
        
        Int_t FindPadID(Double_t x, Double_t y);

        Int_t GetSectionID(int PadID); // [section: 0=Type1LeftPad, 1=Type2MiddlePad, 2=Type1RightPad]
        Int_t GetSectionID(int layer, int row);

        Int_t GetLayerID(int padID);
        Int_t GetLayerID(int asad, int aget, int chan);

        Int_t GetRowID(int padID);
        Int_t GetRowID(int asad, int aget, int chan);

        Int_t GetPadID(int layer, int row);
        Int_t GetPadID(int asad, int aget, int chan);

        Int_t GetAsAdID(int padID);
        Int_t GetAgetID(int padID);
        Int_t GetChanID(int padID);

        Double_t GetX(int padID);
        Double_t GetX(int layer, int row);

        Double_t GetY(int padID);
        Double_t GetY(int layer, int row);

        Int_t GetFPNChannelID(int chan);

        Int_t GetAsAdNum();
        Int_t GetAGETNum();
        Int_t GetChanNum();

        Int_t GetPadNum();
        Int_t GetLayerNum();
        Int_t GetRowNum();

        Int_t GetType1PadNum();
        Int_t GetType1LeftPadNum();
        Int_t GetType1RightPadNum();
        Int_t GetType2PadNum();

        Double_t GetPadHeight();
        Double_t GetType1PadWidth();
        Double_t GetType2PadWidth();
        Double_t GetPadGap();

        TH2Poly* GetPadPlanePoly();

        LKMCTag* GetMCTag(int idx){return (LKMCTag*)fMCTagArray->At(idx);}


        TH2* GetHist(Option_t *option="");
        void DrawFrame(Option_t *option="");

        bool IsFPNChannel(int chan);

    private:
        void InitPadPlaneGeometry();
        void InitPadMapping();
        void InitChannelArray();
        // void InitPadMapping();

        const int fAsAdNum = 3;
        const int fAGETNum = 4;
        const int fChanNum = 68;

        double fType1PadWidth = 0.9; //[mm] left, right side type-1 pad width
        double fType2PadWidth = 1.9; //[mm] middle side type-2 pad width
        double fPadHeight = 11.9; //[mm] type-1 and type-2 pad height
        double fPadGap = 0.1; //[mm] type-1 and type-2 pad height

        int fLeftType1PadNum = 27; // number of left side type-1 pad 
        int fType2PadNum = 10; // number of middle side type-2 pad  
        int fRightType1PadNum = 27; // number of right side type-1 pad 

        static const int fPadNum = 768;
        static const int fLayerNum = 12; // number of layer
        static const int fRowNum = 64; // number of row

        double fPadPosArray[fLayerNum][fRowNum][2]; // [layer][row][x, y]
        map<pair<int, int>, pair<double, double>> fPadPosMap_lr; // pad position map using layer and row
        map<int, pair<double, double>> fPadPosMap_padIdx; // pad position map using pad index
        map<tuple<int, int, int>, pair<int, int>> fPadMap; // pad mapping array, [AsAd, AGET, Channel] to [layer, row]

        TH2Poly* fPadPlanePoly = nullptr;

        TObjArray* fMCTagArray = nullptr;

    ClassDef(STDPadPlane,1);
};

#endif
