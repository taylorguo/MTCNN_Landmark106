//
// Created by Taylor Guo on 2019/6/28.
//

#pragma once
#ifndef LANDMARK106_H
#define LANDMARK106_H

#include "net.h"
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <time.h>
#include <map>

using namespace std;

//struct mtcnn_point
//{
//    float x;
//    float y;
//};

class MTCNN{
public:
    MTCNN(const string &model_path);
    MTCNN(const std::vector<std::string> param_files, const std::vector<std::string> bin_files);
    ~MTCNN();
    // Can set thread numbers for OpenMP.
    // If mobile getting hotter and CPU frequency reduction, set single thread.
    // Or just set multi-thread for ncnn::Extractor in inference.
    void SetNumThreads(int numThreads);
    void Detect(ncnn::Mat &img_, ncnn::Mat &output);
private:
    ncnn::Net mtcnn_net;
    ncnn::Mat img;
    // Preprocess input image with pixels mean & variance
    const float mean_vals[3] = {127.5, 127.5, 127.5};
    const float norm_vals[3] = {0.0078125, 0.0078125, 0.0078125};
    // Set number of threads for OpenMP
    int num_threads =2;
};


struct Bbox
{
    float score;
    int x1;
    int y1;
    int x2;
    int y2;
    float area;
    float ppoint[10];
    float regreCoord[4];
};


class MTCNNBox {

public:
    MTCNNBox(const string &model_path);
    MTCNNBox(const std::vector<std::string> param_files, const std::vector<std::string> bin_files);
    ~MTCNNBox();

    void SetMinFace(int minSize);
    void SetNumThreads(int numThreads);
    void SetTimeCount(int timeCount);

    void detect(ncnn::Mat& img_, std::vector<Bbox>& finalBbox);
    void detectMaxFace(ncnn::Mat& img_, std::vector<Bbox>& finalBbox);
    //  void detection(const cv::Mat& img, std::vector<cv::Rect>& rectangles);
private:
    void generateBbox(ncnn::Mat score, ncnn::Mat location, vector<Bbox>& boundingBox_, float scale);
    void nmsTwoBoxs(vector<Bbox> &boundingBox_, vector<Bbox> &previousBox_, const float overlap_threshold, string modelname = "Union");
    void nms(vector<Bbox> &boundingBox_, const float overlap_threshold, string modelname="Union");
    void refine(vector<Bbox> &vecBbox, const int &height, const int &width, bool square);
    void extractMaxFace(vector<Bbox> &boundingBox_);

    void PNet(float scale);
    void PNet();
    void RNet();
    void ONet();
    ncnn::Net Pnet, Rnet, Onet;
    ncnn::Mat img;
    const float nms_threshold[3] = {0.5f, 0.7f, 0.7f};

    const float mean_vals[3] = {127.5, 127.5, 127.5};
    const float norm_vals[3] = {0.0078125, 0.0078125, 0.0078125};
    const int MIN_DET_SIZE = 12;
    std::vector<Bbox> firstBbox_, secondBbox_,thirdBbox_;
    std::vector<Bbox> firstPreviousBbox_, secondPreviousBbox_, thirdPrevioussBbox_;
    int img_w, img_h;

private://部分可调参数
    const float threshold[3] = { 0.8f, 0.8f, 0.6f };
    int minsize = 40;
    const float pre_facetor = 0.709f;

    int count = 10;
    int num_threads = 4;

};

#endif //LANDMARK106_H
