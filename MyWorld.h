#ifndef _MYWORLD_
#define _MYWORLD_

#include <vector>
#include <Eigen/Dense>
using namespace std;

#define IX(i, j) ((i)+(getNumCells()+2)*(j))
#define SWAP(x0,x) {double *tmp=x0;x0=x;x=tmp;}

class MyWorld {
 public:
    MyWorld(int _numCells, double _timeStep, double _diffCoef, double _viscCoef, unsigned char* data);

    virtual ~MyWorld();

    int getNumCells() { return mNumCells;}

    double getDensity(int _index, int color) 
    {
        if(color == 0)
            return mDensityRed[_index];
        else if(color == 1)
            return mDensityGreen[_index];
        else
            return mDensityBlue[_index];
    }

    double getVelocityU(int _index) { return mU[_index]; }
    double getVelocityV(int _index) { return mV[_index]; }
    void setDensity(int _i, int _j, double _source) {

        srand(time(NULL));
        int randomNumber;

        randomNumber = rand()%100 + 1;

        mDensityRed[IX(_i, _j)] += mTimeStep * (randomNumber);
        randomNumber = rand()%100 + 1;
        mDensityGreen[IX(_i, _j)] += mTimeStep * (randomNumber);
        randomNumber = rand()%100 + 1;
        mDensityBlue[IX(_i, _j)] += mTimeStep * (randomNumber);
    }
    void setU(int _i, int _j, double _force) { mU[IX(_i, _j)] += mTimeStep * _force; }
    void setV(int _i, int _j, double _force) { mV[IX(_i, _j)] += mTimeStep * _force; }
    
    void simulate();
    
 protected:
    void densityStep(double *_x, double *_x0);
    void velocityStep(double *_u, double *_v, double *_u0, double *_v0);
    void diffuseDensity(double *_x, double *_x0);
    void diffuseVelocity(double *_u, double *_v, double *_u0, double *_v0);
    void advectDensity(double *_d, double *_d0, double *_u, double *_v);
    void advectVelocity(double *_u, double *_v, double *_u0, double *_v0);
    void project(double *_u, double *_v);
    void externalForces();
    void linearSolve(double *_x, double *_x0, double _a, double _c);
    void setBoundary(double *_x);
    void setVelocityBoundary(double *_u, double *_v);

    int mNumCells;
    double mTimeStep;
    double mDiffusionCoef;
    double mViscosityCoef;
    double *mU;
    double *mV;
    double *mPreU;
    double *mPreV;
    double *mDensityRed;
    double *mDensityBlue;
    double *mDensityGreen;
    double *mPreDensityRed;
    double *mPreDensityBlue;
    double *mPreDensityGreen;
    
    double *mP;
    double *mB;
};

#endif
