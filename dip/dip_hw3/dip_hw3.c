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
int pixel_num;//图像大小，一般不变
int main()
{
    //打开文件并判断是否成功
    FILE *fp;
    fp = fopen("saber.bmp", "rb"); // rb 打开一个二进制文件
    if (!fp) {
    printf("BMP Image Not Found!\n");
    exit(0);
    }
    printf("Successfully open the image\n");

    //读取文件头和图像信息头
    fread(&bmfh, 14, 1, fp);
    fread(&bmih, 40, 1, fp);

    //计算图像大小
    pixel_num=bmih.biHeight*bmih.biWidth;

    //创建数组读取位图数据
    PIXEL a[pixel_num];

    fread(&a, sizeof(PIXEL), pixel_num, fp);
    fclose(fp);//关闭该图
    
    int maxY=0;//获取最大Y值
    //RGB->YUV
    YUV b[pixel_num];
    int histogram[256]={0};//记录直方图不同灰度像素数
    int histogramR[256]={0};//记录直方图不同R像素数
    int histogramG[256]={0};//记录直方图不同G像素数
    int histogramB[256]={0};//记录直方图不同B像素数
    
    for(int i=0;i<pixel_num;i++){
        b[i].Y=a[i].red*0.299+a[i].green*0.587+a[i].blue*0.114;
        b[i].U=a[i].red*-0.147+a[i].green*-0.289+a[i].blue*0.435;
        b[i].V=a[i].red*0.615+a[i].green*-0.515+a[i].blue*-0.100;
        //将Y控制在[0,255]
        if(b[i].Y<0)    b[i].Y=0;
        if(b[i].Y>255)  b[i].Y=255;
        if(b[i].Y>maxY) maxY=b[i].Y;
        //记录直方图
        int temp=b[i].Y;
        histogram[temp]++;
        histogramR[a[i].red]++;
        histogramG[a[i].green]++;
        histogramB[a[i].blue]++;

    }
    
    //直方图均衡化操作
    PIXEL new_pixel[pixel_num];
    int s[256]={0};//前i个灰度的像素数量的和
    
    for(int i=0;i<256;i++){
        //通项公式
        if(i==0){
            s[0]=histogram[0];
        }else{
            s[i]=s[i-1]+histogram[i];
        }
        for(int j=0;j<pixel_num;j++){
            int temp=b[j].Y;
            //改变Y值为i的像素为s[i]*1.0/pixel_num*255
            if(temp==i){
                new_pixel[j].red=new_pixel[j].green=new_pixel[j].blue=s[i]*1.0/pixel_num*255;
                
            }
        }
    }

    //将平衡后的灰度图输出
    
    //打开一个bmp空白图像并写入数据
    fp = fopen("histogram_gray.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel, 3, pixel_num, fp);
    fclose(fp);

    //将灰度图转化为彩图输出
    for(int i=0;i<pixel_num;i++){
        //YUV->RGB
        //用转换后的灰度代替b[i].Y
        int temp_red=new_pixel[i].red+1.140*b[i].V;
        int temp_green=new_pixel[i].red-0.395*b[i].U+0.581*b[i].V;
        int temp_blue=new_pixel[i].red+2.000*b[i].U;
        //防止溢出
        if(temp_red>255) temp_red=255;
        if(temp_red<0) temp_red=0;
        if(temp_green>255) temp_green=255;
        if(temp_green<0) temp_green=0;
        if(temp_blue>255) temp_blue=255;
        if(temp_blue<0) temp_blue=0;

        new_pixel[i].red=temp_red;
        new_pixel[i].green=temp_green;
        new_pixel[i].blue=temp_blue;
    }

    //打开一个bmp空白图像并写入数据
    fp = fopen("histogram_color.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel, 3, pixel_num, fp);
    fclose(fp);

    
    //直接对RGB彩色通道进行直方图均衡化操作
    int sR[256]={0};//R通道直方图的积分
    int sG[256]={0};//B通道直方图的积分
    int sB[256]={0};//G通道直方图的积分
    sR[0]=histogramR[0];
    sG[0]=histogramG[0];
    sB[0]=histogramB[0];

    for(int i=0;i<256;i++){
        //通项公式
        if(i!=0){
            sR[i]=sR[i-1]+histogramR[i];
            sG[i]=sG[i-1]+histogramG[i];
            sB[i]=sB[i-1]+histogramB[i];
        }

        //改变RGB值为i的像素为s[i]*1.0/pixel_num*255
        for(int j=0;j<pixel_num;j++){
            if(a[j].red==i){
                new_pixel[j].red=sR[i]*1.0/pixel_num*255;
            }
            if(a[j].green==i){
                new_pixel[j].green=sG[i]*1.0/pixel_num*255;
            }
            if(a[j].blue==i){
                new_pixel[j].blue=sB[i]*1.0/pixel_num*255;
            }
        }
    }

    //将平衡后的彩图输出
    //打开一个bmp空白图像并写入数据
    fp = fopen("histogram_RGB.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel, 3, pixel_num, fp);
    fclose(fp);


    
    //对Y值进行对数增强
    for(int i=0;i<pixel_num;i++){
        //类型会自动转化
        b[i].Y=log(b[i].Y+1)/log(maxY+1)*255;
        //灰度图可以表示为RGB分量都等于Y的图
        new_pixel[i].red=new_pixel[i].green=new_pixel[i].blue=b[i].Y;
    }

    //将增强的灰度图输出
    //打开一个bmp空白图像并写入数据
    fp = fopen("logenhance_gray.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel, 3, pixel_num, fp);
    fclose(fp);

    //将灰度图转化为彩图输出
    for(int i=0;i<pixel_num;i++){
        //YUV->RGB
        int temp_red=b[i].Y+1.140*b[i].V;
        int temp_green=b[i].Y-0.395*b[i].U+0.581*b[i].V;
        int temp_blue=b[i].Y+2.000*b[i].U;
        //防止溢出
        if(temp_red>255) temp_red=255;
        if(temp_red<0) temp_red=0;
        if(temp_green>255) temp_green=255;
        if(temp_green<0) temp_green=0;
        if(temp_blue>255) temp_blue=255;
        if(temp_blue<0) temp_blue=0;

        new_pixel[i].red=temp_red;
        new_pixel[i].green=temp_green;
        new_pixel[i].blue=temp_blue;
    }
    //将增强的彩图输出
    //打开一个bmp空白图像并写入数据
    fp = fopen("logenhance.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel, 3, pixel_num, fp);
    fclose(fp);


}
