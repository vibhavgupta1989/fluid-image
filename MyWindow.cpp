#include "MyWindow.h"
#include "dart/gui/GLFuncs.h"
#include <iostream>
#include <cstdio>


using namespace Eigen;

void MyWindow::displayTimer(int _val) {
        mWorld->simulate();
    glutPostRedisplay();
    mFrame++;
    if (mPlaying)
        glutTimerFunc(mDisplayTimeout, refreshTimer, _val);
}

void MyWindow::draw() {

    glViewport ( 0, 0, mWinWidth, mWinHeight);
    glMatrixMode ( GL_PROJECTION );
    glLoadIdentity ();
    gluOrtho2D ( 0.0, 1.0, 0.0, 1.0 );

    if (mViewVelocity)
        drawVelocity();
    else
        drawDensity();

    // Display the frame count in 2D text
    char buff[64];
    std::sprintf(buff,"%d",mFrame);
    std::string frame(buff);
    glDisable(GL_LIGHTING);
    glColor3f(0.0,0.0,0.0);
    dart::gui::drawStringOnScreen(0.02f,0.02f,frame);
    glEnable(GL_LIGHTING);
}

void MyWindow::keyboard(unsigned char key, int x, int y) {
    switch(key){
    case ' ': // Use space key to play or stop the motion
        mPlaying = !mPlaying;
        if(mPlaying)
            glutTimerFunc( mDisplayTimeout, refreshTimer, 0);
        break;
    case 'v':
        mViewVelocity = !mViewVelocity;
        break;
    case 'r':
        mColor = 1;
    default:
        Win3D::keyboard(key,x,y);
    }
    glutPostRedisplay();
}

void MyWindow::click(int button, int state, int x, int y) {
    mMouseDown = !mMouseDown;
        
    if(mMouseDown){
        mMouseX = x;
        mMouseY = y;
        int i = (int)((mMouseX / (double)mWinWidth) * mWorld->getNumCells() + 1);
        int j = (int)(((mWinHeight - mMouseY) / (double)mWinHeight) * mWorld->getNumCells() + 1);
	
        if (i < 1 || i > mWorld->getNumCells() || j < 1 || j > mWorld->getNumCells())
            return;
        
        if (button == GLUT_LEFT_BUTTON) {
            mLeftClick = true;
            mWorld->setDensity(i, j, 100.0);
        
        } else if (button == GLUT_RIGHT_BUTTON || button == GLUT_MIDDLE_BUTTON) {
            mRightClick = true;
            mWorld->setU(i, j, 5.0);
            mWorld->setV(i, j, 5.0);
        }
    } else {
        mLeftClick = false;
        mRightClick = false;
    }
    glutPostRedisplay();
}

void MyWindow::drag(int x, int y) {
    int i = (int)((x / (double)mWinWidth) * mWorld->getNumCells() + 1);
    int j = (int)(((mWinHeight - y) / (double)mWinHeight) * mWorld->getNumCells() + 1);
	
    if (i < 1 || i > mWorld->getNumCells() || j < 1 || j > mWorld->getNumCells())
        return;
        
    if (mLeftClick)
        mWorld->setDensity(i, j, 100.0);
    else if (mRightClick) {        
        mWorld->setU(i, j, x - mMouseX);
        mWorld->setV(i, j, mMouseY - y);
    }

    mMouseX = x;
    mMouseY = y;
    glutPostRedisplay();
}

void MyWindow::drawDensity() {
    double h = 1.0 / mWorld->getNumCells();
    glBegin(GL_QUADS);
    for (int i = 0; i <= mWorld->getNumCells(); i++) {
        double x = (i - 0.5) * h;
        for (int j = 0; j <= mWorld->getNumCells(); j++) {
            double y = (j - 0.5) * h;
                
            double d00r = mWorld->getDensity(IX(i, j), 0);
            double d00g = mWorld->getDensity(IX(i, j), 1);
            double d00b = mWorld->getDensity(IX(i, j), 2);

//            printf("d00r = %lf, d00g = %lf, d00b = %lf\n", d00r, d00g, d00b);

            double d01r = mWorld->getDensity(IX(i, j+1), 0);
            double d01g = mWorld->getDensity(IX(i, j+1), 1);
            double d01b = mWorld->getDensity(IX(i, j+1), 2);

            double d10r = mWorld->getDensity(IX(i+1, j), 0);
            double d10g = mWorld->getDensity(IX(i+1, j), 1);
            double d10b = mWorld->getDensity(IX(i+1, j), 2);
            
            double d11r = mWorld->getDensity(IX(i+1, j+1), 0);
            double d11g = mWorld->getDensity(IX(i+1, j+1), 1);
            double d11b = mWorld->getDensity(IX(i+1, j+1), 2);

//            if (d00 || d01 || d10 || d11){
  //              std::cout << "i = " << i << " j = " << j << " d00 = " << d00 << " d01 = " << d01 << " d10 = " << d10 << " d11 = " << d11 << std::endl;
    //        }           
            glColor3d(d00r, d00g, d00b);
            glVertex3f(x, y, 0);
            glColor3d(d10r, d10g, d10b); 
            glVertex3f(x + h, y, 0);
            glColor3d(d11r, d11g, d11b);
            glVertex3f(x + h, y + h, 0);
            glColor3d(d01r, d01g, d01b); 
            glVertex3f(x, y + h, 0);

            // Draw Red
/*
            d00 = mWorld->getDensity(IX(i, j), 1);
            d01 = mWorld->getDensity(IX(i, j+1), 1);
            d10 = mWorld->getDensity(IX(i+1, j), 1);
            d11 = mWorld->getDensity(IX(i+1, j+1), 1);

            glColor3d(d00, 0, 0);
            glVertex3f(x, y, 0);
            glColor3d(d10, 0, 0); 
            glVertex3f(x + h, y, 0);
            glColor3d(d11, 0, 0);
            glVertex3f(x + h, y + h, 0);
            glColor3d(d01, 0, 0); 
            glVertex3f(x, y + h, 0);
*/
            

        }
    }
    glEnd();
}

void MyWindow::drawVelocity() {
    double h = 1.0 / mWorld->getNumCells();

    glColor3f ( 1.0f, 1.0f, 1.0f );
    glLineWidth ( 1.0f );

    glBegin ( GL_LINES );
    for (int i=1 ; i <= mWorld->getNumCells(); i++) {
        double x = (i - 0.5) * h;
        for (int j = 1; j <= mWorld->getNumCells(); j++) {
            double y = (j - 0.5) * h;

            glVertex2f(x, y );
            glVertex2f (x + mWorld->getVelocityU(IX(i,j)), y + mWorld->getVelocityV(IX(i,j)));
        }
    }
    glEnd ();
}
