#include "MyWorld.h"
#include "dart/math/Helpers.h"

using namespace Eigen;

MyWorld::MyWorld(int _numCells, double _timeStep, double _diffCoef, double _viscCoef, unsigned char* data) {
    mNumCells = _numCells;
    mTimeStep = _timeStep;
    mDiffusionCoef = _diffCoef;
    mViscosityCoef = _viscCoef;

    int size = (mNumCells + 2) * (mNumCells + 2);
    
    // Allocate memory for velocity and density fields
    mU = new double[size];
    mV = new double[size];
    mPreU = new double[size];
    mPreV = new double[size];
    mDensityRed = new double[size];
    mPreDensityRed = new double[size];
    mDensityGreen = new double[size];
    mPreDensityGreen = new double[size];
    mDensityBlue = new double[size];
    mPreDensityBlue = new double[size];
    mP = new double[size];
    mB = new double[size];
    
    for (int i = 0; i < size; i++) {
        mU[i] = mPreU[i] = 0.0;
        mV[i] = mPreV[i] = 0.0;
        mDensityRed[i] = mPreDensityRed[i] = 0.0;
        mDensityGreen[i] = mPreDensityGreen[i] = 0.0;
        mDensityBlue[i] = mPreDensityBlue[i] = 0.0;
    }

    for (int i = 1; i <= mNumCells; i++){
        for(int j = 1; j <= mNumCells; j++){
            mDensityRed[IX(i,j)] = data[((i-1)*mNumCells+(j-1))*3]/255.0;
            mDensityGreen[IX(i,j)] = data[((i-1)*mNumCells+(j-1))*3]/255.0;
            mDensityBlue[IX(i,j)] = data[((i-1)*mNumCells+(j-1))*3]/255.0;
        }
    }
}

MyWorld::~MyWorld() {
}

void MyWorld::simulate() {
    velocityStep(mU, mV, mPreU, mPreV);
    densityStep(mDensityRed, mPreDensityRed);
    densityStep(mDensityGreen, mPreDensityGreen);
    densityStep(mDensityBlue, mPreDensityBlue);
    externalForces();
}

void MyWorld::densityStep(double *_x, double *_x0) {
    SWAP(_x, _x0); // _x now points at mPreDensity
    diffuseDensity(_x, _x0); // Diffusion on _x which pointst at mPreDensity
    SWAP(_x, _x0); // _x now points at mDensity
    advectDensity(_x, _x0, mU, mV); // Advection on _x which points at mDensity
}


void MyWorld::velocityStep(double *_u, double *_v, double *_u0, double *_v0) {
    SWAP(_u, _u0); // _u now points at mPreU
    SWAP(_v, _v0); // _v now points at mPreV
    diffuseVelocity(_u, _v, _u0, _v0);
    project (_u, _v);
    SWAP(_u, _u0); // _u now points at mU
    SWAP(_v, _v0); // _u now points at mV
    advectVelocity(_u, _v, _u0, _v0);
    project(_u, _v);
}

void MyWorld::diffuseDensity(double *_x, double *_x0) {
    double a = mTimeStep * mDiffusionCoef * mNumCells * mNumCells;
    linearSolve(_x, _x0, a, 1 + 4 * a);
    setBoundary(_x);
}

void MyWorld::diffuseVelocity(double *_u, double *_v, double *_u0, double *_v0) {
    double a = mTimeStep * mViscosityCoef * mNumCells * mNumCells;
    linearSolve(_u, _u0, a, 1 + 4 * a);
    linearSolve(_v, _v0, a, 1 + 4 * a);
    setVelocityBoundary(_u, _v);
}

void MyWorld::advectDensity(double *_d, double *_d0, double *_u, double *_v) {
    double dt0 = mTimeStep * mNumCells;  // h / x 
    for (int i = 1; i <= mNumCells; i++) {
        for (int j = 1; j <= mNumCells; j++) {
            double x = i- dt0 * _u[IX(i,j)];  // dt0 * _u[IX(i,j)] computes how many cells can a particle travel in one time step 
            double y = j - dt0 * _v[IX(i,j)];
            if (x < 0.5) 
                x = 0.5f; 
            if (x > mNumCells + 0.5) 
                x = mNumCells + 0.5;
            int i0 = (int)x;
            int i1 = i0 + 1;
            if (y < 0.5) 
                y = 0.5;
            if (y > mNumCells + 0.5)
                y = mNumCells + 0.5;
            int j0 = (int)y;
            int j1 = j0 + 1;
            double s1 = x - i0;
            double s0 = 1 - s1;
            double t1 = y - j0;
            double t0 = 1 - t1;
            _d[IX(i,j)] = s0 * (t0 * _d0[IX(i0, j0)] + t1 * _d0[IX(i0,j1)])+ s1 * (t0 * _d0[IX(i1, j0)] + t1 * _d0[IX(i1,j1)]);
	}
    }
    setBoundary(_d);
}

void MyWorld::advectVelocity(double *_u, double *_v, double *_u0, double *_v0) {
    double dt0 = mTimeStep * mNumCells;  // h / x
    for (int i = 1; i <= mNumCells; i++) {
        for (int j = 1; j <= mNumCells; j++) {
            double x = i- dt0 * _u0[IX(i,j)];  // dt0 * _u0[IX(i,j)] computes how many cells can a particle travel in one time step
            double y = j - dt0 * _v0[IX(i,j)];
            if (x < 0.5)
                x = 0.5f;
            if (x > mNumCells + 0.5)
                x = mNumCells + 0.5;
            int i0 = (int)x;
            int i1 = i0 + 1;
            if (y < 0.5)
                y = 0.5;
            if (y > mNumCells + 0.5)
                y = mNumCells + 0.5;
            int j0 = (int)y;
            int j1 = j0 + 1;
            double s1 = x - i0;
            double s0 = 1 - s1;
            double t1 = y - j0;
            double t0 = 1 - t1;
            _u[IX(i,j)] = s0 * (t0 * _u0[IX(i0, j0)] + t1 * _u0[IX(i0,j1)])+ s1 * (t0 * _u0[IX(i1, j0)] + t1 * _u0[IX(i1,j1)]);
            _v[IX(i,j)] = s0 * (t0 * _v0[IX(i0, j0)] + t1 * _v0[IX(i0,j1)])+ s1 * (t0 * _v0[IX(i1, j0)] + t1 * _v0[IX(i1,j1)]);
        }
    }
    setVelocityBoundary(_u, _v);

}

void MyWorld::project(double *_u, double *_v) {

    double u1, v1;
    double u2, v2;

    for (int i = 1; i <= mNumCells; i++){
        for(int j = 1; j <= mNumCells; j++){
            mB[IX(i,j)] = -1 * 0.5 * (1.0/mNumCells) * (_u[IX(i+1,j)] - _u[IX(i-1,j)] + _v[IX(i,j+1)] - _v[IX(i,j-1)]);
        }
    }
    setBoundary(mB);
    
    for (int i = 1; i <= mNumCells; i++){
        for(int j = 1; j <= mNumCells; j++){
            mP[IX(i,j)] = 0;
        }
    }
    setBoundary(mP);

    linearSolve(mP, mB, 1, 4);
    setBoundary(mP);

    for(int i = 1; i <= mNumCells; i++){
        for(int j = 1; j <= mNumCells; j++){
            u1 = ((_u[IX(i,j)] + _u[IX(i-1,j)])/2.0) - ((mP[IX(i,j)] - mP[IX(i-1,j)])/(1.0/mNumCells));
            u2 = ((_u[IX(i+1,j)] + _u[IX(i,j)])/2.0) - ((mP[IX(i+1,j)] - mP[IX(i,j)])/(1.0/mNumCells));
            v1 = ((_v[IX(i,j)] + _v[IX(i,j-1)])/2.0) - ((mP[IX(i,j)] - mP[IX(i,j-1)])/(1.0/mNumCells));
            v2 = ((_v[IX(i,j+1)] + _v[IX(i,j)])/2.0) - ((mP[IX(i,j+1)] - mP[IX(i,j)])/(1.0/mNumCells));

            //std::cout << "u1 = " << u1 << ", u2 = "<< u2 << ", v1 = " << v1 << ", v2 = " << v2 << std::endl;
            _u[IX(i,j)] = (u1 + u2)/2; // Not sure if we could do this over here
            _v[IX(i,j)] = (v1 + v2)/2; // Not sure if we could do this over here
        }
    }
    setVelocityBoundary(_u, _v);
}

void MyWorld::externalForces() {
    int size = (mNumCells + 2) * (mNumCells + 2);
    for (int i = 0; i< size; i++) {
        mPreDensityRed[i] = 0;
        mPreDensityGreen[i] = 0;
        mPreDensityBlue[i] = 0;
        mPreU[i] = 0;
        mPreV[i] = 0;
    }
}

void MyWorld::linearSolve(double *_x, double *_x0, double _a, double _c) {
    for (int k = 0; k < 20; k++) {
        for (int i = 1; i <= mNumCells; i++) {
            for (int j = 1; j <= mNumCells; j++) {
                _x[IX(i, j)] = (_x0[IX(i, j)] + _a * (_x[IX(i-1, j)] + _x[IX(i+1, j)] + _x[IX(i, j-1)] + _x[IX(i, j+1)])) / _c;
            }
        }
    }
}

void MyWorld::setBoundary(double *_x) {
    for (int i = 1; i <= mNumCells; i++) {
        _x[IX(0 ,i)] = _x[IX(1,i)];
        _x[IX(mNumCells+1, i)] = _x[IX(mNumCells, i)];
        _x[IX(i, 0)] = _x[IX(i, 1)];
        _x[IX(i, mNumCells+1)] = _x[IX(i, mNumCells)];
 
    }
    _x[IX(0, 0)] = 0.5 * (_x[IX(1, 0)] + _x[IX(0, 1)]);
    _x[IX(0, mNumCells+1)] = 0.5 * (_x[IX(1, mNumCells+1)] + _x[IX(0, mNumCells)]);
    _x[IX(mNumCells+1, 0)] = 0.5 * (_x[IX(mNumCells, 0)] + _x[IX(mNumCells+1, 1)]);
    _x[IX(mNumCells+1, mNumCells+1)] = 0.5 * (_x[IX(mNumCells, mNumCells+1)] + _x[IX(mNumCells+1, mNumCells)]);
}

void MyWorld::setVelocityBoundary(double *_u, double *_v) {
    for (int i = 1; i <= mNumCells; i++) {
        _u[IX(0 ,i)] = -_u[IX(1,i)];
        _u[IX(mNumCells+1, i)] = -_u[IX(mNumCells, i)];
        _u[IX(i, 0)] = _u[IX(i, 1)];
        _u[IX(i, mNumCells+1)] = _u[IX(i, mNumCells)];

        _v[IX(0 ,i)] = _v[IX(1,i)];
        _v[IX(mNumCells+1, i)] = _v[IX(mNumCells, i)];
        _v[IX(i, 0)] = -_v[IX(i, 1)];
        _v[IX(i, mNumCells+1)] = -_v[IX(i, mNumCells)];
    }
    _u[IX(0, 0)] = 0.5 * (_u[IX(1, 0)] + _u[IX(0, 1)]);
    _u[IX(0, mNumCells+1)] = 0.5 * (_u[IX(1, mNumCells+1)] + _u[IX(0, mNumCells)]);
    _u[IX(mNumCells+1, 0)] = 0.5 * (_u[IX(mNumCells, 0)] + _u[IX(mNumCells+1, 1)]);
    _u[IX(mNumCells+1, mNumCells+1)] = 0.5 * (_u[IX(mNumCells, mNumCells+1)] + _u[IX(mNumCells+1, mNumCells)]);
    _v[IX(0, 0)] = 0.5 * (_v[IX(1, 0)] + _v[IX(0, 1)]);
    _v[IX(0, mNumCells+1)] = 0.5 * (_v[IX(1, mNumCells+1)] + _v[IX(0, mNumCells)]);
    _v[IX(mNumCells+1, 0)] = 0.5 * (_v[IX(mNumCells, 0)] + _v[IX(mNumCells+1, 1)]);
    _v[IX(mNumCells+1, mNumCells+1)] = 0.5 * (_v[IX(mNumCells, mNumCells+1)] + _v[IX(mNumCells+1, mNumCells)]);

}
