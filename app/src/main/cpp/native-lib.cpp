#include <android/bitmap.h>
#include <android/log.h>

#include <jni.h>
#include <string>
//#include <vector>
#include "include/net.h"
#include "landmark106.h"

using namespace std;
#define TAG "mtcnnSo"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG ,__VA_ARGS__)

static MTCNN * mtcnn;
static MTCNNBox *mtcnnBox;
//sdk是否初始化成功
bool detection_sdk_init_ok = false;

extern "C"{

    JNIEXPORT jboolean JNICALL
    Java_com_example_test_MTCNN_ModelInit(JNIEnv *env, jobject instance, jstring LandmarkModelPath_){

        const char *LandmarkModelPath = env->GetStringUTFChars(LandmarkModelPath_, 0);
        string tFaceModelDir = LandmarkModelPath;
        string tLastChar = tFaceModelDir.substr(tFaceModelDir.length() - 1, 1);

        if ("\\" == tLastChar) {
            tFaceModelDir = tFaceModelDir.substr(0, tFaceModelDir.length() - 1) + "/";
        } else if (tLastChar != "/") {
            tFaceModelDir += "/";
        }
        LOGI(" ******** init, tFaceModelDir=%s", tFaceModelDir.c_str());

        mtcnn = new MTCNN(tFaceModelDir);
        env->ReleaseStringUTFChars(LandmarkModelPath_, LandmarkModelPath);

        return true;
    }


    JNIEXPORT jboolean JNICALL
    Java_com_example_test_MTCNN_SetThreadsNumber(JNIEnv *env, jobject instance, jint threadsNumber) {

        if(threadsNumber!=1&&threadsNumber!=2&&threadsNumber!=4&&threadsNumber!=8){
            LOGE("线程只能设置1，2，4，8");
            return false;
        }

        mtcnn->SetNumThreads(threadsNumber);
        LOGI(" ******** SetThreads=%d", threadsNumber);
        return  true;
    }


    // imageWidth, imageHeight, imageChannel is the interface for function improvement in future.
    JNIEXPORT jfloatArray JNICALL
    Java_com_example_test_MTCNN_Detect(JNIEnv *env, jobject instance, jobject bitmap, jint imageWidth, jint imageHeight, jint imageChannel){

        AndroidBitmapInfo info;
        AndroidBitmap_getInfo(env, bitmap, &info);
        int width = info.width;
        int height = info.height;

        if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
            return NULL;

        void *indata;
        AndroidBitmap_lockPixels(env, bitmap, &indata);

        ncnn::Mat in = ncnn::Mat::from_pixels_resize((const unsigned char*)indata, ncnn::Mat::PIXEL_BGR, width, height, 48, 48);
        LOGI(" ******** JNI 图像格式转换完成, Mat已读入, 大小已经更改为(96x96).");

        AndroidBitmap_unlockPixels(env, bitmap);

        //std::vector<mtcnn_point> outPoints;
        ncnn::Mat output;
        mtcnn->Detect(in, output);

        LOGI(" ******** JNI 人脸关键点检测完成, 转换结果 ...");
        float *landmarks = new float[106*2];

        for (int i=0; i<106; ++i){
            float x = abs(output[2*i]*width);
            float y = abs(output[2*i + 1]*height);
            //LOGI(" ******** JNI 检测结果: 第%d个点 *** x:%f, y:%f", i, x, y);
            landmarks[2*i] = x;
            landmarks[2*i+1] = y;
        }
        LOGI(" ******** JNI 检测完成: %d", sizeof(landmarks));

        jfloatArray jout = env->NewFloatArray(106*2);
        env->SetFloatArrayRegion(jout, 0, 106*2, landmarks);
        delete[] landmarks;

        return jout;
    }

    ///////////
    //////////
    JNIEXPORT jboolean JNICALL
    Java_com_example_test_MTCNNBox_FaceDetectionModelInit(JNIEnv *env, jobject instance,
                                                       jstring faceDetectionModelPath_) {
        LOGE("JNI*****************开始人脸检测模型初始化");
        //如果已初始化则直接返回
        if (detection_sdk_init_ok) {
            //  LOGD("人脸检测模型已经导入");
            return true;
        }
        jboolean tRet = false;
        if (NULL == faceDetectionModelPath_) {
            LOGE("导入的人脸检测的目录为空");
            return tRet;
        }

        //获取MTCNN模型的绝对路径的目录（不是/aaa/bbb.bin这样的路径，是/aaa/)
        const char *faceDetectionModelPath = env->GetStringUTFChars(faceDetectionModelPath_, 0);
        if (NULL == faceDetectionModelPath) {
            return tRet;
        }

        string tFaceModelDir = faceDetectionModelPath;
        string tLastChar = tFaceModelDir.substr(tFaceModelDir.length() - 1, 1);
        //LOGD("init, tFaceModelDir last =%s", tLastChar.c_str());
        //目录补齐/
        if ("\\" == tLastChar) {
            tFaceModelDir = tFaceModelDir.substr(0, tFaceModelDir.length() - 1) + "/";
        } else if (tLastChar != "/") {
            tFaceModelDir += "/";
        }
        LOGI("init, tFaceModelDir=%s", tFaceModelDir.c_str());

        //没判断是否正确导入，懒得改了
        mtcnnBox = new MTCNNBox(tFaceModelDir);
        mtcnnBox->SetMinFace(40);

        env->ReleaseStringUTFChars(faceDetectionModelPath_, faceDetectionModelPath);
        detection_sdk_init_ok = true;
        tRet = true;
        return tRet;
    }

    JNIEXPORT jintArray JNICALL
    Java_com_example_test_MTCNNBox_MaxFaceDetect(JNIEnv *env, jobject instance, jbyteArray imageDate_,
                                                 jint imageWidth, jint imageHeight, jint imageChannel) {
        LOGE("JNI******************开始检测人脸");
        if(!detection_sdk_init_ok){
            LOGE("人脸检测MTCNN模型SDK未初始化，直接返回空");
            return NULL;
        }

        int tImageDateLen = env->GetArrayLength(imageDate_);
        if(imageChannel == tImageDateLen / imageWidth / imageHeight){
            LOGE("数据宽=%d,高=%d,通道=%d",imageWidth,imageHeight,imageChannel);
        }
        else{
            LOGE("数据长宽高通道不匹配，直接返回空");
            return NULL;
        }

        jbyte *imageDate = env->GetByteArrayElements(imageDate_, NULL);
        if (NULL == imageDate){
            LOGE("导入数据为空，直接返回空");
            env->ReleaseByteArrayElements(imageDate_, imageDate, 0);
            return NULL;
        }

        if(imageWidth<20||imageHeight<20){
            LOGE("导入数据的宽和高小于20，直接返回空");
            env->ReleaseByteArrayElements(imageDate_, imageDate, 0);
            return NULL;
        }

        //TODO 通道需测试
        if(3 == imageChannel || 4 == imageChannel){
            //图像通道数只能是3或4；
        }else{
            LOGE("图像通道数只能是3或4，直接返回空");
            env->ReleaseByteArrayElements(imageDate_, imageDate, 0);
            return NULL;
        }

        //int32_t minFaceSize=40;
        //mtcnn->SetMinFace(minFaceSize);

        unsigned char *faceImageCharDate = (unsigned char*)imageDate;
        ncnn::Mat ncnn_img;
        if(imageChannel==3) {
            ncnn_img = ncnn::Mat::from_pixels(faceImageCharDate, ncnn::Mat::PIXEL_BGR2RGB,
                                              imageWidth, imageHeight);
        }else{
            ncnn_img = ncnn::Mat::from_pixels(faceImageCharDate, ncnn::Mat::PIXEL_RGBA2RGB, imageWidth, imageHeight);
        }

        std::vector<Bbox> finalBbox;
        mtcnnBox->detectMaxFace(ncnn_img, finalBbox);

        int32_t num_face = static_cast<int32_t>(finalBbox.size());
        LOGE("检测到的人脸数目：%d\n", num_face);

        int out_size = 1+num_face*14;
        //  LOGD("内部人脸检测完成,开始导出数据");
        int *faceInfo = new int[out_size];
        faceInfo[0] = num_face;
        for(int i=0;i<num_face;i++){
            faceInfo[14*i+1] = finalBbox[i].x1;//left
            faceInfo[14*i+2] = finalBbox[i].y1;//top
            faceInfo[14*i+3] = finalBbox[i].x2;//right
            faceInfo[14*i+4] = finalBbox[i].y2;//bottom
            for (int j =0;j<10;j++){
                faceInfo[14*i+5+j]=static_cast<int>(finalBbox[i].ppoint[j]);
            }
        }

        jintArray tFaceInfo = env->NewIntArray(out_size);
        env->SetIntArrayRegion(tFaceInfo,0,out_size,faceInfo);
        //  LOGD("内部人脸检测完成,导出数据成功");
        delete[] faceInfo;
        env->ReleaseByteArrayElements(imageDate_, imageDate, 0);
        return tFaceInfo;
    }

    JNIEXPORT jboolean JNICALL
    Java_com_example_test_MTCNNBox_FaceDetectionModelUnInit(JNIEnv *env, jobject instance) {
        if(!detection_sdk_init_ok){
            LOGE("人脸检测MTCNN模型已经释放过或者未初始化");
            return true;
        }
        jboolean tDetectionUnInit = false;
        delete mtcnn;


        detection_sdk_init_ok=false;
        tDetectionUnInit = true;
        LOGE("人脸检测初始化锁，重新置零");
        return tDetectionUnInit;

    }

    JNIEXPORT jboolean JNICALL
    Java_com_example_test_MTCNNBox_SetMinFaceSize(JNIEnv *env, jobject instance, jint minSize) {
        if(!detection_sdk_init_ok){
            LOGE("人脸检测MTCNN模型SDK未初始化，直接返回");
            return false;
        }

        if(minSize<=20){
            minSize=20;
        }

        mtcnnBox->SetMinFace(minSize);
        return true;
    }


    JNIEXPORT jboolean JNICALL
    Java_com_example_test_MTCNNBox_SetThreadsNumber(JNIEnv *env, jobject instance, jint threadsNumber) {
        if(!detection_sdk_init_ok){
            LOGE("人脸检测MTCNN模型SDK未初始化，直接返回");
            return false;
        }

        if(threadsNumber!=1&&threadsNumber!=2&&threadsNumber!=4&&threadsNumber!=8){
            LOGE("线程只能设置1，2，4，8");
            return false;
        }

        mtcnn->SetNumThreads(threadsNumber);
        return  true;
    }


    JNIEXPORT jboolean JNICALL
    Java_com_example_test_MTCNNBox_SetTimeCount(JNIEnv *env, jobject instance, jint timeCount) {

        if(!detection_sdk_init_ok){
            LOGE("人脸检测MTCNN模型SDK未初始化，直接返回");
            return false;
        }

        mtcnnBox->SetTimeCount(timeCount);
        return true;

    }

}

