//
// Created by jason on 2022/6/28.
//
//#include <mgl2/fltk.h>
//#include <algorithm>
//
//int graph(mglGraph *gr) {
//    FILE *fp;
//    if( (fp=fopen("../data/20220121_090433_mulit.bin","rb")) == NULL ){
//        printf("Fail to open file!\n");
//        exit(0);
//    }
//
//
//    //求得文件的大小
//    fseek(fp, 0, SEEK_END);
//    int size = ftell(fp);
//    rewind(fp);
//
//    //申请一块能装下整个文件的空间
//    auto *ar = (int16_t*)malloc(sizeof(int16_t)*size);
//
//    int16_t *ar_raw = ar;
//    //读文件
//    fread(ar,size,1,fp);//每次读一个，共读size次
//
//    fclose(fp);
//
//    std::vector<int> data;
//
//    for(int i=0; i < 256; i++)
//    {
//        int16_t aa = *ar_raw;
//        printf("aa=%d\n", aa);
//        data.push_back(aa);
//        ar_raw +=2;
//    }
//
//    free(ar);
//    std::vector<float> datavec1{ 1,2,3,4,5,6 ,7,};
//    std::vector<float> datavec2{ 2,4,9,1,6,8, 8,};
//    mglData x(datavec1);
//    mglData y(datavec2);
//    mglData mgl_data(data);
//
//    gr->Title("MathGL Demo");
//    auto miny = *std::min_element(data.begin(), data.end());
//    auto maxy = *std::max_element(data.begin(), data.end());
//    auto y_margin = (maxy - miny) / data.size();
//    gr->SetBarWidth(2);
//    gr->SetOrigin(0, 0);
//    gr->SetRanges(0, 256, -4000, 4000);
////    gr->FPlot("sin(1.7*2*pi*x) + sin(1.9*2*pi*x)", "r-4");
//    gr->Plot(mgl_data, "H");
//    gr->Axis();
//    gr->Grid();
//    return 0;
// }
//
//int32_t main(int32_t argc, char *argv[]) {
//
//    int status = 0;
//
//    mglGraph gr(0, 1600, 1200);
//
//    mglFLTK gr(graph, "MathGL demo window title");
//    return gr.Run();
//
//    return status;
//}
//

//
//#include <opencv2/core.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2/highgui.hpp>
//#include "mgl2/mgl.h"

//using namespace cv;
//int my_plot(mglGraph* gr, std::vector<int> data)
//{
//
//    mglData mgl_data(data);
//    auto miny = *std::min_element(data.begin(), data.end());
//    auto maxy = *std::max_element(data.begin(), data.end());
//    gr->SetRanges(0, (int)data.size(), miny, maxy);
//    gr->Axis(); // 坐标轴
//    gr->Plot(mgl_data, "H");
//    return 0;
//}
//
//    //创建gr对象，指定图像大小为800x500,kind=0说明不使用OpenGL
//    mglGraph gr1(0, 1600, 1200);
//
//    my_plot(&gr1, data);
//    //用OpenCV显示图片
//    Mat pic(gr1.GetHeight(), gr1.GetWidth(), CV_8UC3);
//    pic.data = const_cast<uchar*>(gr1.GetRGB());
//    imshow("test", pic);
//
//
//    waitKey(0);

#include <iostream>
#include <algorithm>
#include <vector>


#include "vcop.h"
#include "vcop_dcoffset_windowing_kernel.k"
#include "vcop_fft_npt_16ix16o_wrapper.h"
#include "vcop_fft_npt_16ix16o_32inter_wrapper.h"

#include "vcop_fft_npt_16ix16o_gen_twiddleFactor.h"

#include "vcop_fft_doppler_correction_kernel.h"

#define FIXED_PATTERN (0)
//#define __DEBUG
#define MAX_CONFIG_LINE_SIZE     (300)
#define MAX_FILE_NAME_SIZE       (200)
#define MAX_PARAM_STRING_SIZE    (30)

#define   ALIGN_2SIMD(a)   (((a) + 2*VCOP_SIMD_WIDTH-1) & ~(2*VCOP_SIMD_WIDTH-1))
#define CEIL(x, y) ((( x + y - 1) / y) * y)

#define TRANSPOSE_STRIDE ( (VCOP_SIMD_WIDTH + 1 ) * 4 )

#if (VCOP_HOST_EMULATION)
#define DCOFFSET_WINDOWING_TEST_malloc(heap, size)   malloc(size)
#define DCOFFSET_WINDOWING_TEST_free(ptr)            free(ptr)
#else
#define DCOFFSET_WINDOWING_TEST_malloc(heap, size)   (vcop_malloc((heap), (size)))
#define DCOFFSET_WINDOWING_TEST_free(ptr)            (vcop_free(ptr))
#endif

using namespace std;

int main(int argc, char **argv) {
    FILE *fp;
    if ((fp = fopen("../data/20220121_090433_mulit.bin", "rb")) == NULL) {
        printf("Fail to open file!\n");
        exit(0);
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    rewind(fp);
    auto *raw_data = (int16_t *) malloc(sizeof(int16_t) * size);
    fread(raw_data, size, 1, fp);
    fclose(fp);

    if ((fp = fopen("../data/hann_256_32768.bin", "rb")) == NULL) {
        printf("Fail to open file!\n");
        exit(0);
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    rewind(fp);
    auto *win_data = (int16_t *) malloc(sizeof(int16_t) * size);
    fread(win_data, size, 1, fp);
    fclose(fp);

    uint16_t numLines = 8;
    uint16_t numPoints = 256;

    int32_t i, j;
    int16_t *inputData = NULL;
    int16_t *outputData = NULL;
    int32_t *scratchBuf = NULL;
    uint16_t *pScatterIndex = NULL;
    int16_t *winCoeffBuf = NULL;
    int16_t *dcOffsetBuf = NULL;
    uint16_t interimTransposeStride;
    uint16_t transposeStride;

    if (numLines < VCOP_SIMD_WIDTH) {
        interimTransposeStride = ((2 * VCOP_SIMD_WIDTH) + 1) * sizeof(uint32_t);
        transposeStride = ((VCOP_SIMD_WIDTH) + 1) * sizeof(uint32_t);
    } else {
        interimTransposeStride = ((numLines * 2) + 1) * sizeof(uint32_t);
        transposeStride = (((numLines % 2) ? numLines :
                            numLines + 1)) * sizeof(uint32_t);
    }

    inputData = (int16_t *) DCOFFSET_WINDOWING_TEST_malloc(VCOP_IBUFHA, (numPoints * sizeof(int16_t) * 2) * numLines);
    outputData = (int16_t *) DCOFFSET_WINDOWING_TEST_malloc(VCOP_IBUFHA, (numPoints * sizeof(int16_t) * 2) * numLines);
    scratchBuf = (int32_t *) DCOFFSET_WINDOWING_TEST_malloc(VCOP_WMEM, (VCOP_SIMD_WIDTH * interimTransposeStride));
    dcOffsetBuf = (int16_t *) DCOFFSET_WINDOWING_TEST_malloc(VCOP_WMEM, (sizeof(int32_t)) * numLines);
    winCoeffBuf = (int16_t *) DCOFFSET_WINDOWING_TEST_malloc(VCOP_WMEM, (numPoints) * sizeof(int16_t));
    pScatterIndex = (uint16_t *) DCOFFSET_WINDOWING_TEST_malloc(VCOP_WMEM, (VCOP_SIMD_WIDTH * sizeof(int16_t)));
    for (i = 0; i < VCOP_SIMD_WIDTH; i++) {
        pScatterIndex[i] = interimTransposeStride * i;
    }
    memcpy(inputData, raw_data, numPoints * numLines * sizeof(int16_t) * 2);
    memcpy(winCoeffBuf, win_data, 256*sizeof(uint16_t));


    vcop_dcoffset_kernel(inputData,
                         scratchBuf,
                         pScatterIndex,
                         dcOffsetBuf,
                         numPoints * sizeof(int16_t) * 2,
                         interimTransposeStride,
                         numPoints,
                         numLines,
                         8
    );

    vcop_windowing_kernel(inputData,/*Input Ptr */
                          winCoeffBuf,
                          dcOffsetBuf,
                          outputData,/*Output Ptr */
                          numPoints,
                          numLines,
                          10,
                          32768,
                          numPoints * sizeof(int16_t) * 2,
                          numPoints * sizeof(int16_t) * 2
    );


    for (i = 0; i < numLines; i++) {
        printf("line %d: %d, %d \n", i, outputData[i * 2], outputData[i * 2 + 1]);
    }

    return 0;
}
