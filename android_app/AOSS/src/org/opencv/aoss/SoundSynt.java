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

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

/*
    Support class that implements a basic sound synthesizer
*/
public class SoundSynt {
	private static final String TAG = "Sample::SoundSynth";
	
	private double freqOfTone;
	
	private final int duration    = 2;
	private final int sampleRate  = 10000;
    private final int numSamples  = duration * sampleRate;
    private final double sample[] = new double[numSamples];
    private final byte generatedSnd[] = new byte[2 * numSamples];
    
    AudioTrack track;

    
    // Constructor
    public SoundSynt() {
        // Allocate a new audio track
    	track = new AudioTrack( AudioManager.STREAM_MUSIC, 
                                sampleRate, 
    							AudioFormat.CHANNEL_CONFIGURATION_MONO, 
                                AudioFormat.ENCODING_PCM_16BIT, 
    							numSamples, 
                                AudioTrack.MODE_STREAM);
    }
    
    
    public void init() {
    }
    
    // Stop the output sound
    public void stopSound() {
    	if ( track.getPlayState() == 3 ) // 3 = code for "playing"
			track.stop();
    }

    // Update the output sound
    public void updateSound(double val) {
    	
        // If the track isn't already playing, then start it
    	if (track.getPlayState() != 3 )
    		track.play();
    	
        // Use a new tread as this can take a while
    	final double dist   = val;
    	final Thread thread = new Thread(new Runnable() {
    		public void run() {
                // Generate a new tone
    			genTone(dist);

                // Write the new tone on the track
    			playSound();
    		}
    	});
    	thread.start();	
    }

    // Generate tones using a distance as parameters for modulate the frequency
    void genTone(double dist){
        // Choose the max value between 100 and the distance
        //  100 is a lower limit for frequencies
    	freqOfTone = Math.max(100, dist);
    	Log.i(TAG, Double.toString(dist) + " >> " + Double.toString(freqOfTone));
    	    	
        // Generate new samples of tan function using that frequence
    	for (int i = 1; i < numSamples; ++i) {
    		sample[i] = Math.tan(2 * Math.PI * i / (sampleRate/freqOfTone));
    	}      

        // Convert to 16 bit pcm sound array
        // Assumes the sample buffer is normalised
        int idx = 0;
        for (final double dVal : sample) {
            // Scale to maximum amplitude
            final short val = (short) ((dVal * 32767));
            // In 16 bit wav PCM, first byte is the low order byte
            generatedSnd[idx++] = (byte) (val & 0x00ff);
            generatedSnd[idx++] = (byte) ((val & 0xff00) >>> 8);

        }
    }

    // Write the genereated sound into the track
    void playSound(){
    	track.write( generatedSnd, 0, generatedSnd.length );
    }
}