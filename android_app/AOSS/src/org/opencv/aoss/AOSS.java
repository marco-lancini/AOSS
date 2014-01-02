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

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Window;

/*
    Main activity of AOSS
*/
public class AOSS extends Activity {
    private static final String TAG = "AOSS::Activity";

    public static final int VIEW_MODE_RGBA     = 0;
    public static final int VIEW_MODE_FEATURES = 1;

    public static int viewMode = VIEW_MODE_RGBA;
    private MenuItem  mItemPreviewRGBA;
    private MenuItem  mItemPreviewFeatures;
    private MenuItem  mItemAbout;

    // Constructor
    public AOSS() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    // Called when the activity is first created
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.i(TAG, "onCreate");
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(new AOSSView(this));
    }

    // Called when the menu is called
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.i(TAG, "onCreateOptionsMenu");
        mItemPreviewRGBA     = menu.add("Sound Synthesizer - OFF");
        mItemPreviewFeatures = menu.add("Sound Synthesizer - ON");
        mItemAbout           = menu.add("About");
        return true;
    }

    // Apply the function selected by the menu
    public boolean onOptionsItemSelected(MenuItem item) {
        Log.i(TAG, "Menu Item selected " + item);
        if (item == mItemPreviewRGBA)
            viewMode = VIEW_MODE_RGBA;
        else if (item == mItemPreviewFeatures)
            viewMode = VIEW_MODE_FEATURES;
        else if (item == mItemAbout) 
        {
            // Create the "About" dialog box and show it
        	AlertDialog about = new AlertDialog.Builder(AOSS.this)
            .setTitle("AOSS")
            .setMessage("Marco Lancini\nwww.marcolancini.it")
            .setPositiveButton("OK", new DialogInterface.OnClickListener() {
            	public void onClick(DialogInterface dialog, int whichButton) {}
            })
            .create();
        	
        	about.show();
        }
            
        return true;
    }
}
