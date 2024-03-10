// #ifndef YOLOV5_LITE_H
// #define YOLOV5_LITE_H

// #include <fstream>
// #include <iostream>
// #include <opencv2/dnn.hpp>    // 深度学习模块
// #include <opencv2/highgui.hpp>// 高层GUI图形用户界面
// #include <opencv2/imgproc.hpp>// 图像处理模块
// #include <sstream>

// #include <time.h>


// // 命名空间
// using namespace cv;
// using namespace dnn;
// using namespace std;

// // 自定义配置结构
// struct Configuration {
// public:
//     float confThreshold;// Confidence threshold
//     float nmsThreshold; // Non-maximum suppression threshold
//     float objThreshold; //Object Confidence threshold
//     string modelpath;
// };

// // 模型
// class YOLOv5
// {
// public:
//     // 初始化
//     YOLOv5(Configuration config, bool isCuda);
//     void detect(Mat &frame);// 检测函数
// private:
//     float confThreshold;
//     float nmsThreshold;
//     float objThreshold;
//     int inpWidth;
//     int inpHeight;
//     int num_classes;
//     string classes[80] = {"person", "bicycle", "car", "motorbike", "aeroplane", "bus",
//                           "train", "truck", "boat", "traffic light", "fire hydrant",
//                           "stop sign", "parking meter", "bench", "bird", "cat", "dog",
//                           "horse", "sheep", "cow", "elephant", "bear", "zebra", "giraffe",
//                           "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
//                           "skis", "snowboard", "sports ball", "kite", "baseball bat",
//                           "baseball glove", "skateboard", "surfboard", "tennis racket",
//                           "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl",
//                           "banana", "apple", "sandwich", "orange", "broccoli", "carrot",
//                           "hot dog", "pizza", "donut", "cake", "chair", "sofa", "pottedplant",
//                           "bed", "diningtable", "toilet", "tvmonitor", "laptop", "mouse",
//                           "remote", "keyboard", "cell phone", "microwave", "oven", "toaster",
//                           "sink", "refrigerator", "book", "clock", "vase", "scissors",
//                           "teddy bear", "hair drier", "toothbrush"};
//     //vector<string> class_names;
//     const bool keep_ratio = true;
//     Net net;// dnn里的
//     void drawPred(float conf, int left, int top, int right, int bottom, Mat &frame, int classid);
//     Mat resize_image(Mat srcimg, int *newh, int *neww, int *top, int *left);
// };


// int main()
// {
//     double timeStart = (double) getTickCount();
//     clock_t startTime, endTime;//计算时间
//     // 自己定义的yolo一些配置超参
//     Configuration yolo_nets = {0.3, 0.5, 0.3, "yolov5s.onnx"};
//     YOLOv5 yolo_model(yolo_nets, false);
//     string imgpath = "bus.jpg";
//     Mat srcimg = imread(imgpath);

//     double nTime = ((double) getTickCount() - timeStart) / getTickFrequency();
//     startTime = clock();//计时开始
//     yolo_model.detect(srcimg);
//     endTime = clock();//计时结束
//     cout << "clock_running time is:" << (double) (endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
//     cout << "The whole run time is:" << (double) clock() / CLOCKS_PER_SEC << "s" << endl;
//     cout << "getTickCount_running time :" << nTime << "s" << endl;

//     static const string kWinName = "Deep learning object detection in OpenCV";
//     namedWindow(kWinName, WINDOW_NORMAL);// 自适应调节窗口大小
//     imwrite("restult_cpu.jpg", srcimg);
//     imshow(kWinName, srcimg);
//     waitKey(0);
//     destroyAllWindows();
//     return 0;
// }

// #endif//YOLOV5_LITE_H