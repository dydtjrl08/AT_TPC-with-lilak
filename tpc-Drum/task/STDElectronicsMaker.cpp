#include "STDElectronicsMaker.h"

ClassImp(STDElectronicsMaker);

STDElectronicsMaker::STDElectronicsMaker()
{
    fName = "STDElectronicsMaker";
}

bool STDElectronicsMaker::Init()
{
    fDetector = (TPCDrum *) fRun -> GetDetector();
    fPadPlane = (STDPadPlane*) fDetector -> GetDetectorPlane();

    fChannelArray = fRun -> GetBranchA("RawPad");
    fMCTagArray = fRun -> GetBranchA("MCTag");

    fDynamicRange = 1000.; // [fC]
    if(fPar -> CheckPar("TPCDrum/DynamicRange")){
        fDynamicRange = fPar -> GetParDouble("TPCDrum/DynamicRange");
    }
    fEChargeToADC = fElectronCharge/(fDynamicRange *1.0e-15)*fADCMaxAmp;

    fTuneManager = STDSimTuningManager::GetSimTuningManager();

    return true;
}

void STDElectronicsMaker::Exec(Option_t *option)
{
    const int chanNum = fChannelArray -> GetEntries();
    for(int chan=0; chan<chanNum; chan++){
        fChannel = (GETChannel*)fChannelArray -> At(chan);
        fMCTag = (LKMCTag*)fMCTagArray -> At(chan);

        auto mcWeightArr = fChannel -> GetWaveformY();

        int asadID = fChannel -> GetAsad();
        int agetID = fChannel -> GetAget();
        int chanID = fChannel -> GetChan();
        int padID = fChannel -> GetPadID();
        
        for(int tb=0; tb<512; tb++){
            int weight = mcWeightArr[tb];

            // truth track id
            int mcNum = fMCTag -> GetMCNum(tb);
            for(int id=0; id<mcNum; id++){
                int mcID = fMCTag -> GetMCID(id, tb);
                int mcPurity = fMCTag -> GetMCPurity(id, tb);
            }
            
            int adc = weight * fEChargeToADC;            
            if(adc > fADCMaxAmp){adc = fADCMaxAmp;}
            mcWeightArr[tb] = adc;
        }
    }
}

bool STDElectronicsMaker::EndOfRun()
{
    return true;
}
