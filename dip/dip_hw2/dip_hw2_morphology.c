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
    fp = fopen("binary_local.bmp", "rb"); // rb 打开一个二进制文件
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

    //新的数组存储新的位图数据
    PIXEL new_pixel[pixel_num];
    PIXEL new_pixel2[pixel_num];

    //腐蚀操作
    for(int i=0;i<pixel_num;i++){//遍历每个像素
        new_pixel[i].blue=new_pixel[i].green=new_pixel[i].red=Erosion(i,a);
    }
    //先腐蚀后膨胀——开操作
    for(int i=0;i<pixel_num;i++){//遍历每个像素
        new_pixel2[i].blue=new_pixel2[i].green=new_pixel2[i].red=Dilation(i,new_pixel);
    }

    //打开一个bmp空白图像并写入数据
    fp = fopen("erosion.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel, 3, pixel_num, fp);
    fclose(fp);

    //打开一个bmp空白图像并写入数据
    fp = fopen("opening.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel2, 3, pixel_num, fp);
    fclose(fp);


    //膨胀操作
    for(int i=0;i<pixel_num;i++){//遍历每个像素
        new_pixel[i].blue=new_pixel[i].green=new_pixel[i].red=Dilation(i,a);
    }
    //先膨胀后腐蚀——闭操作
    for(int i=0;i<pixel_num;i++){//遍历每个像素
        new_pixel2[i].blue=new_pixel2[i].green=new_pixel2[i].red=Erosion(i,new_pixel);
    }

    //打开一个bmp空白图像并写入数据
    fp = fopen("dilation.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel, 3, pixel_num, fp);
    fclose(fp);

    //打开一个bmp空白图像并写入数据
    fp = fopen("closing.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih, 40, 1, fp);//信息头
    //写入位图数据
    fwrite(&new_pixel2, 3, pixel_num, fp);
    fclose(fp);
    
}
