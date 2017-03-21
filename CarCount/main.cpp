/*
 * 创建时间：2017/3/19
 * 代码功能：车流量计数
 * 实验内容：添加相关测试功能，删除linux下不能使用的函数
 * 实验内容: 实现双向独立单线检测

 * 使用者： hugooood
 *
 *
 */
// main.cpp

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include <QTextStream>

//控制台输入输出
#include<conio.h>           // it may be necessary to change or remove this line if not using Windows

#include "Block.h"

#define SHOW_STEPS            // un-comment or comment this line to show steps or not

// global variables ///////////////////////////////////////////////////////////////////////////////
const cv::Scalar SCALAR_BLACK = cv::Scalar(0.0, 0.0, 0.0);
const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::Scalar SCALAR_YELLOW = cv::Scalar(0.0, 255.0, 255.0);
const cv::Scalar SCALAR_GREEN = cv::Scalar(0.0, 200.0, 0.0);
const cv::Scalar SCALAR_RED = cv::Scalar(0.0, 0.0, 255.0);

// function prototypes ////////////////////////////////////////////////////////////////////////////
void matchCurrentFrameBlocksToExistingBlocks(std::vector<Block> &existingBlocks, std::vector<Block> &currentFrameBlocks);
void addBlockToExistingBlocks(Block &currentFrameBlock, std::vector<Block> &existingBlocks, int &intIndex);
void addNewBlock(Block &currentFrameBlock, std::vector<Block> &existingBlocks);
double distanceBetweenPoints(cv::Point point1, cv::Point point2);
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName);
void drawAndShowContours(cv::Size imageSize, std::vector<Block> blocks, std::string strImageName);
bool checkIfBlocksCrossedTheLine01(std::vector<Block> &blocks, int &intHorizontalLinePosition01, int &carCount01);
void drawBlockInfoOnImage(std::vector<Block> &blocks, cv::Mat &imgFrame2Copy);
void drawCarCountOnImage01(int &carCount01, cv::Mat &imgFrame2Copy);

bool checkIfBlocksCrossedTheLine02(std::vector<Block> &blocks, int &intHorizontalLinePosition02, int &carCount02);
void drawCarCountOnImage02(int &carCount02, cv::Mat &imgFrame2Copy);


///////////////////////////////////////////////////////////////////////////////////////////////////
int main(void) {

    cv::VideoCapture capVideo(1);

    cv::Mat imgFrame1;
    cv::Mat imgFrame2;

    std::vector<Block> blocks;

    cv::Point crossingLine01[2];//下行线
    cv::Point crossingLine02[2];//上行线

    int carCount01 = 0;//下行计数
    int carCount02 = 0;//上行计数

    //控制台输出中文
    QTextStream out(stdout);
    //算法原始视频 D:\\NewCV\\resource\\NewCar00.mp4
    //原分辨率视频 D:\\NewCV\\resource\\00797.avi
    //实验视频 D:\\NewCV\\resource\\NewCar01L.mp4
    capVideo.open("D:\\NewCV\\resource\\NewCar01L.mp4");

    if (!capVideo.isOpened()) {                                                 // if unable to open video file
        std::cout << "error reading video file" << std::endl << std::endl;      // show error message
       return(0);                                                              // and exit program
    }

    //测试视频一共多少帧
    long totalFrameNumber = capVideo.get(CV_CAP_PROP_FRAME_COUNT);
    out<<QString("整个视频共")<<totalFrameNumber<<QString("帧")<<endl;

    //设置开始帧()
    long frameToStart = 1;
    capVideo.set( CV_CAP_PROP_POS_FRAMES,frameToStart);
    out<<QString("从第")<<frameToStart<<QString("帧开始读")<<endl;


    //设置结束帧（）
    int frameToStop = 3000;

    if(frameToStop < frameToStart)
    {
        out<<QString("结束帧小于开始帧，程序错误，即将退出！")<<endl;
        return -1;
    }
    else
    {
        out<<QString("结束帧为：第")<<frameToStop<<QString("帧")<<endl;
    }


    //获取帧率
    double rate = capVideo.get(CV_CAP_PROP_FPS);
    out<<QString("帧率为:")<<rate<<endl;




    //注意参数为2，视频总帧数小于两帧时报错
    if (capVideo.get(CV_CAP_PROP_FRAME_COUNT) < 2) {

        std::cout << "error: video file must have at least two frames";
        return(0);
    }
//char chCheckForEscKey = 0;
//   while (capVideo.isOpened() && chCheckForEscKey != 27) {
    capVideo.read(imgFrame1);
    capVideo.read(imgFrame2);
    cv::imshow("imgFrame1",imgFrame1);
    cv::imshow("imgFrame2",imgFrame2);

    //设置下行虚拟检测线 原始参数为0.35
    int intHorizontalLinePosition01 = (int)std::round((double)imgFrame1.rows * 0.55);

    // 中间单线（0.35）      下行单线(0.55)
    //      370                 220
    //      340                 400
    //越大越右（右边）
    crossingLine01[0].x = 220;
    crossingLine01[0].y = intHorizontalLinePosition01;
    //越大越左（左边）
    crossingLine01[1].x = imgFrame1.cols - 400;  //列
    crossingLine01[1].y = intHorizontalLinePosition01;

    //设置上行虚拟检测线
    int intHorizontalLinePosition02 = (int)std::round((double)imgFrame1.rows * 0.35);

    //越大越右（右边）
    crossingLine02[0].x = 350;
    crossingLine02[0].y = intHorizontalLinePosition02;
    //越大越左（左边）
    crossingLine02[1].x = imgFrame1.cols - 160;  //列
    crossingLine02[1].y = intHorizontalLinePosition02;


    char chCheckForEscKey = 0;

    bool blnFirstFrame = true;

    int frameCount = 2;

    while (capVideo.isOpened() && chCheckForEscKey != 27) {

        std::vector<Block> currentFrameBlocks;

        //
        cv::Mat imgFrame1Copy = imgFrame1.clone();
        cv::Mat imgFrame2Copy = imgFrame2.clone();

        cv::Mat imgDifference;
        cv::Mat imgThresh;

        cv::cvtColor(imgFrame1Copy, imgFrame1Copy, CV_BGR2GRAY);
        cv::cvtColor(imgFrame2Copy, imgFrame2Copy, CV_BGR2GRAY);

        //高斯平滑
        cv::GaussianBlur(imgFrame1Copy, imgFrame1Copy, cv::Size(5, 5), 0);
        cv::GaussianBlur(imgFrame2Copy, imgFrame2Copy, cv::Size(5, 5), 0);

        //计算两个数组差的绝对值 dst(I)c = abs(src1(I)c - src2(I)c).
        cv::absdiff(imgFrame1Copy, imgFrame2Copy, imgDifference);

      //  实验观察使用
        cv::imshow("imgDifference", imgDifference);

        cv::threshold(imgDifference, imgThresh, 30, 255.0, CV_THRESH_BINARY);

        cv::imshow("imgThresh", imgThresh);

        cv::Mat structuringElement3x3 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));

        cv::Mat structuringElement7x7 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
        cv::Mat structuringElement15x15 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15));

        cv::Mat structuringElement5x5 = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        for (unsigned int i = 0; i < 2; i++) {
            //膨胀
            cv::dilate(imgThresh, imgThresh, structuringElement5x5);
            cv::dilate(imgThresh, imgThresh, structuringElement5x5);
            //腐蚀
            cv::erode(imgThresh, imgThresh, structuringElement5x5);
        }

        //形态学处理后的效果图
        cv::imshow("imgThresh2", imgThresh);

       //                       创建一个完整的数组副本
        cv::Mat imgThreshCopy = imgThresh.clone();

        //验证上面是否创建完整副本
     //   cv::imshow("imgThreshCopy", imgThreshCopy);


        std::vector<std::vector<cv::Point> > contours;//contours被定义成二维浮点型向量，这里面将来会存储找到的边界的（x,y）坐标


        cv::findContours(imgThreshCopy, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        //这个函数很吊的样子
        drawAndShowContours(imgThresh.size(), contours, "imgContours");

        //凸包
        std::vector<std::vector<cv::Point> > convexHulls(contours.size());

        for (unsigned int i = 0; i < contours.size(); i++) {

            //由点组成的轮廓，通过convexHull函数，就能得到轮廓的凸包
            cv::convexHull(
                        contours[i],//要求凸包的点集
                        convexHulls[i]//输出的凸包点
                           );
        }

        drawAndShowContours(imgThresh.size(), convexHulls, "imgConvexHulls");

        //convexHull(凸包)在这里是变量
        //该for循环用于寻找符合条件的凸包
        for (auto &convexHull : convexHulls) {
            Block possibleBlock(convexHull);   //潜在的凸包

            //重点代码
            //寻找在取值范围内的图像，去除干扰项 原始400
            if (//possibleBlock.currentBoundingRect.area() < 4000 &&
                possibleBlock.currentBoundingRect.area() > 100 &&
                possibleBlock.dblCurrentAspectRatio > 0.2 && //最适横轴比
                possibleBlock.dblCurrentAspectRatio < 4.0 &&
                possibleBlock.currentBoundingRect.width > 30 &&
                possibleBlock.currentBoundingRect.height > 30 &&
                possibleBlock.dblCurrentDiagonalSize > 60.0 &&  //最适对角线尺寸
                (cv::contourArea(possibleBlock.currentContour) / (double)possibleBlock.currentBoundingRect.area()) > 0.50)
            {
                currentFrameBlocks.push_back(possibleBlock);//尾插
            }
        }

        drawAndShowContours(imgThresh.size(), currentFrameBlocks, "imgCurrentFrameBlocks");

        if (blnFirstFrame == true) {
            for (auto &currentFrameBlock : currentFrameBlocks) {
                blocks.push_back(currentFrameBlock);//std::vector<Block>类型
            }
        } else {
            matchCurrentFrameBlocksToExistingBlocks(blocks, currentFrameBlocks);
        }

        drawAndShowContours(imgThresh.size(), blocks, "imgBlocks");

        imgFrame2Copy = imgFrame2.clone();          // get another copy of frame 2 since we changed the previous frame 2 copy in the processing above

        //画框
        drawBlockInfoOnImage(blocks, imgFrame2Copy);

        //判断车辆是否过线（Down）
        bool blnAtLeastOneBlockCrossedTheLine01 = checkIfBlocksCrossedTheLine01(blocks, intHorizontalLinePosition01, carCount01);

        if (blnAtLeastOneBlockCrossedTheLine01 == true) {
            cv::line(imgFrame2Copy,//要划的线所在的图像
                     crossingLine01[0],//起点
                    crossingLine01[1],//终点
                    SCALAR_GREEN,
                    5//粗细
                    );
        }
        else {
            cv::line(imgFrame2Copy, crossingLine01[0], crossingLine01[1], SCALAR_RED, 5);
        }

        //判断车辆是否过线（Up）
        bool blnAtLeastOneBlockCrossedTheLine = checkIfBlocksCrossedTheLine02(blocks, intHorizontalLinePosition02, carCount02);

        if (blnAtLeastOneBlockCrossedTheLine == true) {
            cv::line(imgFrame2Copy,//要划的线所在的图像
                     crossingLine02[0],//起点
                    crossingLine02[1],//终点
                    SCALAR_GREEN,
                    5//粗细
                    );
        }
        else {
            cv::line(imgFrame2Copy, crossingLine02[0], crossingLine02[1], SCALAR_RED, 5);
        }




        //显示数字
        drawCarCountOnImage01(carCount01, imgFrame2Copy);
        drawCarCountOnImage02(carCount02, imgFrame2Copy);

        cv::namedWindow("imgFrame2Copy",0);
        cv::imshow("imgFrame2Copy", imgFrame2Copy);

        //cv::waitKey(0);                 // uncomment this line to go frame by frame for debugging

                // now we prepare for the next iteration

        currentFrameBlocks.clear();

        imgFrame1 = imgFrame2.clone();           // move frame 1 up to where frame 2 is

        if ((capVideo.get(CV_CAP_PROP_POS_FRAMES) + 1) < capVideo.get(CV_CAP_PROP_FRAME_COUNT)) {
            capVideo.read(imgFrame2);
        }
        else {
            std::cout << "end of video\n";
            break;
        }

        blnFirstFrame = false;
        frameCount++;
        chCheckForEscKey = cv::waitKey(1);
    }

    if (chCheckForEscKey != 27) {               // if the user did not press esc (i.e. we reached the end of the video)
        cv::waitKey(1);                         // hold the windows open to allow the "end of video" message to show
    }
    // note that if the user did press esc, we don't need to hold the windows open, we can simply let the program end which will close the windows

    return(0);
}


//自定义函数
///////////////////////////////////////////////////////////////////////////////////////////////////
//匹配当前帧的团块到                                             现有的团块？                         最近的团块？
void matchCurrentFrameBlocksToExistingBlocks(std::vector<Block> &existingBlocks, std::vector<Block> &currentFrameBlocks) {

    for (auto &existingBlock : existingBlocks) {

        existingBlock.blnCurrentMatchFoundOrNewBlock = false;

        existingBlock.predictNextPosition();//预测凸包出现位置？
    }

    for (auto &currentFrameBlock : currentFrameBlocks) {

        int intIndexOfLeastDistance = 0;    //标记

        //该参数决定了车辆间的距离，参数越小则车辆相邻越近才会被判定为车辆100000.0
        double dblLeastDistance = 100000.0; //车辆中心的最小距离

        for (unsigned int i = 0; i < existingBlocks.size(); i++) {

            if (existingBlocks[i].blnStillBeingTracked == true) {

                //计算两个凸包中心点的距离
                double dblDistance = distanceBetweenPoints(
                     currentFrameBlock.centerPositions.back(),//back()返回最后一个元素
                     existingBlocks[i].predictedNextPosition
                            );

                if (dblDistance < dblLeastDistance) {
                    dblLeastDistance = dblDistance;
                    intIndexOfLeastDistance = i;//标记
                }
            }
        }


        if (dblLeastDistance < currentFrameBlock.dblCurrentDiagonalSize * 0.5) {
            addBlockToExistingBlocks(currentFrameBlock,
                                   existingBlocks,
                                   intIndexOfLeastDistance//标记(索引)
                                   );
        }
        else {
            addNewBlock(currentFrameBlock, existingBlocks);
        }

    }
/////???????????
    for (auto &existingBlock : existingBlocks) {

        if (existingBlock.blnCurrentMatchFoundOrNewBlock == false) {
            existingBlock.intNumOfConsecutiveFramesWithoutAMatch++;
        }

        if (existingBlock.intNumOfConsecutiveFramesWithoutAMatch >= 5) {
            existingBlock.blnStillBeingTracked = false;
        }

    }

}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addBlockToExistingBlocks(Block &currentFrameBlock, std::vector<Block> &existingBlocks, int &intIndex) {

    existingBlocks[intIndex].currentContour = currentFrameBlock.currentContour;
    existingBlocks[intIndex].currentBoundingRect = currentFrameBlock.currentBoundingRect;

    existingBlocks[intIndex].centerPositions.push_back(currentFrameBlock.centerPositions.back());

    existingBlocks[intIndex].dblCurrentDiagonalSize = currentFrameBlock.dblCurrentDiagonalSize;
    existingBlocks[intIndex].dblCurrentAspectRatio = currentFrameBlock.dblCurrentAspectRatio;

    existingBlocks[intIndex].blnStillBeingTracked = true;
    existingBlocks[intIndex].blnCurrentMatchFoundOrNewBlock = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void addNewBlock(Block &currentFrameBlock, std::vector<Block> &existingBlocks) {

    currentFrameBlock.blnCurrentMatchFoundOrNewBlock = true;

    existingBlocks.push_back(currentFrameBlock);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//两点间的距离公式
double distanceBetweenPoints(cv::Point point1, cv::Point point2) {


    int intX = abs(point1.x - point2.x);
    int intY = abs(point1.y - point2.y);

    //平方和开根号(pow用于计算次幂)
    return(sqrt(pow(intX, 2) + pow(intY, 2)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawAndShowContours(cv::Size imageSize, std::vector<std::vector<cv::Point> > contours, std::string strImageName) {
    cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

    cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

    cv::imshow(strImageName, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawAndShowContours(cv::Size imageSize, std::vector<Block> blocks, std::string strImageName) {

    cv::Mat image(imageSize, CV_8UC3, SCALAR_BLACK);

    std::vector<std::vector<cv::Point> > contours;

    for (auto &block : blocks) {
        if (block.blnStillBeingTracked == true) {
            contours.push_back(block.currentContour);
        }
    }

    cv::drawContours(image, contours, -1, SCALAR_WHITE, -1);

    cv::imshow(strImageName, image);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//下行计数（Down）
bool checkIfBlocksCrossedTheLine01(std::vector<Block> &blocks, int &intHorizontalLinePosition01, int &carCount01) {
    bool blnAtLeastOneBlockCrossedTheLine01 = false;

    for (auto block : blocks) {

        if (block.blnStillBeingTracked == true && block.centerPositions.size() >= 2) {
            int prevFrameIndex = (int)block.centerPositions.size() - 2;//上一帧索引
            int currFrameIndex = (int)block.centerPositions.size() - 1;//当前帧索引

//上行判断
//block.centerPositions[prevFrameIndex].y > intHorizontalLinePosition01 &&
//block.centerPositions[currFrameIndex].y <= intHorizontalLinePosition01

//下行判断
//block.centerPositions[prevFrameIndex].y < intHorizontalLinePosition01 &&
//block.centerPositions[currFrameIndex].y >= intHorizontalLinePosition01
            if (block.centerPositions[prevFrameIndex].y < intHorizontalLinePosition01 && block.centerPositions[currFrameIndex].y >= intHorizontalLinePosition01) {
                carCount01++;
                blnAtLeastOneBlockCrossedTheLine01 = true;
            }
        }

    }

    return blnAtLeastOneBlockCrossedTheLine01;
}

////////////////////////////////////////////

//上行计数（Up）
bool checkIfBlocksCrossedTheLine02(std::vector<Block> &blocks, int &intHorizontalLinePosition02, int &carCount02) {
    bool blnAtLeastOneBlockCrossedTheLine = false;

    for (auto block : blocks) {

        if (block.blnStillBeingTracked == true && block.centerPositions.size() >= 2) {
            int prevFrameIndex = (int)block.centerPositions.size() - 2;//上一帧索引
            int currFrameIndex = (int)block.centerPositions.size() - 1;//当前帧索引
          if (block.centerPositions[prevFrameIndex].y > intHorizontalLinePosition02 && block.centerPositions[currFrameIndex].y <= intHorizontalLinePosition02) {
                carCount02++;
                blnAtLeastOneBlockCrossedTheLine = true;
            }
        }

    }

    return blnAtLeastOneBlockCrossedTheLine;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
void drawBlockInfoOnImage(std::vector<Block> &blocks, cv::Mat &imgFrame2Copy) {

    for (unsigned int i = 0; i < blocks.size(); i++) {

        if (blocks[i].blnStillBeingTracked == true) {
            cv::rectangle(imgFrame2Copy, blocks[i].currentBoundingRect, SCALAR_RED, 2);

            int intFontFace = CV_FONT_HERSHEY_SIMPLEX;
            double dblFontScale = blocks[i].dblCurrentDiagonalSize / 60.0;
            int intFontThickness = (int)std::round(dblFontScale * 1.0);

            cv::putText(imgFrame2Copy,//作用的图片
                        std::to_string(i),//给识别的车辆标号
                        blocks[i].centerPositions.back(),//坐标
                        intFontFace,//字体
                        dblFontScale,//字形
                        SCALAR_GREEN,
                        intFontThickness
                        );
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void drawCarCountOnImage01(int &carCount01, cv::Mat &imgFrame2Copy) {

    int intFontFace = CV_FONT_HERSHEY_SIMPLEX;//字体：正常大小无衬线
    double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 100000.0;//字体缩放大小
    int intFontThickness = (int)std::round(dblFontScale * 1.5);//粗细

    cv::Size textSize = cv::getTextSize(std::to_string(carCount01), intFontFace, dblFontScale, intFontThickness, 0);

    cv::Point ptTextBottomLeftPosition;

    //carCount01坐标
    ptTextBottomLeftPosition.x = imgFrame2Copy.cols - 1 - (int)((double)textSize.width * 1.25)-350;
    ptTextBottomLeftPosition.y = (int)((double)textSize.height * 1.25)+110;

    cv::putText(imgFrame2Copy, std::to_string(carCount01), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);


}


////////////////////////////////////////////////////////////
void drawCarCountOnImage02(int &carCount02, cv::Mat &imgFrame2Copy) {

    int intFontFace = CV_FONT_HERSHEY_SIMPLEX;//字体：正常大小无衬线
    double dblFontScale = (imgFrame2Copy.rows * imgFrame2Copy.cols) / 100000.0;//字体缩放大小
    int intFontThickness = (int)std::round(dblFontScale * 1.5);//粗细

    cv::Size textSize = cv::getTextSize(std::to_string(carCount02), intFontFace, dblFontScale, intFontThickness, 0);

    cv::Point ptTextBottomLeftPosition;

    //carCount02坐标
    ptTextBottomLeftPosition.x = imgFrame2Copy.cols - 1 - (int)((double)textSize.width * 1.25)-150;
    ptTextBottomLeftPosition.y = (int)((double)textSize.height * 1.25)+50;

    cv::putText(imgFrame2Copy, std::to_string(carCount02), ptTextBottomLeftPosition, intFontFace, dblFontScale, SCALAR_GREEN, intFontThickness);

}
