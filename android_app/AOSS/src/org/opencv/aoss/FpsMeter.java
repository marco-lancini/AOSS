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

import java.text.DecimalFormat;

import org.opencv.core.Core;

import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.util.Log;

/*
    Support class that evaluates the speed of the app in term of FramePerSecond (FPS)
*/
public class FpsMeter {
    private static final String TAG = "AOSS::FpsMeter";
    int            step;
    int            framesCouner;
    double         freq;
    long           prevFrameTime;
    String         strfps;
    DecimalFormat  twoPlaces = new DecimalFormat("0.00");
    Paint          paint;

    // Initialize the counter
    public void init() {
        strfps        = "";
        step          = 20;
        framesCouner  = 0;
        freq          = Core.getTickFrequency();
        prevFrameTime = Core.getTickCount();

        paint = new Paint();
        paint.setColor(Color.BLUE);
        paint.setTextSize(50);
    }

    // Measure the new FPS rate
    public void measure() {
        framesCouner++;
        if (framesCouner % step == 0) {
            long time     = Core.getTickCount();
            double fps    = step * freq / (time - prevFrameTime);
            prevFrameTime = time;
            DecimalFormat twoPlaces = new DecimalFormat("0.00");
            strfps = twoPlaces.format(fps) + " FPS";
            Log.i(TAG, strfps);
        }
    }

    // Draw the FPS rate on screen
    public void draw(Canvas canvas, float offsetx, float offsety) {
        canvas.drawText(strfps, 20 + offsetx, 10 + 50 + offsety, paint);
    }

}
