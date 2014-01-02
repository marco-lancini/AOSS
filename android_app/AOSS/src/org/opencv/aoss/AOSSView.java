////////////////////////////////////////////////////////////////////////////////
//
// AOSS - Marco Lancini (www.marcolancini.it)
//
//
// Copyright (C) 2012 Marco Lancini
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////
package org.opencv.aoss;

import org.opencv.android.Utils;
import org.opencv.core.Mat;
import org.opencv.core.CvType;
import org.opencv.imgproc.Imgproc;

import android.content.Context;
import android.graphics.Bitmap;
import android.view.SurfaceHolder;

/*
    View that manages the processing of the frame (calling JNI) and the creation of the output sound
*/
class AOSSView extends AOSSViewBase {
    private Mat mYuv;
    private Mat mRgba;
    private Mat mGraySubmat;

    private Mat skin;
    private Mat imgHSV;
    private Mat planeH;
    private Mat planeS;
    private Mat planeV;
    private Mat imgSkin;
    private Mat el1;
    private Mat el2;

    private double distance;
    private SoundSynt soundSynt;
    
    // Constructor + Instantiate the sound synthesizer
    public AOSSView(Context context) {
        super(context);
        soundSynt = new SoundSynt();
    }

    @Override
    public void surfaceChanged(SurfaceHolder _holder, int format, int width, int height) {
        super.surfaceChanged(_holder, format, width, height);

        // Initialize Mats before usage
        synchronized (this) {
            // Original frame in YUV    
            mYuv = new Mat(getFrameHeight() + getFrameHeight() / 2, getFrameWidth(), CvType.CV_8UC1);

            // Gray image from subsampling the original one
            mGraySubmat = mYuv.submat(0, getFrameHeight(), 0, getFrameWidth());

            // Matrices needed by the CV module
            mRgba   = new Mat();
            skin    = new Mat();
            imgHSV  = new Mat();
            planeH  = new Mat();
            planeS  = new Mat();
            planeV  = new Mat();
            imgSkin = new Mat();
            el1     = new Mat();
            el2     = new Mat();   
        }
    }

    @Override
    protected Bitmap processFrame(byte[] data) {
        // Put the new captured image into the basic Mat
        mYuv.put(0, 0, data);

        switch (AOSS.viewMode) {
        case AOSS.VIEW_MODE_RGBA:
            // If the user choose to stop AOSS functionalities,
            //      then simply display what the camera shoots
            //      and stop the output sound

            // Convert from YUV to RGB
            Imgproc.cvtColor(mYuv, mRgba, Imgproc.COLOR_YUV420sp2RGB, 4);

            // Stop the sound
            soundSynt.stopSound();
            break;
            
        case AOSS.VIEW_MODE_FEATURES:
            // If the user choose to start AOSS functionalities,
            //      then process each frame calling the JNI native interfaces
            //      and use the distance returned from JNI to update the 
            //      frequence of the sound synthesizer

            // Convert from YUV to RGB
            Imgproc.cvtColor(mYuv, mRgba, Imgproc.COLOR_YUV420sp2RGB, 4);

            // Call native JNI
            distance = analyzeFrame( 
                                    mGraySubmat.getNativeObjAddr(),
                                    mRgba.getNativeObjAddr(), 
                                    skin.getNativeObjAddr(),
                                    imgHSV.getNativeObjAddr(), 
                                    planeH.getNativeObjAddr(),
                                    planeS.getNativeObjAddr(), 
                                    planeV.getNativeObjAddr(), 
                                    imgSkin.getNativeObjAddr(), 
                                    el1.getNativeObjAddr(),
                                    el2.getNativeObjAddr() );
            
            // Use the new distance to update the sound
            soundSynt.updateSound(distance);
            
            break;
        }

        // Create the new image to be shown
        Bitmap bmp = Bitmap.createBitmap(getFrameWidth(), getFrameHeight(), Bitmap.Config.ARGB_8888);
        if (Utils.matToBitmap(mRgba, bmp))
            return bmp;

        bmp.recycle();
        return null;
    }

    @Override
    public void run() {
        super.run();

        // Explicitly deallocate Mats
        synchronized (this) {    
            if (mYuv != null) mYuv.release();
            if (mRgba != null) mRgba.release();
            if (mGraySubmat != null) mGraySubmat.release();
            if (skin != null) skin.release();
            if (imgHSV != null) imgHSV.release();
            if (planeH != null) planeH.release();
            if (planeS != null) planeS.release();
            if (planeV != null) planeV.release();
            if (imgSkin != null) imgSkin.release();
            if (el1 != null) el1.release();
            if (el2 != null) el2.release();
            
            mYuv        = null;
            mRgba       = null;
            mGraySubmat = null;
            skin        = null;
            imgHSV      = null;
            planeH      = null;
            planeS      = null;
            planeV      = null;
            imgSkin     = null;
            el1         = null;
            el2         = null;
        }
    }

    // Prototype of the native function
    public native double analyzeFrame( long mGraySubmat, long mRgba, 
    								 long skin, long imgHSV, 
    								 long planeH, long planeS, long planeV, long imgSkin, long el1, long el2 );
    
    // Load the native module
    static {
        System.loadLibrary("aoss_jni");
    }
}
