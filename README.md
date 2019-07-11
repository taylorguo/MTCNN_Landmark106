# Android MTCNN Landmark106


## NCNN 优化 MTCNN

### NCNN优化库
https://github.com/Tencent/ncnn

### 模型
来源于https://github.com/MirrorYuChen/ncnn_106landmarks

### BBox检测

代码来源于：
https://github.com/moli232777144/mtcnn_ncnn


*********************
*********************

## 问题点汇总

=== 2019年7月11日 ===
### 问题1: Landmark部分坐标点偏移

[Landmark部分坐标点偏移: 【效果好像不是很理想 #1】](https://github.com/MirrorYuChen/ncnn_106landmarks/issues/1)

#### 方案1
- 1. 修改了Android图像数据格式: 无明显好转
- 2. 采用OpenCV处理图像: 无变化

#### 方案2
- 1. [Windows7/10: train mtcnn with mxnet](https://github.com/zuoqing1988/train-mtcnn)
- 2. [查看Python源代码: MTCNN implementation in MXnet](https://github.com/Seanlinx/mtcnn)
