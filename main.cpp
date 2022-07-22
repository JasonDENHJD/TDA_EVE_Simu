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
#include "vcopmem.h"
#include "file_io.h"

#include "vcop_dcoffset_windowing_kernel.k"

#include "vcop_fft_npt_16ix16o_wrapper.h"
#include "vcop_fft_npt_16ix16o_32inter_wrapper.h"
#include "vcop_fft_npt_16ix16o_gen_twiddleFactor.h"
#include "vcop_fft_doppler_correction_kernel.h"

#define FIXED_PATTERN (0)

#define ALIGN_2SIMD(a)   (((a) + 2*VCOP_SIMD_WIDTH-1) & ~(2*VCOP_SIMD_WIDTH-1))
#define CEIL(x, y) ((( x + y - 1) / y) * y)

#define TRANSPOSE_STRIDE ( (VCOP_SIMD_WIDTH + 1 ) * 4 )


using namespace std;

char data_name[] = "../data/20220121_090433_mulit.bin";
uint16_t numLines = 8;
uint16_t numPoints = 256;


int main(int argc, char **argv) {
    VCOP_Mem_Init();

    auto *raw_data = (int16_t *) Read_File_New_Buffer(data_name, 0);


    uint16_t interimTransposeStride;
    uint16_t transposeStride;
    if (numLines < VCOP_SIMD_WIDTH) {
        interimTransposeStride = ((2 * VCOP_SIMD_WIDTH) + 1) * sizeof(uint32_t);
        transposeStride = ((VCOP_SIMD_WIDTH) + 1) * sizeof(uint32_t);
    }
    else {
        interimTransposeStride = ((numLines * 2) + 1) * sizeof(uint32_t);
        transposeStride = (((numLines % 2) ? numLines :
                            numLines + 1)) * sizeof(uint32_t);
    }

    auto *input_data = (int16_t *) VCOP_Mem_Malloc(VCOP_IBUFLA, (2*numPoints * sizeof(int16_t) * 2) * numLines);
    auto *output_data = (int16_t *) VCOP_Mem_Malloc(VCOP_IBUFHA, (2*numPoints * sizeof(int16_t) * 2) * numLines);

    auto *dc_offset_scratch_buf = (int32_t *) VCOP_Mem_Malloc(VCOP_WMEM, (VCOP_SIMD_WIDTH * interimTransposeStride));
    auto *dc_offset_buf = (int16_t *) VCOP_Mem_Malloc(VCOP_WMEM, (sizeof(int32_t)) * numLines);

    auto *win_data_buf = (int16_t *) VCOP_Mem_Malloc(VCOP_WMEM, (numPoints) * sizeof(int16_t));

    auto *scatter_index = (uint16_t *) VCOP_Mem_Malloc(VCOP_WMEM, (VCOP_SIMD_WIDTH * sizeof(int16_t)));

    auto *pScaleFactorOut =(uint8_t *)VCOP_Mem_Malloc(VCOP_WMEM,VCOP_SIMD_WIDTH * 2 * sizeof(uint8_t));

    auto *pScatterOffset = (uint16_t *)VCOP_Mem_Malloc(VCOP_WMEM,(2 * VCOP_SIMD_WIDTH * sizeof(uint16_t)));

    auto *scaleFactor = (uint16_t *)VCOP_Mem_Malloc(VCOP_WMEM,(8 * sizeof(uint16_t)));

    cout << "Stage1 Memory Usage" << endl;
    VCOP_Mem_Print_Usage();

    for (int i = 0; i < VCOP_SIMD_WIDTH; i++) {
        scatter_index[i] = interimTransposeStride * i;
    }

    char win_file_name[] = "../data/hann/hann_256_int16.bin";
    Read_File_Exist_Buffer(win_file_name, win_data_buf, (numPoints) * sizeof(int16_t));

    uint32_t fft_twiddlefactor_size = getSizeTwiddleFactor_256();
    auto *fft_twiddlefactor = (int16_t *) VCOP_Mem_Malloc(VCOP_WMEM, (fft_twiddlefactor_size * sizeof(int16_t)));
    generateTwiddleFactor_256(fft_twiddlefactor);

    for (int i = 0; i < 256; ++i) {
        cout << "frame:" << i << endl;
        uint32_t addr = numPoints * numLines * sizeof(int16_t) * 2 * i;
        memcpy(input_data, raw_data + addr, numPoints * numLines * sizeof(int16_t) * 2);

        vcop_dcoffset_kernel(input_data,
                             dc_offset_scratch_buf,
                             scatter_index,
                             dc_offset_buf,
                             numPoints * sizeof(int16_t) * 2,
                             interimTransposeStride,
                             numPoints,
                             numLines,
                             8
        );

        vcop_windowing_kernel(input_data,/*Input Ptr */
                              win_data_buf,
                              dc_offset_buf,
                              input_data,/*Output Ptr */
                              numPoints,
                              numLines,
                              10,
                              32768,
                              numPoints * sizeof(int16_t) * 2,
                              numPoints * sizeof(int16_t) * 2
        );
        vcop_fft_512_16i_16o_32inter(input_data,
                                     input_data,
                                     output_data,
                                     fft_twiddlefactor,
                                     pScatterOffset,
                                     pScaleFactorOut,
                                     15,
                                     numPoints * sizeof(int16_t) * 2,
                                     scaleFactor,
                                     numLines,
                                     1,
                                     VCOP_FFT_NPT_16ix16o_STAGE_ALL_OVERFLOW,
                                     32768);
        for (int j = 0; j < numLines; j++) {
            printf("line %d: %d, %d \n", j, input_data[j * 2], input_data[j * 2 + 1]);
        }
    }


    VCOP_Mem_Free(input_data);
    VCOP_Mem_Free(output_data);
    VCOP_Mem_Free(dc_offset_scratch_buf);
    VCOP_Mem_Free(dc_offset_buf);
    VCOP_Mem_Free(win_data_buf);
    VCOP_Mem_Free(scatter_index);

    free(raw_data);

    cout << "End Memory Usage" << endl;
    VCOP_Mem_Print_Usage();

    return 0;
}
