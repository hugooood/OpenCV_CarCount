// Block.h

#ifndef MY_BlOCK
#define MY_BlOCK

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////
class Block {
public:
    // member variables ///////////////////////////////////////////////////////////////////////////
    std::vector<cv::Point> currentContour;  //当前轮廓

    cv::Rect currentBoundingRect;  //当前边界矩形

    std::vector<cv::Point> centerPositions; //中心位置

    double dblCurrentDiagonalSize; //最适对角线尺寸（DBL数据类型）
    double dblCurrentAspectRatio;  //最适横轴比

    bool blnCurrentMatchFoundOrNewBlock;

    bool blnStillBeingTracked;  //一直跟踪

    int intNumOfConsecutiveFramesWithoutAMatch;

    cv::Point predictedNextPosition;  //预测下一个位置

    // function prototypes ////////////////////////////////////////////////////////////////////////
    Block(std::vector<cv::Point> _contour);
    void predictNextPosition(void);

};

#endif    // MY_BLOCK

