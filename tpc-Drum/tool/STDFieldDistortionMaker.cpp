#include "STDFieldDistortionMaker.h"

STDFieldDistortionMaker::STDFieldDistortionMaker() : fInitEFieldFlag(false), fInitBFieldFlag(false)
{
}

bool STDFieldDistortionMaker::Init()
{
    cout << "STDFieldDistortionMaker::Init()" << endl;
    
    fRun = LKRun::GetRun();
    fPar = fRun -> GetParameterContainer();

    bool onFieldDistortion = false;
    if(fPar->CheckPar("TPCDrum/FieldDistortion")){
        onFieldDistortion = fPar->GetParBool("TPCDrum/FieldDistortion");
    }
    if(onFieldDistortion){
        InitEFieldMap();
        InitBFieldMap();
    }

    return true;
}

double STDFieldDistortionMaker::GetEField(double x, double y, double z, int direction)
{
    double fieldValue = 0.;  // [V/cm]

    if(!fInitEFieldFlag){
        if(direction == dirX){fieldValue = 0;}
        else if(direction == dirY){fieldValue = 0;}
        else if(direction == dirZ){fieldValue = 250.;}
        else{fieldValue = -1;} // wrong direction !!
    }
    else{
        // To be updated the E-field map getting function
    }

    return fieldValue;
}

double STDFieldDistortionMaker::GetEFieldMag(double x, double y, double z)
{
    double ex = GetEField(x, y, z, dirX);
    double ey = GetEField(x, y, z, dirY);
    double ez = GetEField(x, y, z, dirZ);
    double e = sqrt(ex*ex + ey*ey + ez*ez);
    return e;
}

double STDFieldDistortionMaker::GetBField(double x, double y, double z, int direction)
{
    double fieldValue = 0.;  // [T]

    if(!fInitBFieldFlag){
        if(direction == dirX){fieldValue = 0;}
        else if(direction == dirY){fieldValue = 0;}
        else if(direction == dirZ){fieldValue = 1.;}
        else{fieldValue = -999;} // wrong direction !!
    }
    else{
        // To be updated the B-field map getting function
    }

    return fieldValue;
}

double STDFieldDistortionMaker::GetBFieldMag(double x, double y, double z)
{
    double bx = GetEField(x, y, z, dirX);
    double by = GetEField(x, y, z, dirY);
    double bz = GetEField(x, y, z, dirZ);
    double b = sqrt(bx*bx + by*by + bz*bz);
    return b;
}

bool STDFieldDistortionMaker::GetElectronDirection(double x, double y, double z, double& dx, double& dy, double& dz)
{
    double ex = fabs(GetEField(x, y, z, dirX));
    double ey = fabs(GetEField(x, y, z, dirY));
    double ez = fabs(GetEField(x, y, z, dirZ));
    double e  = GetEFieldMag(x, y, z);
    if(e <= 0.1){return false;}

    dx = ex/e;
    dy = ey/e;
    dz = -1.*ez/e;
    return true;
}

void STDFieldDistortionMaker::InitEFieldMap()
{
    // To be updated 
    fInitEFieldFlag = true;
}

void STDFieldDistortionMaker::InitBFieldMap()
{
    // To be updated 
    fInitBFieldFlag = true;
}