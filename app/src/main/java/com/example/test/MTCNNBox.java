package com.example.test;

public class MTCNNBox {
    //人脸检测模型导入
    public native boolean FaceDetectionModelInit(String faceDetectionModelPath);

    public native int[] MaxFaceDetect(byte[] imageDate, int imageWidth , int imageHeight, int imageChannel);

    //人脸检测模型反初始化
    public native boolean FaceDetectionModelUnInit();

    //检测的最小人脸设置
    public native boolean SetMinFaceSize(int minSize);

    //线程设置
    public native boolean SetThreadsNumber(int threadsNumber);

    //循环测试次数
    public native boolean SetTimeCount(int timeCount);

    static {
        System.loadLibrary("landmark106");
    }
}
