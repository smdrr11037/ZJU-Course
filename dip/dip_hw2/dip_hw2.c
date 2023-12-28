#include <stdio.h>
#include <stdlib.h>
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

//腐蚀函数
int Erosion(int i,PIXEL a[]){
    //flag为返回值，默认为0（黑色）
    //有一个像素不为黑色，则返回255（白色）
    int flag=0;
    //正方形结构元
    /*for(int m=-1;m<2;m++){//m为当前像素相对中心像素的行
        for(int n=-1;n<2;n++){//m为当前像素相对中心像素的列
            int subscript=i+m*bmih.biWidth+n;
            //判断下标是否合法
            if(subscript<0||subscript>=pixel_num){
                continue;
            }
            //有一个像素不为黑色，则返回255（白色）
            if(a[subscript].red==255){
                flag=255;
                break;
            }
        }
    }*/
    //十字形结构元
    //中心像素上面的像素
    int subscript=i-bmih.biWidth;
    if(subscript<0||subscript>=pixel_num){
        //下标不合法，不做任何事
    }else if(a[subscript].red==255){
            flag=255;
        }
    
    //和中心像素一排的像素
    for(int n=-1;n<2;n++){//m为当前像素相对中心像素的列
        subscript=i+n;
        //判断下标是否合法
        if(subscript<0||subscript>=pixel_num){
            continue;
        }
        if(a[subscript].red==255){
            flag=255;
            break;
        }
    }

    //中心像素下面的像素
    subscript=i+bmih.biWidth;
    if(subscript<0||subscript>=pixel_num){
        //下标不合法，不做任何事
    }else if(a[subscript].red==255){
            flag=255;
        }
    
    return flag;
}

//膨胀函数
int Dilation(int i,PIXEL a[]){
    //flag为返回值，默认为255（白色）
    //有一个像素为黑色，则返回0（黑色）
    int flag=255;
    
    //十字形结构元
    int subscript=i-bmih.biWidth;
    if(subscript<0||subscript>=pixel_num){
        //下标不合法，不做任何事
    }else if(a[subscript].red==0){
            flag=0;
        }
    
    for(int n=-1;n<2;n++){//m为当前像素相对中心像素的列
        subscript=i+n;
        //判断下标是否合法
        if(subscript<0||subscript>=pixel_num){
            continue;
        }
        if(a[subscript].red==0){
            flag=0;
            break;
        }
    }
    subscript=i+bmih.biWidth;
    if(subscript<0||subscript>=pixel_num){
        //下标不合法，不做任何事
    }else if(a[subscript].red==0){
            flag=0;
        }
    
    return flag;
}

int main()
{
    //打开文件并判断是否成功
    FILE *fp;
    fp = fopen("monokuma.bmp", "rb"); // rb 打开一个二进制文件
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

    //RGB->YUV
    int min=255;
    int max=0;
    YUV b[pixel_num];
    for(int i=0;i<pixel_num;i++){
        b[i].Y=a[i].red*0.299+a[i].green*0.587+a[i].blue*0.114;
        b[i].U=a[i].red*-0.147+a[i].green*-0.289+a[i].blue*0.435;
        b[i].V=a[i].red*0.615+a[i].green*-0.515+a[i].blue*-0.100;
        //将Y控制在[0,255]
        if(b[i].Y<0)    b[i].Y=0;
        if(b[i].Y>255)  b[i].Y=255;
        //找到最小、大的Y值
        if(b[i].Y<min)  min=b[i].Y;
        if(b[i].Y>max)  max=b[i].Y;
    }
    
    //全局大津法算threshold
    int threshold;
    double variance=0;
    //遍历找到最大方差
    for(int i=1;i<max-min;i++){
        int fore_num=0;//计数前景像素数
        double fore_Y=0;//统计前景灰度总和
        int back_num=0;//计数后景像素数
        double back_Y=0;//统计后景灰度总和
        for(int j=0;j<pixel_num;j++){
            //Y值<threshold(min+i),前景像素数++，反之后景++
            if(b[j].Y<min+i){
                back_num++;
                back_Y+=b[j].Y;
            }else{
                fore_num++;
                fore_Y+=b[j].Y;
            }
        }
        //大津算法计算方差
        double temp=0;
        temp=fore_num*back_num*1.0/pixel_num/pixel_num*
        (fore_Y/fore_num-back_Y/back_num)*(fore_Y/fore_num-back_Y/back_num);
        //令方差最大时的灰度值为threshold
        if(temp>variance){
            variance=temp;
            threshold=min+i;
        }
    }
    PIXEL new_pixel[pixel_num];//建立数组存储二值图数据  
    for(int i=0;i<pixel_num;i++){
        if(b[i].Y<threshold){
            new_pixel[i].red=new_pixel[i].green=new_pixel[i].blue=0;
        }else{
            new_pixel[i].red=new_pixel[i].green=new_pixel[i].blue=255;
        }
    }
    
    //打开一个bmp空白图像并写入数据
    fp = fopen("binary.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel, 3, pixel_num, fp);
    fclose(fp);

    //滑动窗口的局部大津法二值化
    //初始化方差
    variance=0;
    //设置小窗口的长宽和重合值
    int width=199;
    int height=60;
    int overlap=10;

    for(int m=0;m<bmih.biHeight;m+=height-overlap){//m表示窗口的第一个像素在第几行
        for(int n=0;n<bmih.biWidth;n+=width-overlap){//n表示窗口的第一个像素在第几列
            for(int i=1;i<max-min;i++){
                int fore_num=0;//计数前景像素数
                double fore_Y=0;//统计前景灰度总和
                int back_num=0;//计数后景像素数
                double back_Y=0;//统计后景灰度总和
                for(int k=0;k<height;k++){//当前像素在小窗口的第k行
                    for(int j=0;j<width;j++){//像素在小窗口的第j列
                        //Y值<threshold(min+i),前景像素数++，反之后景++
                        //计算当前像素值的下标
                        int subscript=m*bmih.biWidth+n+j+k*bmih.biWidth;
                        //判断下标是否合法
                        if(subscript<0||subscript>=pixel_num){
                            continue;
                        }
                        if(b[subscript].Y<min+i){
                            back_num++;
                            back_Y+=b[subscript].Y;
                        }else{
                            fore_num++;
                            fore_Y+=b[subscript].Y;
                        }
                    }
                }
            //大津算法计算方差
                double temp=0;
                temp=fore_num*back_num*1.0/width/height/width/height*
                (fore_Y/fore_num-back_Y/back_num)*(fore_Y/fore_num-back_Y/back_num);
                if(temp>variance){
                    variance=temp;
                    threshold=min+i;
                }
            }
            for(int k=0;k<height;k++){//当前像素在小窗口的第k行
                for(int j=0;j<width;j++){//像素在小窗口的第j列
                    //Y值<threshold(min+i),前景像素数++，反之后景++
                    //计算当前像素值的下标
                    int subscript=m*bmih.biWidth+n+j+k*bmih.biWidth;
                    //判断下标是否合法
                    if(subscript<0||subscript>=pixel_num){
                        continue;
                    }
                    //二值化
                    if(b[subscript].Y<threshold){
                        new_pixel[subscript].red=new_pixel[subscript].green=new_pixel[subscript].blue=0;
                    }else{
                        new_pixel[subscript].red=new_pixel[subscript].green=new_pixel[subscript].blue=255;
                    }
                }
            }
            variance=0;//将方差重新置为0
        }
    }
    
    //打开一个bmp空白图像并写入数据
    fp = fopen("binary_local.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel, 3, pixel_num, fp);
    fclose(fp);
}
