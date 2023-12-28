#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef unsigned char BYTE; // 1 byte
typedef unsigned short WORD; // 2 bytes
typedef unsigned int DWORD; // 4 bytes
typedef int LONG; // 4 bytes


//位图文件头
typedef struct tagBITMAPFILEHEADER {
WORD bfType;//指定文件类型，必须是0x424D，即字符串“BM”
DWORD bfSize;//指定文件大小，包括这14个字节。
WORD bfReserved1;
WORD bfReserved2;//为保留字，不用考虑
DWORD bfOffBits;
} BITMAPFILEHEADER;

//位图信息头
typedef struct tagBITMAPINFOHEADER{
DWORD biSize;
LONG biWidth;
LONG biHeight;
WORD biPlanes;
WORD biBitCount;
DWORD biCompression;
DWORD biSizeImage;
LONG biXPelsPerMeter;
LONG biYPelsPerMeter;
DWORD biClrUsed;
DWORD biClrImportant;
} BITMAPINFOHEADER;

//真色彩图的每个像素3BYTE，分别表示RGB的分量
typedef struct tagPIXEL {
BYTE red;
BYTE green;
BYTE blue;
} PIXEL;

//YUV的结构体
typedef struct tagYUV {
double Y;
double U;
double V;
} YUV;


//创建图的位图文件头和位图信息头
BITMAPFILEHEADER bmfh;
BITMAPINFOHEADER bmih;
//int pixel_num;//图像大小，一般不变
void simple_mean(PIXEL **a)
{
    FILE *fp;
    PIXEL *new_pixel[bmih.biHeight];
    for (int i=0;i<bmih.biHeight;i++){
        new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih.biWidth));
    }
    for(int i=0;i<bmih.biHeight;i++){
        for(int j=0;j<bmih.biWidth;j++){
            double sum_r=0;
            double sum_g=0;
            double sum_b=0;

            int cnt=9;
            //滤波窗口相对于当前像素的位置
            for(int n=-1;n<2;n++){
                for(int m=-1;m<2;m++){
                    if(i+n<0||j+m<0||i+n>=bmih.biHeight||j+m>=bmih.biWidth){
                        cnt--;//周围像素不在图像内，求和的像素数减少
                    }else{//周围像素在图像中，求和
                        sum_r+=a[i+n][j+m].red;
                        sum_g+=a[i+n][j+m].green;
                        sum_b+=a[i+n][j+m].blue;
                    }
                }
            }//求周围像素值和的平均数
            new_pixel[i][j].red=sum_r/cnt;
            new_pixel[i][j].green=sum_g/cnt;
            new_pixel[i][j].blue=sum_b/cnt;
        }
    }
    fp = fopen("simple_mean.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih.biWidth, fp);
    }
    fclose(fp);
    //释放内存
    for (int i=0;i<bmih.biHeight;i++){
        free(new_pixel[i]);
    }
}


void weight_mean(PIXEL **a)
{
    FILE *fp;
    int weight[3][3]={{1,2,1},{2,4,2},{1,2,1}};//权重

    PIXEL *new_pixel[bmih.biHeight];
    for (int i=0;i<bmih.biHeight;i++){
        new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih.biWidth));
    }
    for(int i=0;i<bmih.biHeight;i++){
        for(int j=0;j<bmih.biWidth;j++){
            double sum_r=0;
            double sum_g=0;
            double sum_b=0;

            int cnt=16;
            //滤波窗口相对于当前像素的位置
            for(int n=-1;n<2;n++){
                for(int m=-1;m<2;m++){
                    if(i+n<0||j+m<0||i+n>=bmih.biHeight||j+m>=bmih.biWidth){
                        cnt-=weight[1+n][1+m];//周围像素不在图像内，求和的像素数（代权重）减少
                    }else{//周围像素在图像中，乘以权重再求和
                        sum_r+=a[i+n][j+m].red*weight[1+n][1+m];
                        sum_g+=a[i+n][j+m].green*weight[1+n][1+m];
                        sum_b+=a[i+n][j+m].blue*weight[1+n][1+m];
                    }
                }
            }
            new_pixel[i][j].red=sum_r/cnt;
            new_pixel[i][j].green=sum_g/cnt;
            new_pixel[i][j].blue=sum_b/cnt;
        }
    }
    fp = fopen("weight_mean.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih.biWidth, fp);
    }
    fclose(fp);
    //释放内存
    for (int i=0;i<bmih.biHeight;i++){
        free(new_pixel[i]);
    }
}

void gray(PIXEL **a)
{
    FILE *fp;
    YUV *b[bmih.biHeight];
    for (int i=0;i<bmih.biHeight;i++){
        b[i] = (YUV*)malloc(sizeof(YUV)*bmih.biWidth);
    }
    // YUV* b=(YUV*)malloc(sizeof(YUV)*pixel_num);
    // PIXEL* new_pixel=(PIXEL*)malloc(sizeof(PIXEL)*pixel_num);
    PIXEL *new_pixel[bmih.biHeight];
    for (int i=0;i<bmih.biHeight;i++){
        new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*bmih.biWidth);
    }
    for(int i=0;i<bmih.biHeight;i++){
        for(int j=0;j<bmih.biWidth;j++){
            b[i][j].Y=a[i][j].red*0.299+a[i][j].green*0.587+a[i][j].blue*0.114;
            b[i][j].U=a[i][j].red*-0.147+a[i][j].green*-0.289+a[i][j].blue*0.435;
            b[i][j].V=a[i][j].red*0.615+a[i][j].green*-0.515+a[i][j].blue*-0.100;
            //将Y控制在[0,255]
            if(b[i][j].Y<0)    b[i][j].Y=0;
            if(b[i][j].Y>255)  b[i][j].Y=255;
            new_pixel[i][j].blue=new_pixel[i][j].green=new_pixel[i][j].red=b[i][j].Y;
        }
    }
    
    

    //将灰度图输出
    
    //打开一个bmp空白图像并写入数据
    fp = fopen("gray.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih.biWidth, fp);
    }
    //fwrite(new_pixel, 3, pixel_num, fp);
    fclose(fp);
}
void laplacian_enhance(PIXEL **a)
{
    FILE *fp;
    //尝试了多种遮罩
    //int weight[3][3]={{0,1,0},{1,-4,1},{0,1,0}};
    //int weight[3][3]={{0,-1,0},{-1,4,-1},{0,-1,0}};
    //int weight[3][3]={{1,1,1},{1,-8,1},{1,1,1}};
    int weight[3][3]={{-1,-1,-1},{-1,8,-1},{-1,-1,-1}};

    PIXEL *new_pixel[bmih.biHeight];
    for (int i=0;i<bmih.biHeight;i++){
        new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih.biWidth));
    }
    //存储拉普拉斯算子的结果
    //0,1,2分别存储r,g,b
    int *change[3];
    for (int i=0;i<3;i++){
        change[i] = (int*)malloc(sizeof(int)*(bmih.biHeight*bmih.biWidth));
    }
    for(int i=0;i<bmih.biHeight;i++){
        for(int j=0;j<bmih.biWidth;j++){
            double sum_r=0;
            double sum_g=0;
            double sum_b=0;
            //滤波窗口相对于当前像素的位置
            for(int n=-1;n<2;n++){
                for(int m=-1;m<2;m++){
                    if(i+n<0||j+m<0||i+n>=bmih.biHeight||j+m>=bmih.biWidth){
                        continue;//像素位置不合法
                    }else{//计算加权和
                        sum_r+=a[i+n][j+m].red*weight[1+n][1+m];
                        sum_g+=a[i+n][j+m].green*weight[1+n][1+m];
                        sum_b+=a[i+n][j+m].blue*weight[1+n][1+m];
                    }
                }
            }
            int new_r=sum_r;
            int new_g=sum_g;
            int new_b=sum_b;
            change[0][i*bmih.biHeight+j]=new_r;
            change[1][i*bmih.biHeight+j]=new_g;
            change[2][i*bmih.biHeight+j]=new_b;
            //用于输出中间图像
            if(new_r>255)   new_r=255;
            if(new_g>255)   new_g=255;
            if(new_b>255)   new_b=255;
            
            if(new_r<0)   new_r=0;
            if(new_g<0)   new_g=0;
            if(new_b<0)   new_b=0;
            new_pixel[i][j].red=new_r;
            new_pixel[i][j].green=new_g;
            new_pixel[i][j].blue=new_b;
        }
    }
    fp = fopen("Laplacian_result.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih.biWidth, fp);
    }
    fclose(fp);

    //Rearrange后的中间图像
    for(int i=0;i<bmih.biHeight;i++){
        for(int j=0;j<bmih.biWidth;j++){
            new_pixel[i][j].red=new_pixel[i][j].red*0.2;
            new_pixel[i][j].green=new_pixel[i][j].green*0.2;
            new_pixel[i][j].blue=new_pixel[i][j].blue*0.2;
        }
    }
    fp = fopen("rearranged_Laplacian_result.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih.biWidth, fp);
    }
    fclose(fp);

    //叠加增强
    for(int i=0;i<bmih.biHeight;i++){
        for(int j=0;j<bmih.biWidth;j++){
            int new_r=a[i][j].red+change[0][i*bmih.biHeight+j]*0.2;
            int new_g=a[i][j].green+change[1][i*bmih.biHeight+j]*0.2;
            int new_b=a[i][j].blue+change[2][i*bmih.biHeight+j]*0.2;
            //防止溢出
            if(new_r>255)   new_r=255;
            if(new_g>255)   new_g=255;
            if(new_b>255)   new_b=255;
            
            if(new_r<0)   new_r=0;
            if(new_g<0)   new_g=0;
            if(new_b<0)   new_b=0;

            
            new_pixel[i][j].red=new_r;
            new_pixel[i][j].green=new_g;
            new_pixel[i][j].blue=new_b;
        }
    }
    fp = fopen("Laplacian_enhance.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih.biWidth, fp);
    }
    fclose(fp);

    //释放内存
    for (int i=0;i<bmih.biHeight;i++){
        free(new_pixel[i]);
    }
    for (int i=0;i<3;i++){
        free(change[i]);
    }
}
int main()
{
    
    //打开文件并判断是否成功
    FILE *fp;
    fp = fopen("42.bmp", "rb"); // rb 打开一个二进制文件
    if (!fp) {
    printf("BMP Image Not Found!\n");
    exit(0);
    }
    printf("Successfully open the image\n");

    //读取文件头和图像信息头
    fread(&bmfh, 14, 1, fp);
    fread(&bmih, 40, 1, fp);

    //计算图像大小
    //pixel_num=bmih.biHeight*bmih.biWidth;

    //创建数组读取位图数据
    PIXEL *a[bmih.biHeight];
    for (int i=0;i<bmih.biHeight;i++){
        a[i] = (PIXEL*)malloc(sizeof(PIXEL)*bmih.biWidth);
    }
    //PIXEL* a=(PIXEL*)malloc(sizeof(PIXEL)*pixel_num);
    

    //一般来说，.bMP文件的数据从下到上，从左到右存放
    for(int i=bmih.biHeight-1;i>=0;i--){
        fread(a[i], sizeof(PIXEL), bmih.biWidth, fp);
    }
    
    fclose(fp);//关闭该图
    int test;
    printf("Test for?\n0:colored 42.bmp\n1:gray.bmp\n");
    scanf("%d",&test);
    if(test==0){
        simple_mean(a);
        weight_mean(a);
        laplacian_enhance(a);
    }else{
        fp = fopen("gray.bmp", "rb"); // rb 打开一个二进制文件
        if (!fp) {
        printf("BMP Image Not Found!\n");
        exit(0);
        }
        printf("Successfully open the image\n");

        //读取文件头和图像信息头
        fread(&bmfh, 14, 1, fp);
        fread(&bmih, 40, 1, fp);
        for(int i=bmih.biHeight-1;i>=0;i--){
            fread(a[i], sizeof(PIXEL), bmih.biWidth, fp);
        }
        simple_mean(a);
        weight_mean(a);
        laplacian_enhance(a);
    }
    
    for (int i=0;i<bmih.biHeight;i++){
        free(a[i]);
    }

}

