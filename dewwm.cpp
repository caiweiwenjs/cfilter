#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "watermark.h"

/************************************************************************/
/* 坐标结构体                                                           */
/************************************************************************/
struct Corrnidate  
{
	int row;
	int col;
};

void dct(float*outImage,unsigned char *inImage,int inImageRow,int inImageCol);
void idct(unsigned char*outImage,float*inImage,int inImageRow,int inImageCol);
void dctBlock(float*outImage,const float *inImage,int inImageRow,int inImageCol,int M,int N);
void idctBlock(float *outImage,const float*inImage,int inImageRow,int inImageCol,int M,int N);
void Arnold(unsigned char *outImage,unsigned char*inImage,int N,int times);
void floyd_halftone(unsigned char *outImage,const unsigned char *inImage,int inImageRow,int inImageCol);
void Gauss_DBS(float *P,int N);
void xcorr2(float *outImage,const float*inImageA,const float*inImageB,int inImageARow,int inImageACol,int inImageBRow,int inImageBCol);
void conv2(float *outImage,const float *inImageA,const float *inImageB,int inImageARow,int inImageACol,int inImageBRow,int inImageBCol,const char *type);
void rot90(float *outImage,const float *inImage,int inImageRow,int inImageCol);
void filter2(float *outImage,const float *inImageA,const float *inImageB,int inImageARow,int inImageACol,int inImageBRow,int inImageBCol,const char *type);
float std2(float *inImage,int inImageRow,int inImageCol);
void rgb2gray(unsigned char *outImage,const unsigned char *rImage,const unsigned char *gImage,unsigned char *bImage,int Row,int Col);
float erf(float x);
struct Corrnidate zig_zag(int x);
void updateO(float *outImage,const float *inImage,int i,int j,int ii,int jj,int a0,int a1,int imageRow,int imageCol);
void updateCep(float *outImage,const float *Cep, const float *Cpp,int m0x,int m0y,int m1x,int m1y,int a0,int a1,int CepRow,int CepCol,int CppRow,int CppCol);
void dbs(unsigned char *halftoneImage,const unsigned char *grayImage,int imageRow,int imageCol);
void get_pixel_change(int *N,const unsigned char* halftone, const unsigned char *I_gray,const unsigned char *watermark,int I_grayRow,int I_grayCol,int watermarkRow,int watermarkCol,int blockM,int blockN);
void new_flip_pixel(unsigned char *outImage,unsigned char *halftoneImage,const int *N,int imageRow,int imageCol,int blockRow,int blockCol);
void extractWatermark(unsigned char *outImage,unsigned char *inImage,int imageRow,int imageCol,int blockRow,int blockCol);
void embedWatermark(unsigned char *outImage,const unsigned char *grayImage,const unsigned char *watermark,int imageRow,int imageCol,int watermarkRow,int watermarkCol,int blockRow,int blockCol);

/************************************************************************/
/* 问题总结文档20130328
1.dew算法的彩色图像处理已经测试通过;
2.一般编译器的栈空间的大小为1M,dbs方法需要的图片比较大，开的空间比较大，
测试的时候可以都采用堆空间（问题已经解决！）*/
/************************************************************************/

/************************************************************************/
/* 字符常量定义                                                         */
/************************************************************************/

const float PI=3.1415926;
const float E=2.71828;
const float eps=1e-6;



/************************************************************************/
/*  DCT变化的程序  (测试成功)                                           */
/************************************************************************/
void dct(float*outImage,unsigned char *inImage,int inImageRow,int inImageCol)
{
	int i,j,u,v;
	float parameterA,parameterB;
	float temp;
	for (i=0;i<inImageRow;i++)
		for (j=0;j<inImageCol;j++)
		{

			if (i==0)
			{
				parameterA=sqrt(1.0/inImageRow);
			}
			else
			{
				parameterA=sqrt(2.0/inImageRow);
			}
			
			if (j==0)
			{
				parameterB=sqrt(1.0/inImageCol);
			}
			else
			{
				parameterB=sqrt(2.0/inImageCol);
			}

			temp=0;
			for(u=0;u<inImageRow;u++)
				for (v=0;v<inImageCol;v++)
				{
					temp=temp+*((inImage+u*inImageCol)+v)*cos((2*u+1)*i*PI/(2*inImageRow))*cos((2*v+1)*j*PI/(2*inImageCol)); 
				}
			*((outImage+i*inImageCol)+j)=parameterA*parameterB*temp;
			
	}

		return ;
}


/************************************************************************/
/*  IDCT变化的程序
	outImage表示IDCT变换到的图像;
	inImage表示输入的原始图像;
	inImageRow表示图像的行数；
	inImageCol表示图像的列数*/
/************************************************************************/
void idct(unsigned char*outImage,float*inImage,int inImageRow,int inImageCol)
{
	int i,j,u,v;
	float parameterA,parameterB;
	float temp;
	for (i=0;i<inImageRow;i++)
		for (j=0;j<inImageCol;j++)
		{
			temp=0;
			for(u=0;u<inImageRow;u++)
				for (v=0;v<inImageCol;v++)
				{
					if (u==0)
					{
						parameterA=sqrt(1.0/inImageRow);
					}
					else
					{
						parameterA=sqrt(2.0/inImageRow);
					}
					
					if (v==0)
					{
						parameterB=sqrt(1.0/inImageCol);
					}
					else
					{
						parameterB=sqrt(2.0/inImageCol);
			}

					temp=temp+*((inImage+u*inImageCol)+v)*cos((2*i+1)*u*PI/(2*inImageRow))*cos((2*j+1)*v*PI/(2*inImageCol))*parameterA*parameterB; 
				}
				*((outImage+i*inImageCol)+j)=(unsigned char)(temp+0.5);
				
		}
		
		return ;
}




/************************************************************************/
/*  分块DCT变化的程序 
	outImage表示DCT变换输出的图像；
	inImage表示输入的原始图像;
	inImageRow表示图像的行数；
	inImageCol表示图像的列数；
	M表示分块大小的行数；
	N表示分块大小的列数*/
/************************************************************************/
void dctBlock(float*outImage,const float *inImage,int inImageRow,int inImageCol,int M,int N)
{
	int i,j,u,v,p,q,uw,vw,pw,qw;
	float parameterA,parameterB;
	float temp;
	//判断是否能够分块处理;
	if (inImageRow%M || inImageCol%N)
	{
		printf("不满足分块条件!\n");
		return;
	}
	for (i=0;i<inImageRow;i=i+M)
		for (j=0;j<inImageCol;j=j+N)
			for(p=i,pw=0;p<i+M;p++,pw++)
				for(q=j,qw=0;q<j+N;q++,qw++)
					{
						if (pw==0)
						{ 
							parameterA=sqrt(1.0/M);
						}
						else
						{
							parameterA=sqrt(2.0/M);
						}
			
						if (qw==0)
						{
							parameterB=sqrt(1.0/N);
						}
						else
						{
							parameterB=sqrt(2.0/N);
						}
			
						temp=0;
						for(u=i,uw=0;u<i+M;u++,uw++)
							for (v=j,vw=0;v<j+N;v++,vw++)
							{
								temp=temp+*((inImage+u*inImageCol)+v)*cos((2*uw+1)*pw*PI/(2*M))*cos((2*vw+1)*qw*PI/(2*N)); 
							}
						*((outImage+p*inImageCol)+q)=parameterA*parameterB*temp;
				
					}
			
				return ;
}


/************************************************************************/
/*  IDCT变化的程序
	outImage表示输出的图像；
	inImage表示原始输入的图像；
	inImageRow表示图像的行数;
	inImageCol表示图像的列数;
	M表示分块大小的行数;
	N表示分块大小的列数*/
/************************************************************************/
void idctBlock(float *outImage,const float*inImage,int inImageRow,int inImageCol,int M,int N)
{
	int i,j,u,v,uw,vw,p,q,pw,qw;
	float parameterA,parameterB;
	float temp;
	for (i=0;i<inImageRow;i=i+M)
		for (j=0;j<inImageCol;j=j+N)
			for(p=i,pw=0;p<i+M;p++,pw++)
				for (q=j,qw=0;q<j+N;q++,qw++)
			
					{
						temp=0;
						for(u=i,uw=0;u<i+M;u++,uw++)
							for (v=j,vw=0;v<j+N;v++,vw++)
							{
								if (uw==0)
								{
									parameterA=sqrt(1.0/M);
								}
								else
								{
									parameterA=sqrt(2.0/M);
								}
					
								if (vw==0)
								{
									parameterB=sqrt(1.0/N);
								}
								else
								{
									parameterB=sqrt(2.0/N);
								}
					
								temp=temp+*((inImage+u*inImageCol)+v)*cos((2*pw+1)*uw*PI/(2*M))*cos((2*qw+1)*vw*PI/(2*N))*parameterA*parameterB; 
							}
							*((outImage+p*inImageCol)+q)=temp;
				
			}
		
		return ;
}




/************************************************************************/
/* Arnold置乱程序
   outImage表示置乱之后的图像；
   inImage表示初始的图像；
   N表示图像的大小;
   times表示置乱的次数*/
/************************************************************************/
void Arnold(unsigned char *outImage,unsigned char*inImage,int N,int times)
{
	int i=0;
	int v;
	int u;
	int uw,vw;
	memcpy(outImage,inImage,sizeof(unsigned char)*N*N);
	while (i<times)
	{
		for(u=0;u<N;u++)
			for (v=0;v<N;v++)
			{
				uw=(u+v)%N;
				vw=(u+2*v)%N;
				*(outImage+uw*N+vw)=*(inImage+u*N+v);
			}
			i++;

			memcpy(inImage,outImage,sizeof(unsigned char)*N*N);
	}
	return ;

}


/************************************************************************/
/* floyd_halftone函数 （测试通过）     
   outImage表示floyd半色调之后的图像;
   inImage表示原始灰度图像；
   inImageRow表示图像的行数;
   inImageCol表示图像的列数*/
/************************************************************************/

void floyd_halftone(unsigned char *outImage,const unsigned char *inImage,int inImageRow,int inImageCol)
{
	int i,j;
	float e;
	float *tempImage=(float *)malloc(sizeof(float)*(inImageCol+2)*(inImageRow+2)); //临时变量;
	memset(tempImage,0,sizeof(float)*(inImageCol+2)*(inImageRow+2));
	for(i=0;i<inImageRow;i++)
		for(j=0;j<inImageCol;j++)
		{
			*(tempImage+(i+1)*(inImageCol+2)+(j+1))=(float)*(inImage+i*inImageCol+j);
		}

	for(i=0;i<inImageRow;i++)
		for (j=0;j<inImageCol;j++)
		{
			if (*(tempImage+(i+1)*(inImageCol+2)+(j+1))>127)
			{
				*(outImage+i*inImageCol+j)=255;
			}
			else
			{
				*(outImage+i*inImageCol+j)=0;
			}

			e=*(tempImage+(i+1)*(inImageCol+2)+(j+1))-*(outImage+i*inImageCol+j);
            *(tempImage+(i+1)*(inImageCol+2)+(j+1)+1)+=(float)((7.0/16.0)*e);
			*(tempImage+(i+1+1)*(inImageCol+2)+(j+1)-1)+=(float)((3.0/16.0)*e);
			*(tempImage+(i+1+1)*(inImageCol+2)+(j+1))+=(float)((5.0/16.0)*e);
			*(tempImage+(i+1+1)*(inImageCol+2)+(j+1)+1)+=(float)((1.0/16.0)*e);
		}


		free(tempImage);
		return;

}


/************************************************************************/
/* 高斯滤波器函数
   P表示高斯滤波器的矩阵;
   N表示要求的高斯滤波器的大小*/
/************************************************************************/
void Gauss_DBS(float *P,int N)
{
	
	int siz,i,j;
	int *x=(int *)malloc(sizeof(int)*N*N);
	int *y=(int *)malloc(sizeof(int)*N*N);
	float sum=0.0;
	float maxP=0.0;
	siz=(N-1)/2;
	for(i=0;i<N;i++)
	{
		for(j=0;j<N;j++)
		{
			*(x+i*N+j)=(i-3)*(i-3);
	
		}
	}

	for(i=0;i<N;i++)
	{
		for(j=0;j<N;j++)
		{
			*(y+j*N+i)=(i-3)*(i-3);
		}
	}

	for(i=0;i<N;i++)
	{
		for(j=0;j<N;j++)
		{
			*(P+i*N+j)=exp((-((*(x+i*N+j))+*(y+i*N+j)))/(1.3*1.3));
			sum=sum+*(P+i*N+j);
			if (maxP<*(P+i*N+j))
			{
				maxP=*(P+i*N+j);
			}
			
		}
	}
		for(i=0;i<N;i++)
			for(j=0;j<N;j++)
			{
				if (*(P+i*N+j)<eps*maxP)
				{
					*(P+i*N+j)=0.0;
				}

				*(P+i*N+j)=*(P+i*N+j)/sum;

			}

	return;
}


/************************************************************************/
/* xcorr2函数(矩阵的相关过程求解)   测试成功      
   outImage表示两个矩阵求相关运算的结果；
   inImageA表示输入的矩阵A；
   inImageB表示输入的矩阵B；
   inImageARow表示矩阵A的行数；
   inImageARow表示矩阵A的列数;
   inImageBRow表示矩阵B的行数；
   inImageBCol表示矩阵B的列数*/
/************************************************************************/
void xcorr2(float *outImage,const float*inImageA,const float*inImageB,int inImageARow,int inImageACol,int inImageBRow,int inImageBCol)
{
	int i,j,Row,Col,istart,jstart,p,q,pw,qw;
	float *tempImage,temp;
	Row=inImageARow+inImageBRow-1;
	Col=inImageACol+inImageBCol-1;
	tempImage=(float *)malloc(sizeof(float)*Row*Col);//临时矩阵;
	memset(tempImage,0,sizeof(float)*Row*Col);
	for(i=0;i<inImageARow;i++)
		for (j=0;j<inImageACol;j++)
		{
			*(tempImage+i*Row+j)=*(inImageA+i*inImageARow+j);

		}

	for(i=0;i<Row;i++)
		for (j=0;j<Col;j++)
		{
		
			istart=(inImageBRow-1-i>=0)?inImageBRow-1-i:0;
			jstart=(inImageBCol-1-j>=0)?inImageBCol-1-j:0;
			
			temp=0;
			for (p=istart;p<inImageBRow;p++)
				for(q=jstart;q<inImageBCol;q++)
				{	
					pw=p+i-(inImageBRow-1);
					qw=q+j-(inImageBCol-1);
					temp=temp+*(inImageB+p*inImageBRow+q)*(*(tempImage+pw*Row+qw));
				}

				*(outImage+i*Row+j)=temp;

				
		}


		free(tempImage);
		return;
}



/************************************************************************/
/* 实现二维离散卷积函数(conv2)  （测试成功）   
	outImage表示连个矩阵卷积之后的结果矩阵;
	inImageA表示输入的矩阵A；
	inImageB表示输入的矩阵B；
	inImageARow表示矩阵A的行数;
	inImageACol表示矩阵A的列数;
	inImageBRow表示矩阵B的行数;
	inImageBCol表示矩阵B的列数;
	type:"full","same"
	核心矩阵为矩阵B，与Matlab保持一致*/
/************************************************************************/
void conv2(float *outImage,const float *inImageA,const float *inImageB,int inImageARow,int inImageACol,int inImageBRow,int inImageBCol,const char *type)
{
	int i,j,p,q,Row,Col,centerRow,centerCol;
	float *tempInImageA,*tempInImageB,*tempOutImage;
	float temp,B;
	Row=inImageARow+inImageBRow-1;
	Col=inImageACol+inImageBCol-1;
	//扩展inImageA为tempInImageA;
	tempInImageA=(float *)malloc(sizeof(float)*Row*Col);
	memset(tempInImageA,0,sizeof(float)*Row*Col);
	for(i=0;i<inImageARow;i++)
		for (j=0;j<inImageACol;j++)
		{
			*(tempInImageA+i*Col+j)=*(inImageA+i*inImageACol+j);
	
		}

	//扩展inImageB为tempInImageB；
	tempInImageB=(float*)malloc(sizeof(float)*Row*Col);
	memset(tempInImageB,0,sizeof(float)*Row*Col);
	for(i=0;i<inImageBRow;i++)
		for (j=0;j<inImageBCol;j++)
		{
			*(tempInImageB+i*Col+j)=*(inImageB+i*inImageBCol+j);
			
		}
	
    //计算卷积的过程;
	tempOutImage=(float *)malloc(sizeof(float)*Row*Col);
	memset(tempOutImage,0,sizeof(float)*Row*Col);

	for(i=0;i<Row;i++)
		for (j=0;j<Col;j++)
		{
			temp=0.0;
			for(p=0;p<Row;p++)
				for (q=0;q<Col;q++)
				{
					B=((i-p)>=0&&(j-q)>=0)?(*(tempInImageB+(i-p)*Col+(j-q))):0;
					
					temp=temp+(*(tempInImageA+p*Col+q))*B;
				}

				*(tempOutImage+i*Col+j)=temp;
		}



	if (!strcmp("full",type))
	{
		for(i=0;i<Row;i++)
			for (j=0;j<Col;j++)
				*(outImage+i*Col+j)=*(tempOutImage+i*Col+j);					
	}else if (!strcmp("same",type))
	{
		centerRow=inImageBRow/2;
		centerCol=inImageBCol/2;
	
		for(i=0;i<inImageARow;i++)
			for (j=0;j<inImageACol;j++)
			{
				*(outImage+i*inImageACol+j)=(*(tempOutImage+(i+centerRow)*Col+j+centerCol));
			}

	}
	else
	{
		printf("类型错误！！\n");
	
	}

	free(tempInImageA);
	free(tempInImageB);
	free(tempOutImage);

	return;


}

/************************************************************************/
/* 逆时针旋转90度函数 （测试成功）                                     */
/************************************************************************/
void rot90(float *outImage,const float *inImage,int inImageRow,int inImageCol)
{
	int i,j;
	float *tempInImage;
	int maxN;
	maxN=(inImageRow>inImageCol)?inImageRow:inImageCol;
	tempInImage=(float *)malloc(sizeof(float)*maxN*maxN);
	memset(tempInImage,0,sizeof(float)*maxN*maxN);
	//扩展inImage矩阵为tempInImage矩阵；
	for(i=0;i<inImageRow;i++)
		for(j=0;j<inImageCol;j++)
			*(tempInImage+i*maxN+j)=*(inImage+i*inImageCol+j);

	//第一步，矩阵转置
		for(i=0;i<maxN;i++)
			for (j=i+1;j<maxN;j++)
			{
				*(tempInImage+i*maxN+j)=*(tempInImage+i*maxN+j)+*(tempInImage+j*maxN+i);
				*(tempInImage+j*maxN+i)=*(tempInImage+i*maxN+j)-*(tempInImage+j*maxN+i);
				*(tempInImage+i*maxN+j)=*(tempInImage+i*maxN+j)-*(tempInImage+j*maxN+i);
			}

	//第二步，上下互换;

		for(i=0;i<maxN/2;i++)
			for (j=0;j<maxN;j++)
			{
				*(tempInImage+i*maxN+j)=*(tempInImage+i*maxN+j)+*(tempInImage+(maxN-1-i)*maxN+j);
				*(tempInImage+(maxN-1-i)*maxN+j)=*(tempInImage+i*maxN+j)-*(tempInImage+(maxN-1-i)*maxN+j);
				*(tempInImage+i*maxN+j)=*(tempInImage+i*maxN+j)-*(tempInImage+(maxN-1-i)*maxN+j);
			}	
		
	//第三步，赋值;
		for(i=0;i<inImageCol;i++)
			for(j=0;j<inImageRow;j++)
			{
				*(outImage+i*inImageRow+j)=*(tempInImage+i*maxN+j);

			}

			free(tempInImage);

		return ;

}


/************************************************************************/
/* 滤波器函数filter2 遵循matlab的规律,第一个矩阵为核心矩阵（测试成功) 
   outImage表示滤波器的输出矩阵;
   inImageA表示输入的矩阵A；
   inImageB表示输入的矩阵B；
   inImageARow表示矩阵A的行数；
   inImageACol表示矩阵A的列数;
   inImageBRow表示矩阵B的行数;
   inImageBCol表示矩阵B的列数;
   type="full","same"
   核心矩阵为矩阵A；与Matlab保持一致*/
/************************************************************************/
void filter2(float *outImage,const float *inImageA,const float *inImageB,int inImageARow,int inImageACol,int inImageBRow,int inImageBCol,const char *type)
{
	//首先旋转180度核心矩阵，然后调用conv2函数实现滤波器函数
	float *tempInImageRot90,*tempInImageRot180;
	tempInImageRot90=(float *)malloc(sizeof(float)*inImageACol*inImageARow);
	memset(tempInImageRot90,0,sizeof(float)*inImageACol*inImageARow);
	rot90(tempInImageRot90,inImageA,inImageARow,inImageACol);
	tempInImageRot180=(float*)malloc(sizeof(float)*inImageARow*inImageACol);
	memset(tempInImageRot180,0,sizeof(float)*inImageACol*inImageARow);
	rot90(tempInImageRot180,tempInImageRot90,inImageACol,inImageARow);
	//调用卷积函数;
	conv2(outImage,inImageB,tempInImageRot180,inImageBRow,inImageBCol,inImageARow,inImageACol,type);
	free(tempInImageRot90);
	free(tempInImageRot180);
	return ;
}



/************************************************************************/
/* std2()函数求解矩阵的标准差
	inImage表示输入的矩阵
	inImageRow表示图像矩阵的行数
	inImageCol表示图像矩阵的列数;
	返回值为float类型数据表示得到的矩阵的标准差   
    （测试成功  注意：标准差的N=N-1）;真奇葩啊！                       */
/************************************************************************/
float std2(float *inImage,int inImageRow,int inImageCol)
{
	
	int i,j;
	float average;
	float sum;
	float sum2;
	//计算平均值；
	sum=0.0;
	for(i=0;i<inImageRow;i++)
		for (j=0;j<inImageCol;j++)
			sum=sum+*(inImage+i*inImageCol+j);
	average=sum/(inImageRow*inImageCol);
	
	//计算平方和
	sum2=0;
	for(i=0;i<inImageRow;i++)
		for (j=0;j<inImageCol;j++)
			sum2=sum2+(*(inImage+i*inImageCol+j)-average)*(*(inImage+i*inImageCol+j)-average);

	return (sqrt(sum2/(inImageRow*inImageCol-1)));

}


/************************************************************************/
/* rgb2gray()函数将彩色图像转换为灰度图像
	outImage表示输出的灰度图像;
	rImage表示彩色图像的红色通道的数据;
	gImage表示彩色图像的绿色通道的数据;
	bImage表示彩色图像的蓝色通道的数据;
	Row表示图像的行数;
	Col表示图像的列数;
	测试成功;				                                    */
/************************************************************************/

void rgb2gray(unsigned char *outImage,const unsigned char *rImage,const unsigned char *gImage,unsigned char *bImage,int Row,int Col)
{
	int i,j;
	for(i=0;i<Row;i++)
		for (j=0;j<Col;j++)
			*(outImage+i*Col+j)=0.2989*(*(rImage+i*Col+j))+0.5870*(*(gImage+i*Col+j))+0.1140*(*(bImage+i*Col+j))+0.5;

	return;
}




/************************************************************************/
/* 标准正态分布函数erf()     
    测试成功！
	实质是求积分*/
/************************************************************************/
float erf(float x)
{
	int n;//循环次数，决定积分的步长;
	float ga;//步长;
	float t;//积分变量；
	float sum;
	sum=0.0;
	ga=x/10000.0;
	n=0;
	t=0;
	while(n<10001)
	{
		sum=sum+exp(-t*t)*ga;
		t=t+ga;	
		n++;
	}

	return ((2/sqrt(PI))*sum>1?1.0:(2/sqrt(PI))*sum);

}


/************************************************************************/
/* zig-zag坐标转换函数，适用于8*8分块的情况
                                                                     */
/************************************************************************/
struct Corrnidate zig_zag(int x)
{
	int i,j;
	struct Corrnidate y;
	int a[8][8]={{0, 1, 5, 6,14,15,27,28},
	{2, 4, 7,13,16,26,29,42},
	{3, 8,12,17,25,30,41,43},
	{9,11,18,24,31,40,44,53},
	{10,19,23,32,39,45,52,54},
	{20,22,33,38,46,51,55,60},
	{21,34,37,47,50,56,59,61},
	{35,36,48,49,57,58,62,63}};
	x=x>64?63:x;
	for(i=0;i<8;i++)
		for (j=0;j<8;j++)
		{
			if (x==a[i][j])
			{
				y.row=i;
				y.col=j;
				return(y);
			}
		}
	 
}



/************************************************************************/
/* updateO函数实现 (测试完成，注意matlab与c语言数组开始下标不一样，
 matlab从1开始，c语言从0开始)  测试成功                                 */
/************************************************************************/

void updateO(float *outImage,const float *inImage,int i,int j,int ii,int jj,int a0,int a1,int imageRow,int imageCol)
{
	memcpy(outImage,inImage,sizeof(float)*imageRow*imageCol);
	if (ii==i && jj==j)
	{
		*(outImage+i*imageCol+j)=*(outImage+i*imageCol+j)+a0*1;
	} 
	else
	{
		*(outImage+i*imageCol+j)=*(outImage+i*imageCol+j)+a0*1;
		*(outImage+ii*imageCol+jj)=*(outImage+ii*imageCol+jj)+a1*1;
	}

	return;
	
}


/************************************************************************/
/* updateCep函数实现(测试成功)                                          */
/************************************************************************/

void updateCep(float *outImage,const float *Cep, const float *Cpp,int m0x,int m0y,int m1x,int m1y,int a0,int a1,int CepRow,int CepCol,int CppRow,int CppCol)
{
	int i,j,p0,mstart,mend,nstart,nend;
	float Cpp_m0,Cpp_m1;
	memcpy(outImage,Cep,sizeof(float)*CepRow*CepCol);
	p0=(CppRow+1)/2-1;
	if (1+m0x<1+m1x)
	{
		mstart=1+m0x-p0;
		mend=CppRow+m1x-p0;
	} 
	else
	{
		mstart=1+m1x-p0;
		mend=CppRow+m0x-p0;
	}

	if (1+m0y<1+m1y)
	{
		nstart=1+m0y-p0;
		nend=CppCol+m1y-p0;
	} 
	else
	{
		nstart=1+m1y-p0;
		nend=CppCol+m0y-p0;
	}
	
	if (mend>CepRow-1)
	{
		mend=CepRow-1;
	}
	if (nend>CepCol-1)
	{
		nend=CepCol-1;
	}

	if (mstart<0)
	{
		mstart=0;
	}
	if (nstart<0)
	{
		nstart=0;	
	}
	

	for(i=mstart;i<=mend;i++)//循环开始
		for (j=nstart;j<=nend;j++)
		{
			if (i-m0x+p0<0 || j-m0y+p0<0 || i-m0x+p0>CppRow-1 || j-m0y+p0>CppCol-1)
			{
				Cpp_m0=0;	
			
			} 
			else
			{
				Cpp_m0=*(Cpp+(i-m0x+p0)*CppCol+(j-m0y+p0));
			}


			if (i-m1x+p0<0 || j-m1y+p0<0 || i-m1x+p0 >CppRow-1 || j-m1y+p0 >CppCol-1)
			{
				Cpp_m1=0;

			} 
			else
			{

				Cpp_m1=*(Cpp+(i-m1x+p0)*CppCol+(j-m1y+p0));
			}

			*(outImage+i*CepCol+j)=*(outImage+i*CepCol+j)-a0*Cpp_m0-a1*Cpp_m1;
			
		}
		
	
		return;
	
}


/************************************************************************/
/* dbs半调实现过程  halftone表示初始半色调图像,gray表示灰度图像   
imageRow 表示图像的行数，inmageCol表示图像的列数 (测试成功)             */
/************************************************************************/
void dbs(unsigned char *halftoneImage,const unsigned char *grayImage,int imageRow,int imageCol)
{
	//变量的定义以及初始化过程
	int i,j,ii,jj,istart,jstart,iend,jend;
	int a0,a1;
	int PN;//表示滤波器的大小；
	int flagNum;
	float *tempHalftoneImage,*tempGrayImage,*eImage;
	float *P;//表示高斯滤波器，为了和matlab程序中的名称一致;
	float *Cpp;//表示P的自相关矩阵;
	float *Cep;//表示eImage与P矩阵的互相关矩阵;
	float  E;//初始的E;
	float  E1;
	float  E0;
	float  E_temp_end;
	float  E_temp;
	float *tempHalftoneImage_temp_end;
	float *tempHalftoneImage_temp;
	float *Cep_temp_end;
	float *E_matric;
	tempHalftoneImage=(float*)malloc(sizeof(float)*imageRow*imageCol);
	tempGrayImage=(float*)malloc(sizeof(float)*imageRow*imageCol);
	eImage=(float *)malloc(sizeof(float)*imageRow*imageCol);
	tempHalftoneImage_temp_end=(float *)malloc(sizeof(float)*imageRow*imageCol);
	tempHalftoneImage_temp=(float *)malloc(sizeof(float)*imageRow*imageCol);
	Cep_temp_end=(float *)malloc(sizeof(float)*imageRow*imageCol);


	//图像归一化除以255
	for(i=0;i<imageRow;i++)
	{
		for (j=0;j<imageCol;j++)
		{
			*(tempHalftoneImage+i*imageCol+j)=(float)(*(halftoneImage+i*imageCol+j)/255.0);
			*(tempGrayImage+i*imageCol+j)=(float)(*(grayImage+i*imageCol+j)/255.0);
			*(eImage+i*imageCol+j)=*(tempGrayImage+i*imageCol+j)-*(tempHalftoneImage+i*imageCol+j);//灰度图像减去半调图像;
		}
	
	}
	//得到高斯滤波器;
	PN=7;
	P=(float *)malloc(sizeof(float)*PN*PN);
	Gauss_DBS(P,PN);
	//得到Cpp矩阵;
	Cpp=(float *)malloc(sizeof(float)*(2*PN-1)*(2*PN-1));
	xcorr2(Cpp,P,P,PN,PN,PN,PN);

	//得到Cep矩阵;
	Cep=(float *)malloc(sizeof(float)*(imageRow)*(imageCol));
	filter2(Cep,Cpp,eImage,(2*PN-1),(2*PN-1),imageRow,imageCol,"same");
	//得到E_matric；
	E_matric=(float *)malloc(sizeof(float)*(imageRow+PN-1)*(imageCol+PN-1));
	filter2(E_matric,P,eImage,PN,PN,imageRow,imageCol,"full");
	//得到E;
	E=0.0;
	for(i=0;i<imageRow+PN-1;i++)
	{
		for (j=0;j<imageCol+PN-1;j++)
		{
			E=E+*(E_matric+i*(imageCol+PN-1)+j)*(*(E_matric+i*(imageCol+PN-1)+j));

		}
		
	}

	E1=E;
	E0=10000000;
    flagNum=0;
	while(E1<E0)//whileStart
	{
		for(i=0;i<imageRow;i++)
			for (j=0;j<imageCol;j++)//外层循环开始;
			{
				E_temp_end=E;
				memcpy(tempHalftoneImage_temp_end,tempHalftoneImage,sizeof(float)*imageRow*imageCol);
				memcpy(Cep_temp_end,Cep,sizeof(float)*imageRow*imageCol);

				if (i>=1 &&i<=imageRow-2 && j>=1 &&j<=imageCol-2) //中间部分的元素;
				{
					istart=i-1;
					iend=i+1;
					jstart=j-1;
					jend=j+1;
				}
				
				if (i==0 && j>=1&&j<=imageCol-2)// 第一行中间部分；
				{
					istart=i;
					iend=i+1;
					jstart=j-1;
					jend=j+1;
				}

				if (i==imageRow-1 && j>=1&&j<=imageCol-2)//最后一行中间部分
				{
					istart=i-1;
					iend=i;
					jstart=j-1;
					jend=j+1;

				}

				if (i>=1 && i<=imageRow-2 &&j==0)//第一列中间部分
				{
					istart=i-1;
					iend=i+1;
					jstart=j;
					jend=j+1;

				}
				
				if (i>=1 && i<=imageRow-2 &&j==imageCol-1)//最后一列中间部分
				{
					istart=i-1;
					iend=i+1;
					jstart=j-1;
					jend=j;
				}
				
				if (i==0 && j==0) //第一行第一个元素;
				{
					istart=i;
					iend=i+1;
					jstart=j;
					jend=j+1;
				}
	            
				if (i==0 &&j==imageCol-1) //第一行最后一个元素;
				{
					istart=i;
					iend=i+1;
					jstart=j-1;
					jend=j;
				}
				
				if (i==imageRow-1 && j==0) //最后一行第一个元素;
				{
					istart=i-1;
					iend=i;
					jstart=j;
					jend=j+1;

				}
				
				if (i==imageRow-1 && j==imageCol-1)//最后一行最后一个元素
				{
					istart=i-1;
					iend=i;
					jstart=j-1;
					jend=j;
				}

				for(ii=istart;ii<=iend;ii++)//内层循环开始
					for (jj=jstart;jj<=jend;jj++)
					{
						
						memcpy(tempHalftoneImage_temp,tempHalftoneImage,sizeof(float)*imageRow*imageCol);
						if (ii==i && jj==j)//toggle
						{
							if (*(tempHalftoneImage_temp+i*imageCol+j)==0)
							{
								a0=0;
								a1=0;
							} 
							else
							{
								a0=0;
								a1=0;
							}
						} 
						else//swap
						{	
							a0=(int)(*(tempHalftoneImage_temp+ii*imageCol+jj)-*(tempHalftoneImage_temp+i*imageCol+j));
							a1=(int)(*(tempHalftoneImage_temp+i*imageCol+j)-*(tempHalftoneImage_temp+ii*imageCol+jj));							
						}
					
						E_temp=(a0*a0+a1*a1)*(*(Cpp+(PN-1)*(2*PN-1)+(PN-1)))-2*a0*(*(Cep+i*imageCol+j))-2*a1*(*(Cep+ii*imageCol+jj))+2*a0*a1*(*(Cpp+(ii-i+PN-1)*(2*PN-1)+(jj-j+PN-1)));
						E_temp=E+E_temp;
						if (E_temp<0)
						{
							return;
						}

						if (E_temp<E_temp_end)
						{
							E_temp_end=E_temp;
							updateO(tempHalftoneImage_temp_end,tempHalftoneImage,i,j,ii,jj,a0,a1,imageRow,imageCol);
							updateCep(Cep_temp_end,Cep,Cpp,i,j,ii,jj,a0,a1,imageRow,imageCol,(2*PN-1),(2*PN-1));

						}

					}//内层循环结束;


				E=E_temp_end;
				memcpy(tempHalftoneImage,tempHalftoneImage_temp_end,sizeof(float)*imageRow*imageCol);
				memcpy(Cep,Cep_temp_end,sizeof(float)*imageRow*imageCol);

			}//外层循环结束;
			

		
			E0=E1;
			E1=E;
			flagNum++;
		}//whileEnd

	for(i=0;i<imageRow;i++)
		for (j=0;j<imageCol;j++)
		{
			*(halftoneImage+i*imageCol+j)=(unsigned char)*(tempHalftoneImage+i*imageCol+j)*255;
		}


		free(tempHalftoneImage);
		free(tempHalftoneImage_temp);
		free(tempHalftoneImage_temp_end);
		free(tempGrayImage);
		free(eImage);
		free(Cep_temp_end);
		free(P);
		free(Cep);
		free(Cpp);

		return;
}

/************************************************************************/
/* 嵌入水印的第一步，DCT域嵌入水印，得到嵌入水印之后的特征矩阵M_modify  
   N表示输出的结果矩阵，表示特征矩阵嵌入水印之前与嵌入水印之后的差值;
   I_gray表示初始灰度图像;
   watermark表示要嵌入的水印图像;
   I_grayRow，I_grayCol表示灰度图像的行和列;
   watermarkRow,watermarkCol表示水印图像的行和列;
   blockM,blockN表示图像分块大小，分别表示分块的行和列;
   测试成功201303142042不容易，各种类型出问题*/
/************************************************************************/

void get_pixel_change(int *N,const unsigned char* halftone, const unsigned char *I_gray,const unsigned char *watermark,int I_grayRow,int I_grayCol,int watermarkRow,int watermarkCol,int blockM,int blockN)
{
	//初始化以及定义变量的部分；
	float *M ,*M_modify;
	float *M_halftone;
	float *C;
	int i,j,p,q,ii,jj,flag;//临时变量;
	int MRow,MCol;
	float H;//调节参数;
	float K;//嵌入的水印强度;
	float para;
	H=255.0;
	K=20;
	MRow=I_grayRow/blockM; //特征矩阵的行;
	MCol=I_grayCol/blockN; //特征矩阵的列;
	//分块统计每一块的平均值，得到初始的特征矩阵;
	M=(float *)malloc(sizeof(float)*MRow*MCol);
	M_modify=(float *)malloc(sizeof(float)*MRow*MCol);
	M_halftone=(float *)malloc(sizeof(float)*MRow*MCol);
	memset(M,0,sizeof(float)*MRow*MCol);
	memset(M_modify,0,sizeof(float)*MRow*MCol);
	memset(M_halftone,0,sizeof(float)*MRow*MCol);

	for(i=0,ii=0;i<I_grayRow;i=i+blockM,ii++)//外层循环start
		for (j=0,jj=0;j<I_grayCol;j=j+blockN,jj++)
		{
			for(p=i;p<i+blockM;p++)//内层循环start；
				for (q=j;q<j+blockN;q++)
				{
					*(M+ii*MCol+jj)=*(M+ii*MCol+jj)+(*(I_gray+p*I_grayCol+q)/255.0);
					*(M_halftone+ii*MCol+jj)=*(M_halftone+ii*MCol+jj)+(*(halftone+p*I_grayCol+q)/255.0);
				//	printf("%.lf   ",*(M+ii*MCol+jj));

				}//内层循环end;

				*(M+ii*MCol+jj)=(*(M+ii*MCol+jj)/(float)(blockM*blockN))*H;//得到块内的均值；
				*(M_halftone+ii*MCol+jj)=(*(M_halftone+ii*MCol+jj)/(float)(blockM*blockN))*H;
			//	printf("%lf   ",*(M+ii*MCol+jj));

		}//外层循环end

	//对特征矩阵进行分块DCT变化
	C=(float *)malloc(sizeof(float)*MRow*MCol);
//	C_modify=(float *)malloc(sizeof(float)*MRow*MCol);
	dctBlock(C,M,MRow,MCol,8,8);

	//嵌入水印
	flag=0;
	for(i=0;i<MRow;i=i+8)//外层循环start;
		for (j=0;j<MCol;j=j+8)
		{
			if (*(watermark+flag)==1)
			{
				*(C+(i+7)*MCol+(j+7))=K;
			} 
			else
			{
				*(C+(i+7)*MCol+(j+7))=-K;
			}//(8,8)

			flag=flag+1;

			if (*(watermark+flag)==1)
			{
				*(C+(i+7)*MCol+(j+6))=K;
			} 
			else
			{
				*(C+(i+7)*MCol+(j+6))=-K;
			}//(8,7)


			flag=flag+1;
			if (*(watermark+flag)==1)
			{
				*(C+(i+6)*MCol+(j+7))=K;
			} 
			else
			{
				*(C+(i+6)*MCol+(j+7))=-K;
			}//(7,8)


			flag=flag+1;

			if (*(watermark+flag)==1)
			{
				*(C+(i+6)*MCol+(j+6))=K;
			} 
			else
			{
				*(C+(i+6)*MCol+(j+6))=-K;
			}//(7,7)

			
			flag=flag+1;



		}//外层循环end;

	//对嵌入水印的特征矩阵进行IDCT变化，得到修改之后的特征矩阵
	idctBlock(M_modify,C,MRow,MCol,8,8);

    //做差求出输出矩阵N 
	for(i=0;i<MRow;i++)
		for (j=0;j<MCol;j++)
		{
			para=(*(M_halftone+i*MCol+j)>*(M_modify+i*MCol+j))?0.5:-0.5;
			*(N+i*MCol+j)=(int)(((*(M_halftone+i*MCol+j)-*(M_modify+i*MCol+j))/H)*blockM*blockN+para);

		}


		free(M);
		free(M_halftone);
		free(M_modify);
		free(C);


		return;
	
}
/************************************************************************/
/* new_flip_pixel()函数的实现
 outImage表示翻转之后的图像;
 halftoneImage表示初始的半色调图像
 N表示要反转的矩阵
 imageRow imageCol表示图像的行数和列数
 blockRow,blockCol表示分块的块的行数和列数	
 测试成功201303142345							*/
/************************************************************************/

void new_flip_pixel(unsigned char *outImage,unsigned char *halftoneImage,const int *N,int imageRow,int imageCol,int blockRow,int blockCol)
{
	int i,j,ii,jj,p,q;
	int aviableNum;
	int flagNum;
	int NRow,NCol;
	NRow=imageRow/blockRow;
	NCol=imageCol/blockCol;
	memcpy(outImage,halftoneImage,sizeof(unsigned char)*imageRow*imageCol);
	for(i=0,ii=0;i<imageRow;i=i+blockRow,ii++)//外层循环start
		for (j=0,jj=0;j<imageCol;j=j+blockCol,jj++)
		{
		//	for(p=i;p<i+blockRow;p++)//内层循环start
		//		for (q=j;q<j+blockCol;q++)
		//		{
					aviableNum=0;
					if (*(N+ii*NCol+jj)>0)//表示需要把白色像素变成黑色像素(1-->0)
					{
						//1.计算可反转的像素的个数；
						for(p=i;p<i+blockRow;p++)
							for (q=j;q<j+blockCol;q++)
							{
								if (*(outImage+p*imageCol+q)/255==1)
								{
									aviableNum++;
								}
									
							}//计算可反转像素结束;

						if (aviableNum<=*(N+ii*NCol+jj))//需要全都反转过来
						{
							for(p=i;p<i+blockRow;p++)
								for (q=j;q<j+blockCol;q++)
								{
									if (*(outImage+p*imageCol+q)/255==1)
									{
										*(outImage+p*imageCol+q)=0;
									}
									
								}
						} 
						else//需要选择性的反转一些像素;
						{	
							flagNum=*(N+ii*NCol+jj);
							//随机选择还是顺序选择反转的像素
							for(p=i;p<i+blockRow && flagNum>0;p++)
								for (q=j;q<j+blockCol;q++)
								{
									if (*(outImage+p*imageCol+q)/255==1)
									{
										*(outImage+p*imageCol+q)=0;
										flagNum--;
									}
									if (flagNum<=0)
									{
										break;
									}
									
								}
						}
								
					} //(1-->0 end)
					if(*(N+ii*NCol+jj)<0)//表示需要把黑色像素变成白色像素(0-->1)
					{
						
						//1.计算可反转的像素的个数；
						for(p=i;p<i+blockRow;p++)
							for (q=j;q<j+blockCol;q++)
							{
								if (*(outImage+p*imageCol+q)==0)
								{
									aviableNum++;
								}
								
							}//计算可反转像素结束;
							
							if (aviableNum<=-(*(N+ii*NCol+jj)))//需要全都反转过来
							{
								for(p=i;p<i+blockRow;p++)
									for (q=j;q<j+blockCol;q++)
									{
										if (*(outImage+p*imageCol+q)==0)
										{
											*(outImage+p*imageCol+q)=255;
										}
										
									}
							} 
							else//需要选择性的反转一些像素;
							{	
								flagNum=-(*(N+ii*NCol+jj));
								//随机选择还是顺序选择反转的像素
								for(p=i;p<i+blockRow &&flagNum>0;p++)
									for (q=j;q<j+blockCol;q++)
									{
										if (*(outImage+p*imageCol+q)==0)
										{
											*(outImage+p*imageCol+q)=255;
											flagNum--;
										}
										if (flagNum<=0)
										{
											break;
										}
										
									}
						}						

					}//(0-->1 end)

			//	}//内层循环end
		}//外层循环end

		return;

}

/************************************************************************/
/* extractWatermark函数提取水印的函数;
   outImage表示提取的水印图像;
   inImage表示输入的含有水印的图像；
   imageRow表示图像的行数;
   imageCol表示图像的列数；
   blockRow表示分块的行数;
   blockCol表示分块的列数；*/
/************************************************************************/
void extractWatermark(unsigned char *outImage,unsigned char *inImage,int imageRow,int imageCol,int blockRow,int blockCol)
{
	//初始化以及定义变量的部分；
	float *M;
	float *C;
	int i,j,p,q,ii,jj,flag;//临时变量;
	int MRow,MCol;
	float H;//调节参数;
	H=255.0;
	MRow=imageRow/blockRow; //特征矩阵的行;
	MCol=imageCol/blockCol; //特征矩阵的列;
	//分块统计每一块的平均值，得到初始的特征矩阵;
	M=(float *)malloc(sizeof(float)*MRow*MCol);
	memset(M,0,sizeof(float)*MRow*MCol);
	for(i=0;i<imageRow;i++)
		for (j=0;j<imageCol;j++)
			*(inImage+i*imageCol+j)=*(inImage+i*imageCol+j)/255;

	

	
	for(i=0,ii=0;i<imageRow;i=i+blockRow,ii++)//外层循环start
		for (j=0,jj=0;j<imageCol;j=j+blockCol,jj++)
		{
			for(p=i;p<i+blockRow;p++)//内层循环start；
				for (q=j;q<j+blockCol;q++)
				{
					*(M+ii*MCol+jj)=*(M+ii*MCol+jj)+*(inImage+p*imageCol+q);		
				}//内层循环end;
				
				*(M+ii*MCol+jj)=(*(M+ii*MCol+jj)/(float)(blockRow*blockCol))*H;//得到块内的均值；
				
		}//外层循环end
		
		//对特征矩阵进行分块DCT变化
		C=(float *)malloc(sizeof(float)*MRow*MCol);
		dctBlock(C,M,MRow,MCol,8,8);
		
		//提取水印
		flag=0;
		for(i=0;i<MRow;i=i+8)//外层循环start;
			for (j=0;j<MCol;j=j+8)
			{
				if (*(C+(i+7)*MCol+(j+7))>0)
				{
					*(outImage+flag)=1;
				} 
				else
				{
					*(outImage+flag)=0;
				}//(8,8)
				
				flag=flag+1;
				
				if (*(C+(i+7)*MCol+(j+6))>0)
				{
					*(outImage+flag)=1;
				} 
				else
				{
					*(outImage+flag)=0;
				}//(8,7)
				
				
				flag=flag+1;
				if (*(C+(i+6)*MCol+(j+7))>0)
				{
					*(outImage+flag)=1;
				} 
				else
				{
					*(outImage+flag)=0;
				}//(7,8)
				
				
				flag=flag+1;
				
				if (*(C+(i+6)*MCol+(j+6))>0)
				{
					*(outImage+flag)=1;
				} 
				else
				{
					*(outImage+flag)=0;
				}//(7,7)
				
				
				flag=flag+1;
				
				
				
		}//外层循环end;

			free(M);
			free(C);
			return;
}


/************************************************************************/
/* embedWatermark()函数表示水印嵌入函数
outImage表示输出的含有水印的图像;
grayImage表示初始的灰度图像；
watermark表示水印图像矩阵;
imageRow表示图像的行数；
imageCol表示图像的列数;
watermarkRow表示水印的行数;
watermarkCol表示水印的列数;
blockRow表示分块的行数;
blockCol表示分块的列数;	*/
/************************************************************************/
void embedWatermark(unsigned char *outImage,const unsigned char *grayImage,const unsigned char *watermark,int imageRow,int imageCol,int watermarkRow,int watermarkCol,int blockRow,int blockCol)
{
	unsigned char *halftoneImage;
	int middleRow;//中间图的行数;
	int middleCol;//中间图的列数；
	int *N;//表示每一个分块反转的个数；
	unsigned char *tempDbs,*tempGray;
	int i,j,p,q;
	
	middleRow=imageRow/blockRow;
	middleCol=imageCol/blockCol;
	//对初始的图像进行半色调;
	halftoneImage=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
	floyd_halftone(halftoneImage,grayImage,imageRow,imageCol);
	N=(int *)malloc(sizeof(int)*middleRow*middleCol);
	get_pixel_change(N,halftoneImage,grayImage,watermark,imageRow,imageCol,watermarkRow,watermarkCol,blockRow,blockCol);
	new_flip_pixel(outImage,halftoneImage,N,imageRow,imageCol,blockRow,blockCol);
	//分块dbs处理;
	tempDbs=(unsigned char *)malloc(sizeof(char)*blockRow*blockCol);
	tempGray=(unsigned char *)malloc(sizeof(char)*blockRow*blockCol);
	for(i=0;i<imageRow;i=i+blockRow)
		for (j=0;j<imageCol;j=j+blockCol)
		{
			for(p=i;p<i+blockRow;p++)
			{
				for (q=j;q<j+blockCol;q++)
				{
					
					*(tempDbs+(p-i)*blockCol+(q-j))=*(outImage+p*imageCol+q);
					*(tempGray+(p-i)*blockCol+(q-j))=*(grayImage+p*imageCol+q);
					
				}
				
			}
			
			dbs(tempDbs,tempGray,blockRow,blockCol);
			for(p=i;p<i+blockRow;p++)
				for (q=j;q<j+blockCol;q++)
				{
					*(outImage+p*imageCol+q)=*(tempDbs+(p-i)*blockCol+(q-j));
					
				}			
		}

		free(tempDbs);
		free(tempGray);
		free(halftoneImage);
		return;
}




/************************************************************************/
/* dewembed()函数采用dew算法在彩色图像中嵌入水印的方法
	inImage 表示原始的载体图像和嵌水印之后的图像;
	watermark表示水印数据;
	imageRow表示载体图像的行数;
	imageCol表示载体图像的列数；
	waterRow表示水印图像的行数;
	waterCol表示水印图像的列数;
	ifColor表示载体图像是否为彩色图像，
	如果为彩色图像ifColor==1,如果是灰度图像ifColor==0
	ifAlign表示载体图像的数据是否为4字节对齐形式，ifAlign==1表示是四字节对齐
	ifAlign==0表示不是四字节对齐*/
/************************************************************************/
void dewWatermarkEmbed(unsigned char *inImage, unsigned char *watermark,int imageRow,int imageCol,int waterRow,int waterCol,unsigned char ifColor,unsigned char ifAlign)
{
	int i,j,p,q;//临时变量
	int index;  //水印的位数;
	float f; //纹理负责度;
	int c;//截止坐标;
	int imageRowTrue;//不论是否四字节对齐，该变量始终等于图像的行数;
	int imageColTrue;//当图像不是四字节对齐的时候，等于图像的列数，如果图像时四字节对齐的话，得计算;
	int waterRowTrue;
	int waterColTrue;

	unsigned char* inImageR;//如果载体图像是彩色图像，用来存储图像的红色通道的数据;
	unsigned char* inImageG;//如果载体图像是彩色图像，用来存储图像的绿色通道的数据；
	unsigned char* inImageB;//如果载体图像是彩色图像，用来存储图像的蓝色通道的数据;
	unsigned char* inImageGray;//如果载体图像时灰度图像，用来存储载体图像;
	unsigned char* watermarkTemp;//临时存储水印的信息;


	float *tempR;//彩色图像时，用来临时存储红色通道的一个8*8分块的数据;
	float *tempB;//彩色图像时，用来临时存储蓝色通道的一个8*8分块的数据;
	float *dctR;//彩色图像时，用来临时存储红色通道的一个8*8分块的DCT变换系数;
	float *dctB;//彩色图像时，用来临时存储蓝色通道的一个8*8分块的DCT变换系数;
	float *idctR;//彩色图像时，用来临时存储红色通道的一个8*8分块的IDCT变换系数;
	float *idctB;//彩色图像时，用来临时存储蓝色通道的一个8*8分块的IDCT变换系数;

	float *tempFirst;//灰度图像时，用来存储嵌水印分块对的第一个8*8分块;
	float *tempSecond;//灰度图像时，用来存储嵌水印分块对的第二个8*8分块;
	float *dctFirst;//灰度图像时，用来存储嵌水印分块对的第一个8*8分块的DCT变换的系数;
	float *dctSecond;//灰度图像时，用来存储嵌水印分块对的第二个8*8分块的DCT变换的系数;
	float *idctFirst;//灰度图像时，用来存储嵌水印分块对的第一个8*8分块的IDCT变换的系数;
	float *idctSecond;//灰度图像时，用来存储嵌水印分块对的第二个8*8分块的IDCT变换的系数;

	unsigned char *iGray;//彩色图像时，用来保存彩色图像转换为灰度图像的数据;
	float *tempIGray;//彩色图像时，用来保存转换为灰度图像的一个8*8分块的数据;
	float theta;//求解c时候的参数；
	struct Corrnidate y;//用于进行坐标的转换;
	int flag;//标志位；
	theta=4.5;//参数值；
	index=0;//嵌水印的位数;

	//首先根据水印图像是否是四字节对齐,获得水印的实际数据信息;
	if (ifAlign==1) // 表示水印图像的数据采用的是四字节对齐的存储方式,读取时候一定要注意读取的方式
	{
		//计算真实图像的行数核列数;
		waterRowTrue=waterRow; //对齐字节的行数;行数不需要改变；
		waterColTrue=((waterCol+3)/4)*4;//对齐字节的列数;
	} 
	else//表示水印图像的数据不是四字节对齐的存储方式，直接读取就OK了。
	{
		//计算真实的图像的行数和列数；
		waterRowTrue=waterRow;
		waterColTrue=waterCol;
	}
	watermarkTemp=(unsigned char *)malloc(sizeof(unsigned char)*waterRowTrue*waterColTrue);
	//提取水印数据；
	for(i=0;i<waterRowTrue;i++)
		for (j=0;j<waterColTrue;j++)
		{
			*(watermarkTemp+i*waterColTrue+j)=*(watermark+i*waterColTrue+j); //读取水印图像的数据；
		}
	//对灰度的水印图像进行半色调处理；
		floyd_halftone(watermark,watermarkTemp,waterRowTrue,waterColTrue);
	//对水印之后的图像变换成01的数据;
	for(i=0;i<waterRow;i++)
		for (j=0;j<waterCol;j++)
		{
			*(watermark+i*waterCol+j)=*(watermark+i*waterCol+j)/255; //读取水印图像的数据；
		}

	if (ifColor==1)//表示载体图像时彩色图像，进行彩色图像的嵌水印处理
	{

		inImageR=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		inImageG=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		inImageB=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);


		if (ifAlign==1) // 表示载体图像的数据采用的是四字节对齐的存储方式,读取时候一定要注意读取的方式
		{
			//计算真实图像的行数核列数;
			imageRowTrue=imageRow; //对齐字节的行数;行数不需要改变；
			imageColTrue=((imageCol*3+3)/4)*4;//对齐字节的列数;
		} 
		else//表示载体图像的数据不是四字节对齐的存储方式，直接读取就OK了。
		{
			//计算真实的图像的行数和列数；
			imageRowTrue=imageRow;
			imageColTrue=imageCol*3;
		}

		//提取图像的数据的RGB三个通道的数据;
		for(i=0;i<imageRowTrue;i++)
			for (j=0;j<=imageColTrue-3;j=j+3)
			{

				*(inImageR+i*imageCol+j/3)=*(inImage+i*imageColTrue+j);//读取红色通道的数据;
				*(inImageG+i*imageCol+j/3)=*(inImage+i*imageColTrue+j+1);//读取绿色通道的数据;
				*(inImageB+i*imageCol+j/3)=*(inImage+i*imageColTrue+j+2);//读取蓝色通道的数据；
			}


		//上面部分得到了图像数据的红绿蓝三个通道的数据;下面开始嵌入水印;
		//从彩色图像到灰度图像的转换;
		iGray=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		rgb2gray(iGray,inImageR,inImageG,inImageB,imageRow,imageCol);//三色图像转换为灰度图像存储在iGray中，用于求解纹理复杂度;
		//嵌入水印的过程;
		dctR=(float *)malloc(sizeof(float)*8*8);
		dctB=(float *)malloc(sizeof(float)*8*8);
		idctR=(float *)malloc(sizeof(float)*8*8);
		idctB=(float *)malloc(sizeof(float)*8*8);
		tempR=(float *)malloc(sizeof(float)*8*8);
		tempB=(float *)malloc(sizeof(float)*8*8);
		tempIGray=(float *)malloc(sizeof(float)*8*8);

		for(i=0;i<=imageRow-8;i=i+8)//如果载体图像的行数和列数不能被8整除，怎会抛弃最右边和最下面的满足8*8大小的块;
			for (j=0;j<=imageCol-8;j=j+8)
			{
				//拷贝数据到临时的存储分块单元;
				for(p=i;p<i+8;p++)
					for (q=j;q<j+8;q++)
					{
						*(tempR+(p-i)*8+(q-j))=*(inImageR+p*imageCol+q);//红色通道的分块数据;
						*(tempB+(p-i)*8+(q-j))=*(inImageB+p*imageCol+q);//蓝色通道的分块数据;
						*(tempIGray+(p-i)*8+(q-j))=(float)*(iGray+p*imageCol+q);//灰度图像的分块数据;
						
					}

					//分别做DCT变换；
					dctBlock(dctR,tempR,8,8,8,8);
					dctBlock(dctB,tempB,8,8,8,8);
					//求解c的过程;
					f=std2(tempIGray,8,8);
					c=18*0.5*(1+erf(f/theta/sqrt(2.0)))+0.5;

					//如果要嵌入的水印容量小于载体图片的最大水印容量，则表示水印嵌入已经完成，从此返回;
					if (index>=waterRowTrue*waterColTrue)
					{
						//将数据写入到返回的彩色图像中;
						for(i=0;i<imageRowTrue;i++)
							for (j=0;j<=imageColTrue-3;j=j+3)
							{
								*(inImage+i*imageColTrue+j)=*(inImageR+i*imageCol+j/3);//写入红色通道的数据;
								*(inImage+i*imageColTrue+j+1)=*(inImageG+i*imageCol+j/3);//写入绿色通道的数据;
								*(inImage+i*imageColTrue+j+2)=*(inImageB+i*imageCol+j/3);//写入蓝色通道的数据；
							}	
						return;
					}


					//嵌入水印的过程;
					if (*(watermark+index)==1)//当水印为1时的操作;
					{
						//令红色通道的能量为红色通道的能量加上蓝色通道的能量；蓝色通道的能量值设置为0;
						for (flag=c;flag<64;flag++)
						{
							y=zig_zag(flag);//求出正常的坐标值;
							*(dctR+y.row*8+y.col)=(*(dctR+y.row*8+y.col))*(*(dctB+y.row*8+y.col))>=0?(*(dctR+y.row*8+y.col))+(*(dctB+y.row*8+y.col)):(*(dctR+y.row*8+y.col))-(*(dctB+y.row*8+y.col));
							*(dctB+y.row*8+y.col)=0;

						}

					} 
					else//当水印为0时的操作;
					{
						//令蓝色通道的能量为红色通道的能量加上蓝色通道的能量；红色通道的能量值设置为0;
						for (flag=c;flag<64;flag++)
						{
							y=zig_zag(flag);//求出正常的坐标值;
							*(dctB+y.row*8+y.col)=(*(dctB+y.row*8+y.col))*(*(dctR+y.row*8+y.col))>=0?(*(dctR+y.row*8+y.col))+(*(dctB+y.row*8+y.col)):(*(dctB+y.row*8+y.col))-(*(dctR+y.row*8+y.col));
							*(dctR+y.row*8+y.col)=0;

						}

					}

					index++;//水印自加，完成一位水印的嵌入过程;
					//进行IDCT变换;
					idctBlock(idctR,dctR,8,8,8,8);
					idctBlock(idctB,dctB,8,8,8,8);

					//处理完成一个分块，然后将数据复制到原来的输出当中;
					for(p=i;p<i+8;p++)
						for (q=j;q<j+8;q++)
						{
							*(inImageR+p*imageCol+q)=(int)(*(idctR+(p-i)*8+(q-j))+0.5);//红色通道的分块数据;
							*(inImageB+p*imageCol+q)=(int)(*(idctB+(p-i)*8+(q-j))+0.5);//蓝色通道的分块数据;
						}


			}//水印的嵌入过程至此完成；

			//赋值操作，将经过嵌入水印的RGB三个通道的数据合成彩色图像，并且写入到inImage里面;
			for(i=0;i<imageRowTrue;i++)
				for (j=0;j<=imageColTrue-3;j=j+3)
				{

					*(inImage+i*imageColTrue+j)=*(inImageR+i*imageCol+j/3);//写入红色通道的数据;
					*(inImage+i*imageColTrue+j+1)=*(inImageG+i*imageCol+j/3);//写入绿色通道的数据;
					*(inImage+i*imageColTrue+j+2)=*(inImageB+i*imageCol+j/3);//写入蓝色通道的数据；
				}


	} 
	else//表示载体图像是灰度图像，进行灰度图像的嵌水印处理
	{
		inImageGray=(unsigned char *)malloc(sizeof(unsigned char)*imageCol*imageRow);

		if (ifAlign==1)//表示该存储时四字节对齐的方式存储的;
		{
			// 计算真实图像的行数和列数;
			imageRowTrue=imageRow; //对齐字节的行数;行数不需要改变；
			imageColTrue=((imageCol+3)/4)*4;//对齐字节的列数
		} 
		else//表示该存储不是四字节对齐方式的;
		{
			// 计算真实图像的行数核列数;
			imageRowTrue=imageRow; //对齐字节的行数;行数不需要改变；
			imageColTrue=imageCol;//对齐字节的列数;		
		}

		//提取数据；
		for(i=0;i<imageRow;i++)
			for (j=0;j<imageCol;j++)
			{
				*(inImageGray+i*imageCol+j)=*(inImage+i*imageColTrue+j); //读取灰度图像的数据；
			}

		//得到灰度图像的数据inImageGray，然后进行嵌水印操作；
		//嵌入水印的过程;
		dctFirst=(float *)malloc(sizeof(float)*8*8);
		dctSecond=(float *)malloc(sizeof(float)*8*8);
		idctFirst=(float *)malloc(sizeof(float)*8*8);
		idctSecond=(float *)malloc(sizeof(float)*8*8);
		tempFirst=(float *)malloc(sizeof(float)*8*8);//在处理过程中用于存储红色通道数据的一个分块的内容;
		tempSecond=(float *)malloc(sizeof(float)*8*8);//在处理过程中用于存储蓝色通道数据的一个分块的内容;

		for(i=0;i<=imageRow-8;i=i+8)//如果载体图像的行数和列数不能被8整除，怎会抛弃最右边和最下面的满足8*8大小的块;
			for (j=0;j<=imageCol-16;j=j+16)
			{
				//拷贝数据到临时的存储分块单元;
				for(p=i;p<i+8;p++)
					for (q=j;q<j+8;q++)
						*(tempFirst+(p-i)*8+(q-j))=*(inImageGray+p*imageCol+q);//第一个分块数据;
				for(p=i;p<i+8;p++)
					for (q=j+8;q<j+16;q++)
						*(tempSecond+(p-i)*8+(q-j-8))=*(inImageGray+p*imageCol+q);//第二个分块数据;

					//分别做DCT变换；
					dctBlock(dctFirst,tempFirst,8,8,8,8);
					dctBlock(dctSecond,tempSecond,8,8,8,8);
					//求解c的过程;
					f=(std2(tempFirst,8,8)+std2(tempSecond,8,8))/2;
					c=18*0.5*(1+erf(f/theta/sqrt(2.0)))+0.5;

					//如果要嵌入的水印容量小于载体图片的最大水印容量，则表示水印嵌入已经完成，从此返回;
					if (index>=waterRowTrue*waterColTrue) return;
					
					//嵌入水印的过程;
					if (*(watermark+index)==1)//当水印为1时的操作;
					{
						//令第一个通道的能量为第一个通道的能量加上第二个通道的能量；第二个通道的能量值设置为0;
						for (flag=c;flag<64;flag++)
						{
							y=zig_zag(flag);//求出正常的坐标值;
							*(dctFirst+y.row*8+y.col)=(*(dctFirst+y.row*8+y.col))*(*(dctSecond+y.row*8+y.col))>=0?(*(dctFirst+y.row*8+y.col))+(*(dctSecond+y.row*8+y.col)):(*(dctFirst+y.row*8+y.col))-(*(dctSecond+y.row*8+y.col));
							*(dctSecond+y.row*8+y.col)=0;
						}

					} 
					else//当水印为0时的操作;
					{
						//令蓝色通道的能量为红色通道的能量加上蓝色通道的能量；红色通道的能量值设置为0;
						for (flag=c;flag<64;flag++)
						{
							y=zig_zag(flag);//求出正常的坐标值;
							*(dctSecond+y.row*8+y.col)=(*(dctSecond+y.row*8+y.col))*(*(dctFirst+y.row*8+y.col))>=0?(*(dctFirst+y.row*8+y.col))+(*(dctSecond+y.row*8+y.col)):(*(dctSecond+y.row*8+y.col))-(*(dctFirst+y.row*8+y.col));
							*(dctFirst+y.row*8+y.col)=0;
						}

					}

					index++;//水印自加，完成一位水印的嵌入过程;
					//进行IDCT变换;
					idctBlock(idctFirst,dctFirst,8,8,8,8);
					idctBlock(idctSecond,dctSecond,8,8,8,8);
					//处理完成一个分块，然后将数据复制到原来的输出当中;
					for(p=i;p<i+8;p++)
						for (q=j;q<j+8;q++)
							*(inImage+p*imageColTrue+q)=*(idctFirst+(p-i)*8+(q-j));//第一个分块数据;
					for(p=i;p<i+8;p++)
						for (q=j+8;q<j+16;q++)
							*(inImage+p*imageColTrue+q)=*(idctSecond+(p-i)*8+(q-j-8));//第二个分块数据;
			}//水印的嵌入过程至此完成；
	}
	
	return;

}



/************************************************************************/
/* 	dewextract()水印提取函数
	outImage提取的水印图像;
	inImage表示带有水印的载体图像;
	imageRow图像的行数;
	imageCol图像的列数;
	waterRow水印图像的行数；
	waterCol水印图像的列数
	ifColor是否为彩色图像;
	ifAlign是否是四字节对其的存储方式/
/************************************************************************/
void dewWatermarkExtract(unsigned char *outImage,const unsigned char *inImage,int imageRow,int imageCol,int waterRow,int waterCol,unsigned char ifColor,unsigned char ifAlign)
{
	int i,j,p,q;//临时变量，用来for循环的计数作用；
	int index;//嵌水印时候的计数作用;
	int imageRowTrue;//图像的真实的行数;
	int imageColTrue;//图像的真实的列数;
	int waterRowTrue;
	int waterColTrue;

	unsigned char *inR;//当图像是彩色图像时，用来存储图像的红色通道的数据;
	unsigned char *inG;//当图像是彩色图像时，用来存储图像的绿色通道的数据;
	unsigned char *inB;//当图像时彩色图像时，用来存储图像的蓝色通道的数据;
	float f;//纹理负责度;
	int c;//截止坐标;

	float *tempR;//当图像是彩色图像时，用来存储图像的红色通道的一个8*8分块；
	float *tempB;//当图像时彩色图像时，用来存储图像的蓝色通道的一个8*8分块;
	float *dctR;//当图像时彩色图像时，用来存储图像的红色通道的一个8*8分块的DCT变换的系数;
	float *dctB;//当图像时彩色图像时，用来存储图像的蓝色通道的一个8*8分块的DCT变换的系数;
	float energyR;//当图像时彩色图像时，用来存储图像的红色通道的一个8*8分块的DCT能量值;
	float energyB;//当图像时彩色图像时，用来存储图像的蓝色通道的一个8*8分块的DCT能量值;

	float energyFirst;//当图像时灰度图像时，用来存储图像的第一个8*8分块的DCT能量值；
	float energySecond;//当图像时灰度图像时，用来存储图像的第二个8*8分块的DCT能量值；

	float *dctFirst;//当图像时灰度图像时，用来存储图像的第一个8*8分块的DCT系数；
	float *dctSecond;//当图像时灰度图像时，用来存储图像的第二个8*8分块的DCT系数；
	float *tempFirst;//当图像时灰度图像时，用来存储图像的第一个8*8分块；
	float *tempSecond;//当图像时灰度图像时，用来存储图像的第二个8*8分块；

	unsigned char * inImageGray;//当图像时灰度图像时，用来存储图像的实际的值；
	unsigned char *iGray;//当图像时彩色图像时，用来存储图像的彩色转换为灰度的图像；
	float *tempIGray;//当图像时彩色图像时，用来存储图像彩色转换为灰度图像的一个8*8分块；
	float theta;//参数；
	struct Corrnidate y;//用于进行坐标的转换;
	int flag;
	theta=4.5;
	index=0;//提取水印的循环计数标志；

	//根据水印是否是4字节对齐的，计算水印图像的真实大小；
	//首先根据水印图像是否是四字节对齐,
	if (ifAlign==1) // 表示水印图像的数据采用的是四字节对齐的存储方式,读取时候一定要注意读取的方式
	{
		//计算真实图像的行数核列数;
		waterRowTrue=waterRow; //对齐字节的行数;行数不需要改变；
		waterColTrue=((waterCol+3)/4)*4;//对齐字节的列数;
	} 
	else//表示水印图像的数据不是四字节对齐的存储方式，直接读取就OK了。
	{
		//计算真实的图像的行数和列数；
		waterRowTrue=waterRow;
		waterColTrue=waterCol;
	}




	//W=(unsigned char *)malloc(sizeof(unsigned char)*waterRow*waterCol);
	if (ifColor==1)//说明载体图像为彩色图像，对彩色图像进行处理;
	{
		
		inR=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		inG=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		inB=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);

		if (ifAlign==1) // 表示载体图像的数据采用的是四字节对齐的存储方式,读取时候一定要注意读取的方式
		{
			// 计算真实图像的行数核列数;
			imageRowTrue=imageRow; //对齐字节的行数;行数不需要改变；
			imageColTrue=((imageCol*3+3)/4)*4;//对齐字节的列数;
		} 
		else//表示载体图像的数据不是四字节对齐的存储方式，直接读取就OK了。
		{
			//计算真实的图像的行数和列数；
			imageRowTrue=imageRow;
			imageColTrue=imageCol*3;
		} 
		//提取图像的数据的RGB三个通道的数据;
		for(i=0;i<imageRowTrue;i++)
			for (j=0;j<=imageColTrue-3;j=j+3)
			{

				*(inR+i*imageCol+j/3)=*(inImage+i*imageColTrue+j);//读取红色通道的数据;
				*(inG+i*imageCol+j/3)=*(inImage+i*imageColTrue+j+1);//读取绿色通道的数据;
				*(inB+i*imageCol+j/3)=*(inImage+i*imageColTrue+j+2);//读取蓝色通道的数据；
			}

		//提取水印的过程;
		//从彩色图像到灰度图像的转换;
		iGray=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		rgb2gray(iGray,inR,inG,inB,imageRow,imageCol);//三色图像转换为灰度图像存储在iGray中，用于求解纹理复杂度;
		//嵌入水印的过程;
		dctR=(float *)malloc(sizeof(float)*8*8);
		dctB=(float *)malloc(sizeof(float)*8*8);
		tempR=(float *)malloc(sizeof(float)*8*8);//在处理过程中用于存储红色通道数据的一个分块的内容;
		tempB=(float *)malloc(sizeof(float)*8*8);//在处理过程中用于存储蓝色通道数据的一个分块的内容;
		tempIGray=(float *)malloc(sizeof(float)*8*8);
		
		for(i=0;i<=imageRow-8;i=i+8)
			for (j=0;j<=imageCol-8;j=j+8)
			{
				//拷贝数据到临时的存储分块单元;
				for(p=i;p<i+8;p++)
					for (q=j;q<j+8;q++)
					{
						*(tempR+(p-i)*8+(q-j))=*(inR+p*imageCol+q);//红色通道的分块数据;
						*(tempB+(p-i)*8+(q-j))=*(inB+p*imageCol+q);//蓝色通道的分块数据;
						*(tempIGray+(p-i)*8+(q-j))=(float)*(iGray+p*imageCol+q);//灰度图像的分块数据;

					}

					//分别做DCT变换；
					dctBlock(dctR,tempR,8,8,8,8);
					dctBlock(dctB,tempB,8,8,8,8);
					//求解c的过程;
					f=std2(tempIGray,8,8);
					c=18*0.5*(1+erf(f/theta/sqrt(2.0)))+0.5;
					//计算各通道的能量值;
					energyR=0.0;
					energyB=0.0;
					for (flag=c;flag<64;flag++)
					{
						y=zig_zag(flag);
						energyR=energyR+*(dctR+y.row*8+y.col)*(*(dctR+y.row*8+y.col));
						energyB=energyB+*(dctB+y.row*8+y.col)*(*(dctB+y.row*8+y.col));
					}

					//提取水印过程;
					if (index>=waterRowTrue*waterColTrue){ return;}
					*(outImage+index)=energyR>energyB?255:0;
					index++;

			}//水印的提取过程至此完成；


	}
	else//说明载体图像为灰度图像，对灰度图像进行处理；
	{
		inImageGray=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		if (ifAlign==1)//表示该存储时四字节对齐的方式存储的;
		{
			// 计算真实图像的行数核列数;
			imageRowTrue=imageRow; //对齐字节的行数;行数不需要改变；
			imageColTrue=((imageCol+3)/4)*4;//对齐字节的列数;
		} 
		else//表示该存储不是四字节对齐方式的;
		{
			// 计算真实图像的行数核列数;
			imageRowTrue=imageRow; //对齐字节的行数;行数不需要改变；
			imageColTrue=imageCol;//对齐字节的列数;
		}
		//提取数据;
		for(i=0;i<imageRow;i++)
			for (j=0;j<imageCol;j++)
			{
				*(inImageGray+i*imageCol+j)=*(inImage+i*imageColTrue+j); //读取灰度图像的数据；
			}

		//提取水印过程;
		dctFirst=(float *)malloc(sizeof(float)*8*8);
		dctSecond=(float *)malloc(sizeof(float)*8*8);
		tempFirst=(float *)malloc(sizeof(float)*8*8);//在处理过程中用于存储红色通道数据的一个分块的内容;
		tempSecond=(float *)malloc(sizeof(float)*8*8);//在处理过程中用于存储蓝色通道数据的一个分块的内容;

		for(i=0;i<=imageRow-8;i=i+8)//如果载体图像的行数和列数不能被8整除，怎会抛弃最右边和最下面的满足8*8大小的块;
			for (j=0;j<=imageCol-16;j=j+16)
			{
				//拷贝数据到临时的存储分块单元;
				for(p=i;p<i+8;p++)
					for (q=j;q<j+8;q++)
						*(tempFirst+(p-i)*8+(q-j))=*(inImageGray+p*imageCol+q);//第一个分块数据;
				for(p=i;p<i+8;p++)
					for (q=j+8;q<j+16;q++)
						*(tempSecond+(p-i)*8+(q-j-8))=*(inImageGray+p*imageCol+q);//第二个分块数据;

				//分别做DCT变换；
				dctBlock(dctFirst,tempFirst,8,8,8,8);
				dctBlock(dctSecond,tempSecond,8,8,8,8);
				//求解c的过程;
				f=(std2(tempFirst,8,8)+std2(tempSecond,8,8))/2;
				c=18*0.5*(1+erf(f/theta/sqrt(2.0)))+0.5;

				//计算各通道的能量值;
				energyFirst=0.0;
				energySecond=0.0;
				for (flag=c;flag<64;flag++)
				{
					y=zig_zag(flag);
					energyFirst=energyFirst+*(dctFirst+y.row*8+y.col)*(*(dctFirst+y.row*8+y.col));
					energySecond=energySecond+*(dctSecond+y.row*8+y.col)*(*(dctSecond+y.row*8+y.col));
				}

				//提取水印过程;
				if (index>=waterRowTrue*waterColTrue){return;}
				*(outImage+index)=energyFirst>energySecond?255:0;
				index++;
			}
	}

	return;

}

