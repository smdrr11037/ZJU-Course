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
void translate(PIXEL **a,int x,int y);
void mirror(PIXEL **a,int xory);
void rotation(PIXEL **a, double theta);
void shear(PIXEL **a,double ratio,int xory);
void scale(PIXEL **a,double ratio_h,double ratio_w);
void scale_interpolation(PIXEL **a,double ratio_h,double ratio_w,double sigma);
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

    translate(a,400,200);
    mirror(a,1);//y方向上镜像
    mirror(a,0);//x方向上镜像
    scale(a,1.2,1.8);//长变为1.2倍，宽变为1.8倍
    shear(a,0.6,0);//x方向上剪切，系数0.6
    shear(a,0.2,1);//y方向上剪切，系数0.2
    rotation(a,1);//旋转一弧度
    for (int i=0;i<bmih.biHeight;i++){
        free(a[i]);
    }

    //线性插值法放大gray_test.bmp
    fp = fopen("gray_test.bmp", "rb"); // rb 打开一个二进制文件
    if (!fp) {
    printf("BMP Image Not Found!\n");
    exit(0);
    }
    printf("Successfully open the image\n");

    //读取文件头和图像信息头
    fread(&bmfh, 14, 1, fp);
    fread(&bmih, 40, 1, fp);

    //创建数组读取位图数据
    PIXEL *gray[bmih.biHeight];
    for (int i=0;i<bmih.biHeight;i++){
        gray[i] = (PIXEL*)malloc(sizeof(PIXEL)*bmih.biWidth);
    }
    //PIXEL* a=(PIXEL*)malloc(sizeof(PIXEL)*pixel_num);
    

    //一般来说，.bMP文件的数据从下到上，从左到右存放
    for(int i=bmih.biHeight-1;i>=0;i--){
        fread(gray[i], sizeof(PIXEL), bmih.biWidth, fp);
    }
    
    fclose(fp);//关闭该图
    // scale(gray,3,3);
    scale_interpolation(gray,3,3,0.1);//长宽放大为原来的3倍
    //释放内存
    for (int i=0;i<bmih.biHeight;i++){
        free(gray[i]);
    }
}

void translate(PIXEL **a,int x,int y)
{
    //translate:平移
    //a是存储原图信息的数组指针
    //向右移动x个像素，向下移动y个像素
    //新的信息头
    BITMAPINFOHEADER bmih_tran=bmih;
    //长宽分别加上位移的像素数
    bmih_tran.biWidth=bmih.biWidth+x;
    bmih_tran.biHeight=bmih.biHeight+y;
    //申请空间存储新的图像的数据
    PIXEL *new_pixel[bmih_tran.biHeight];
    for (int i=0;i<bmih_tran.biHeight;i++){
        new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih_tran.biWidth));
    }

    for(int i=0;i<bmih_tran.biHeight;i++){
        for(int j=0;j<bmih_tran.biWidth;j++){
            if(i-y<0||j-x<0){
                //空白部分为灰色，在白色和黑色背景下都能看清
                new_pixel[i][j].blue=new_pixel[i][j].green=new_pixel[i][j].red=200;
            }else{
                //对应位置的像素平移过去
                new_pixel[i][j].blue=a[i-y][j-x].blue;
                new_pixel[i][j].green=a[i-y][j-x].green;
                new_pixel[i][j].red=a[i-y][j-x].red;
            }
        }
    }
    //将图输出
    //打开一个bmp空白图像并写入数据
    FILE *fp;
    fp = fopen("translate.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    //与原来的图相比，文件头相同
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih_tran, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih_tran.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih_tran.biWidth, fp);
    }
    fclose(fp);
    //释放内存
    for (int i=0;i<bmih_tran.biHeight;i++){
        free(new_pixel[i]);
    }
}

void mirror(PIXEL **a,int xory)
{
    //Mirror:x、y方向上镜像图像
    //x方向上mirror
    BITMAPINFOHEADER bmih_mir=bmih;
    FILE *fp;
    if(xory==0){
        bmih_mir.biWidth=bmih.biWidth*2;

        PIXEL *new_pixel[bmih_mir.biHeight];
        for (int i=0;i<bmih_mir.biHeight;i++){
            new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih_mir.biWidth));
        }
        //遍历赋值
        for(int i=0;i<bmih_mir.biHeight;i++){
            for(int j=0;j<bmih_mir.biWidth;j++){
                if(j<bmih.biWidth){
                    //空白部分为灰色，在白色和黑色背景下都能看清
                    new_pixel[i][j].blue=new_pixel[i][j].green=new_pixel[i][j].red=200;
                }else{
                    //对应位置的像素镜像过去
                    new_pixel[i][j].blue=a[i][bmih_mir.biWidth-1-j].blue;
                    new_pixel[i][j].green=a[i][bmih_mir.biWidth-1-j].green;
                    new_pixel[i][j].red=a[i][bmih_mir.biWidth-1-j].red;
                }
            }
        }
        //将图输出
        //打开一个bmp空白图像并写入数据
        fp = fopen("mirror_x.bmp", "wb");
        // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
        fwrite(&bmfh, 14, 1, fp);//文件头
        fwrite(&bmih_mir, 40, 1, fp);//信息头
        //写入位图数据
        for(int i=bmih_mir.biHeight-1;i>=0;i--){
            fwrite(new_pixel[i], 3, bmih_mir.biWidth, fp);
        }
        //fwrite(new_pixel, 3, pixel_num, fp);
        fclose(fp);

        for (int i=0;i<bmih_mir.biHeight;i++){
            //free(a[i]);
            free(new_pixel[i]);
        }
    }else{
        bmih_mir.biHeight=bmih.biHeight*2;

        PIXEL *new_pixel[bmih_mir.biHeight];
        for (int i=0;i<bmih_mir.biHeight;i++){
            new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih_mir.biWidth));
        }
        //遍历赋值
        for(int i=0;i<bmih_mir.biHeight;i++){
            for(int j=0;j<bmih_mir.biWidth;j++){
                if(i<bmih.biHeight){
                    //空白部分为灰色，在白色和黑色背景下都能看清
                    new_pixel[i][j].blue=new_pixel[i][j].green=new_pixel[i][j].red=200;
                }else{
                    //对应位置的像素镜像过去
                    new_pixel[i][j].blue=a[bmih_mir.biHeight-i-1][j].blue;
                    new_pixel[i][j].green=a[bmih_mir.biHeight-i-1][j].green;
                    new_pixel[i][j].red=a[bmih_mir.biHeight-i-1][j].red;
                }
            }
        }
        //将图输出
        //打开一个bmp空白图像并写入数据
        fp = fopen("mirror_y.bmp", "wb");
        // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
        fwrite(&bmfh, 14, 1, fp);//文件头
        fwrite(&bmih_mir, 40, 1, fp);//信息头
        //写入位图数据
        for(int i=bmih_mir.biHeight-1;i>=0;i--){
            fwrite(new_pixel[i], 3, bmih_mir.biWidth, fp);
        }
        //fwrite(new_pixel, 3, pixel_num, fp);
        fclose(fp);

        for (int i=0;i<bmih_mir.biHeight;i++){
            //free(a[i]);
            free(new_pixel[i]);
        }
    }
}

void rotation(PIXEL **a,double theta)
{
    //Rotation:逆时针旋转theta弧度
    BITMAPINFOHEADER bmih_rot=bmih;
    FILE *fp;
    double min_x=10000;
    double max_x=0;
    double max_y=0;
    double min_y=10000;
    //计算图像长宽
    for(int i=0;i<bmih.biHeight;i++){
        for(int j=0;j<bmih.biWidth;j++){
            //旋转后的坐标
            double x=j*cos(theta)+i*sin(theta);
            double y=j*sin(theta)-i*cos(theta);
            if(x>max_x){
                max_x=x;
            }
            if(x<min_x){
                min_x=x;
            }
            if(y>max_y){
                max_y=y;
            }
            if(y<min_y){
                min_y=y;
            }
        }
    }
    //对齐，每一行必须是四字节的倍数
    if((int)(max_x-min_x)%4==0){
        bmih_rot.biWidth=max_x-min_x;
    }else{
        bmih_rot.biWidth=((int)(max_x-min_x)+4)/4*4;
    }
    if((int)(max_y-min_y)%4==0){
        bmih_rot.biHeight=max_y-min_y;
    }else{
        bmih_rot.biHeight=((int)(max_y-min_y)+4)/4*4;
    }
    
    PIXEL *new_pixel[bmih_rot.biHeight];
    for (int i=0;i<bmih_rot.biHeight;i++){
        new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih_rot.biWidth));
    }

    //赋初值
    for(int i=0;i<bmih_rot.biHeight;i++){
        for(int j=1;j<bmih_rot.biWidth;j++){
            
            new_pixel[i][j].red=200;
            new_pixel[i][j].blue=200;
            new_pixel[i][j].green=200;
            
        }
    }
    //遍历赋值
    for(int i=0;i<bmih.biHeight;i++){
        for(int j=0;j<bmih.biWidth;j++){
            double x=j*cos(theta)+i*sin(theta);
            double y=j*sin(theta)-i*cos(theta);
            int new_i=max_y-y;
            int new_j=x-min_x;
            if(new_i>=bmih_rot.biHeight){
                new_i=bmih_rot.biHeight-1;
            }
            if(new_i<0){
                new_i=0;
            }
            if(new_j>=bmih_rot.biWidth){
                new_j=bmih_rot.biWidth;
            }
            if(new_j<0){
                new_j=0;
            }
            //对应位置的像素旋转过去
            new_pixel[new_i][new_j].blue=a[i][j].blue;
            new_pixel[new_i][new_j].green=a[i][j].green;
            new_pixel[new_i][new_j].red=a[i][j].red;
            
        }
    }

    //遍历，找到每一行的空洞，用前一个元素填充
    for(int i=0;i<bmih_rot.biHeight;i++){
        for(int j=1;j<bmih_rot.biWidth;j++){
            if(new_pixel[i][j].blue==200&&new_pixel[i][j].red==200&&new_pixel[i][j].green==200){
                if(new_pixel[i][j-1].blue!=200||new_pixel[i][j-1].red!=200||new_pixel[i][j-1].green!=200){
                    if(new_pixel[i][j+1].blue!=200||new_pixel[i][j+1].red!=200||new_pixel[i][j+1].green!=200){
                        new_pixel[i][j].red=new_pixel[i][j-1].red;
                        new_pixel[i][j].blue=new_pixel[i][j-1].blue;
                        new_pixel[i][j].green=new_pixel[i][j-1].green;
                    }
                }   
            }
        }
    }
    //将图输出
    //打开一个bmp空白图像并写入数据
    fp = fopen("rotation.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih_rot, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih_rot.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih_rot.biWidth, fp);
    }
    //fwrite(new_pixel, 3, pixel_num, fp);
    fclose(fp);
    

    for (int i=0;i<bmih_rot.biHeight;i++){
        //free(a[i]);
        free(new_pixel[i]);
    }
}

void scale_interpolation(PIXEL **a,double ratio_h,double ratio_w,double sigma)
{
    //插值法求中间像素
    BITMAPINFOHEADER bmih_sca=bmih;
    //求图像大小、对齐
    if((int)(bmih.biHeight*ratio_h)%4==0){
        bmih_sca.biHeight=bmih.biHeight*ratio_h;
    }else{
        bmih_sca.biHeight=((int)(bmih.biHeight*ratio_h)+4)/4*4;
    }
    if((int)(bmih.biWidth*ratio_w)%4==0){
        bmih_sca.biWidth=bmih.biWidth*ratio_w;
    }else{
        bmih_sca.biWidth=((int)(bmih.biWidth*ratio_w)+4)/4*4;
    }
    
    PIXEL *new_pixel[bmih_sca.biHeight];
    for (int i=0;i<bmih_sca.biHeight;i++){
        new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih_sca.biWidth));
    }

    //双线性插值
    for(int i=0;i<bmih_sca.biHeight;i++){
        for(int j=0;j<bmih_sca.biWidth;j++){            
            double old_j=j/ratio_w;
            double old_i=i/ratio_h;
            //取最邻近的四个像素
            int i0=floor(old_i);
            int j0=floor(old_j);
            int i1=ceil(old_i);
            int j1=ceil(old_j);
            //控制范围
            if(i1>=bmih.biHeight){
                i1=bmih.biHeight-1;
            }
            if(i0<0){
                i0=0;
            }
            if(j1>=bmih.biWidth){
                j1=bmih.biWidth-1;
            }
            if(j0<0){
                j0=0;
            }
            
            //在x轴方向上，对上边和下边的R1和R2两个点进行插值
            double temp_red0=(old_j-j0)*(a[i0][j1].red-a[i0][j0].red)+a[i0][j0].red;
            double temp_red1=(old_j-j0)*(a[i1][j1].red-a[i1][j0].red)+a[i1][j0].red;
            //再根据R1和R2两个点对P插值
            int temp_red=(old_i-i0)*(temp_red1-temp_red0)+temp_red0;
            new_pixel[i][j].blue=new_pixel[i][j].green=new_pixel[i][j].red=temp_red;
        }
    }

    //将图输出
    //打开一个bmp空白图像并写入数据
    FILE *fp;
    fp = fopen("scale_linear.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih_sca, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih_sca.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih_sca.biWidth, fp);
    }
    //fwrite(new_pixel, 3, pixel_num, fp);
    fclose(fp);

    /*
    //RBF插值，基函数：exp(-r^2/sigma);
    double sum=0;
    double* phi[bmih_sca.biHeight];
    for (int i=0;i<bmih_sca.biHeight;i++){
        phi[i]=(double*)malloc(sizeof(double)*bmih_sca.biWidth);
    }
    for(int m=0;m<bmih_sca.biHeight;m++){
        for(int n=0;n<bmih_sca.biWidth;n++){
            double old_m=m/ratio_w;
            double old_n=n/ratio_h;  
            //算出所有像素的插值系数
            for(int i=0;i<bmih.biHeight;i++){
                for(int j=0;j<bmih.biWidth;j++){           
                    phi[i][j]=exp(-((i-old_m)*(i-old_m)+(j-old_n)*(j-old_n))/sigma);
                    sum+=phi[i][j];
                }
            }
            //双精度降低舍入误差
            double temp_Y=0;
            for(int i=0;i<bmih.biHeight;i++){
                for(int j=0;j<bmih.biWidth;j++){          
                    temp_Y+=phi[i][j]/sum*a[i][j].red;
                }
            }
            new_pixel[m][n].blue=new_pixel[m][n].green=new_pixel[m][n].red=temp_Y;
            sum=0;
        }
    }
    fp = fopen("scale_RBF.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih_sca, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih_sca.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih_sca.biWidth, fp);
    }
    //fwrite(new_pixel, 3, pixel_num, fp);
    fclose(fp);
    */
    for (int i=0;i<bmih_sca.biHeight;i++){
        //free(a[i]);
        free(new_pixel[i]);
    }
    /*for (int i=0;i<bmih.biHeight;i++){
        free(phi[i]);
    }
    */
}

void scale(PIXEL **a,double ratio_h,double ratio_w)
{
    //Scale：长宽分别放大一定倍数
    //ratio_h和ratio_w：长和宽的放大（缩小）倍数
    BITMAPINFOHEADER bmih_sca=bmih;
    //对齐
    if((int)(bmih.biHeight*ratio_h)%4==0){
        bmih_sca.biHeight=bmih.biHeight*ratio_h;
    }else{
        bmih_sca.biHeight=((int)(bmih.biHeight*ratio_h)+4)/4*4;
    }
    if((int)(bmih.biWidth*ratio_w)%4==0){
        bmih_sca.biWidth=bmih.biWidth*ratio_w;
    }else{
        bmih_sca.biWidth=((int)(bmih.biWidth*ratio_w)+4)/4*4;
    }
    
    PIXEL *new_pixel[bmih_sca.biHeight];
    for (int i=0;i<bmih_sca.biHeight;i++){
        new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih_sca.biWidth));
    }

    //遍历赋值，使用最近邻插值
    for(int i=0;i<bmih_sca.biHeight;i++){
        for(int j=0;j<bmih_sca.biWidth;j++){
            int old_j=j/ratio_w;
            int old_i=i/ratio_h;

            //控制范围
            if(old_i>=bmih.biHeight){
                old_i=bmih.biHeight-1;
            }
            if(old_i<0){
                old_i=0;
            }
            if(old_j>=bmih.biWidth){
                old_j=bmih.biWidth;
            }
            if(old_j<0){
                old_j=0;
            }
            
            new_pixel[i][j].blue=a[old_i][old_j].blue;
            new_pixel[i][j].green=a[old_i][old_j].green;
            new_pixel[i][j].red=a[old_i][old_j].red;
            
        }
    }

    //将图输出
    //打开一个bmp空白图像并写入数据
    FILE *fp;
    fp = fopen("scale.bmp", "wb");
    // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
    fwrite(&bmfh, 14, 1, fp);//文件头
    fwrite(&bmih_sca, 40, 1, fp);//信息头
    //写入位图数据
    for(int i=bmih_sca.biHeight-1;i>=0;i--){
        fwrite(new_pixel[i], 3, bmih_sca.biWidth, fp);
    }
    //fwrite(new_pixel, 3, pixel_num, fp);
    fclose(fp);

    for (int i=0;i<bmih_sca.biHeight;i++){
        //free(a[i]);
        free(new_pixel[i]);
    }
}

void shear(PIXEL **a,double ratio,int xory)
{
    BITMAPINFOHEADER bmih_she=bmih;
    FILE *fp;
    //x方向上剪切
    if(xory==0){
        //计算图像大小，对齐
        bmih_she.biHeight=bmih.biHeight;
        if((int)(bmih.biWidth+bmih.biHeight*ratio)%4==0){
            bmih_she.biWidth=bmih.biWidth+bmih.biHeight*ratio;
        }else{
            bmih_she.biWidth=((int)(bmih.biWidth+bmih.biHeight*ratio)+4)/4*4;
        }
        
        PIXEL *new_pixel[bmih_she.biHeight];
        for (int i=0;i<bmih_she.biHeight;i++){
            new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih_she.biWidth));
        }

        //遍历赋值，使用最近邻插值
        for(int i=0;i<bmih_she.biHeight;i++){
            for(int j=0;j<bmih_she.biWidth;j++){
                int old_j=j-ratio*i;
                int old_i=i;

                //控制范围
                if(old_i>=bmih.biHeight||old_i<0||old_j>=bmih.biWidth||old_j<0){
                    new_pixel[i][j].blue=200;
                    new_pixel[i][j].green=200;
                    new_pixel[i][j].red=200;  
                }else{
                    new_pixel[i][j].blue=a[old_i][old_j].blue;
                    new_pixel[i][j].green=a[old_i][old_j].green;
                    new_pixel[i][j].red=a[old_i][old_j].red;   

                }
            }
        }
        //将图输出
        //打开一个bmp空白图像并写入数据
        
        fp = fopen("shear_x.bmp", "wb");
        // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
        fwrite(&bmfh, 14, 1, fp);//文件头
        fwrite(&bmih_she, 40, 1, fp);//信息头
        //写入位图数据
        for(int i=bmih_she.biHeight-1;i>=0;i--){
            fwrite(new_pixel[i], 3, bmih_she.biWidth, fp);
        }
        //fwrite(new_pixel, 3, pixel_num, fp);
        fclose(fp);

        for (int i=0;i<bmih_she.biHeight;i++){
            //free(a[i]);
            free(new_pixel[i]);
        }
    }else{
        //y方向上
        //计算图像大小，对齐
        bmih_she.biWidth=bmih.biWidth;
        if((int)(bmih.biHeight+bmih.biWidth*ratio)%4==0){
            bmih_she.biHeight=bmih.biHeight+bmih.biWidth*ratio;
        }else{
            bmih_she.biHeight=((int)(bmih.biHeight+bmih.biWidth*ratio)+4)/4*4;
        }
        
        PIXEL *new_pixel[bmih_she.biHeight];
        for (int i=0;i<bmih_she.biHeight;i++){
            new_pixel[i] = (PIXEL*)malloc(sizeof(PIXEL)*(bmih_she.biWidth));
        }

        //遍历赋值，使用最近邻插值
        for(int i=0;i<bmih_she.biHeight;i++){
            for(int j=0;j<bmih_she.biWidth;j++){
                int old_j=j;
                int old_i=i-ratio*j;

                //控制范围
                if(old_i>=bmih.biHeight||old_i<0||old_j>=bmih.biWidth||old_j<0){
                    new_pixel[i][j].blue=200;
                    new_pixel[i][j].green=200;
                    new_pixel[i][j].red=200;  
                }else{
                    new_pixel[i][j].blue=a[old_i][old_j].blue;
                    new_pixel[i][j].green=a[old_i][old_j].green;
                    new_pixel[i][j].red=a[old_i][old_j].red;   

                }
            }
        }

        //将图输出
        //打开一个bmp空白图像并写入数据
        fp = fopen("shear_y.bmp", "wb");
        // fwrite(数据块首地址,元素大小, 元素个数,文件指针)
        fwrite(&bmfh, 14, 1, fp);//文件头
        fwrite(&bmih_she, 40, 1, fp);//信息头
        //写入位图数据
        for(int i=bmih_she.biHeight-1;i>=0;i--){
            fwrite(new_pixel[i], 3, bmih_she.biWidth, fp);
        }
        //fwrite(new_pixel, 3, pixel_num, fp);
        fclose(fp);
        //释放内存
        for (int i=0;i<bmih_she.biHeight;i++){
            //free(a[i]);
            free(new_pixel[i]);
        }
        
    }  
}