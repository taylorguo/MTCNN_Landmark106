package com.example.test;

import android.graphics.Bitmap;

public class MTCNN {
    public native boolean ModelInit(String LandmarkModelPath);
    public native boolean SetThreadsNumber(int threadNumber);
    // imageWidth, imageHeight, imageChannel is the interface for function improvement in future.
    public native float[] Detect(Bitmap bitmap, int imageWidth , int imageHeight, int imageChannel);

    static {
        System.loadLibrary("landmark106");
    }

}
