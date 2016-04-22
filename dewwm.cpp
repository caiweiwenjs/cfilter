#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "watermark.h"

/************************************************************************/
/* ����ṹ��                                                           */
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
/* �����ܽ��ĵ�20130328
1.dew�㷨�Ĳ�ɫͼ�����Ѿ�����ͨ��;
2.һ���������ջ�ռ�Ĵ�СΪ1M,dbs������Ҫ��ͼƬ�Ƚϴ󣬿��Ŀռ�Ƚϴ�
���Ե�ʱ����Զ����öѿռ䣨�����Ѿ��������*/
/************************************************************************/

/************************************************************************/
/* �ַ���������                                                         */
/************************************************************************/

const float PI=3.1415926;
const float E=2.71828;
const float eps=1e-6;



/************************************************************************/
/*  DCT�仯�ĳ���  (���Գɹ�)                                           */
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
/*  IDCT�仯�ĳ���
	outImage��ʾIDCT�任����ͼ��;
	inImage��ʾ�����ԭʼͼ��;
	inImageRow��ʾͼ���������
	inImageCol��ʾͼ�������*/
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
/*  �ֿ�DCT�仯�ĳ��� 
	outImage��ʾDCT�任�����ͼ��
	inImage��ʾ�����ԭʼͼ��;
	inImageRow��ʾͼ���������
	inImageCol��ʾͼ���������
	M��ʾ�ֿ��С��������
	N��ʾ�ֿ��С������*/
/************************************************************************/
void dctBlock(float*outImage,const float *inImage,int inImageRow,int inImageCol,int M,int N)
{
	int i,j,u,v,p,q,uw,vw,pw,qw;
	float parameterA,parameterB;
	float temp;
	//�ж��Ƿ��ܹ��ֿ鴦��;
	if (inImageRow%M || inImageCol%N)
	{
		printf("������ֿ�����!\n");
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
/*  IDCT�仯�ĳ���
	outImage��ʾ�����ͼ��
	inImage��ʾԭʼ�����ͼ��
	inImageRow��ʾͼ�������;
	inImageCol��ʾͼ�������;
	M��ʾ�ֿ��С������;
	N��ʾ�ֿ��С������*/
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
/* Arnold���ҳ���
   outImage��ʾ����֮���ͼ��
   inImage��ʾ��ʼ��ͼ��
   N��ʾͼ��Ĵ�С;
   times��ʾ���ҵĴ���*/
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
/* floyd_halftone���� ������ͨ����     
   outImage��ʾfloyd��ɫ��֮���ͼ��;
   inImage��ʾԭʼ�Ҷ�ͼ��
   inImageRow��ʾͼ�������;
   inImageCol��ʾͼ�������*/
/************************************************************************/

void floyd_halftone(unsigned char *outImage,const unsigned char *inImage,int inImageRow,int inImageCol)
{
	int i,j;
	float e;
	float *tempImage=(float *)malloc(sizeof(float)*(inImageCol+2)*(inImageRow+2)); //��ʱ����;
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
/* ��˹�˲�������
   P��ʾ��˹�˲����ľ���;
   N��ʾҪ��ĸ�˹�˲����Ĵ�С*/
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
/* xcorr2����(�������ع������)   ���Գɹ�      
   outImage��ʾ�����������������Ľ����
   inImageA��ʾ����ľ���A��
   inImageB��ʾ����ľ���B��
   inImageARow��ʾ����A��������
   inImageARow��ʾ����A������;
   inImageBRow��ʾ����B��������
   inImageBCol��ʾ����B������*/
/************************************************************************/
void xcorr2(float *outImage,const float*inImageA,const float*inImageB,int inImageARow,int inImageACol,int inImageBRow,int inImageBCol)
{
	int i,j,Row,Col,istart,jstart,p,q,pw,qw;
	float *tempImage,temp;
	Row=inImageARow+inImageBRow-1;
	Col=inImageACol+inImageBCol-1;
	tempImage=(float *)malloc(sizeof(float)*Row*Col);//��ʱ����;
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
/* ʵ�ֶ�ά��ɢ�������(conv2)  �����Գɹ���   
	outImage��ʾ����������֮��Ľ������;
	inImageA��ʾ����ľ���A��
	inImageB��ʾ����ľ���B��
	inImageARow��ʾ����A������;
	inImageACol��ʾ����A������;
	inImageBRow��ʾ����B������;
	inImageBCol��ʾ����B������;
	type:"full","same"
	���ľ���Ϊ����B����Matlab����һ��*/
/************************************************************************/
void conv2(float *outImage,const float *inImageA,const float *inImageB,int inImageARow,int inImageACol,int inImageBRow,int inImageBCol,const char *type)
{
	int i,j,p,q,Row,Col,centerRow,centerCol;
	float *tempInImageA,*tempInImageB,*tempOutImage;
	float temp,B;
	Row=inImageARow+inImageBRow-1;
	Col=inImageACol+inImageBCol-1;
	//��չinImageAΪtempInImageA;
	tempInImageA=(float *)malloc(sizeof(float)*Row*Col);
	memset(tempInImageA,0,sizeof(float)*Row*Col);
	for(i=0;i<inImageARow;i++)
		for (j=0;j<inImageACol;j++)
		{
			*(tempInImageA+i*Col+j)=*(inImageA+i*inImageACol+j);
	
		}

	//��չinImageBΪtempInImageB��
	tempInImageB=(float*)malloc(sizeof(float)*Row*Col);
	memset(tempInImageB,0,sizeof(float)*Row*Col);
	for(i=0;i<inImageBRow;i++)
		for (j=0;j<inImageBCol;j++)
		{
			*(tempInImageB+i*Col+j)=*(inImageB+i*inImageBCol+j);
			
		}
	
    //�������Ĺ���;
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
		printf("���ʹ��󣡣�\n");
	
	}

	free(tempInImageA);
	free(tempInImageB);
	free(tempOutImage);

	return;


}

/************************************************************************/
/* ��ʱ����ת90�Ⱥ��� �����Գɹ���                                     */
/************************************************************************/
void rot90(float *outImage,const float *inImage,int inImageRow,int inImageCol)
{
	int i,j;
	float *tempInImage;
	int maxN;
	maxN=(inImageRow>inImageCol)?inImageRow:inImageCol;
	tempInImage=(float *)malloc(sizeof(float)*maxN*maxN);
	memset(tempInImage,0,sizeof(float)*maxN*maxN);
	//��չinImage����ΪtempInImage����
	for(i=0;i<inImageRow;i++)
		for(j=0;j<inImageCol;j++)
			*(tempInImage+i*maxN+j)=*(inImage+i*inImageCol+j);

	//��һ��������ת��
		for(i=0;i<maxN;i++)
			for (j=i+1;j<maxN;j++)
			{
				*(tempInImage+i*maxN+j)=*(tempInImage+i*maxN+j)+*(tempInImage+j*maxN+i);
				*(tempInImage+j*maxN+i)=*(tempInImage+i*maxN+j)-*(tempInImage+j*maxN+i);
				*(tempInImage+i*maxN+j)=*(tempInImage+i*maxN+j)-*(tempInImage+j*maxN+i);
			}

	//�ڶ��������»���;

		for(i=0;i<maxN/2;i++)
			for (j=0;j<maxN;j++)
			{
				*(tempInImage+i*maxN+j)=*(tempInImage+i*maxN+j)+*(tempInImage+(maxN-1-i)*maxN+j);
				*(tempInImage+(maxN-1-i)*maxN+j)=*(tempInImage+i*maxN+j)-*(tempInImage+(maxN-1-i)*maxN+j);
				*(tempInImage+i*maxN+j)=*(tempInImage+i*maxN+j)-*(tempInImage+(maxN-1-i)*maxN+j);
			}	
		
	//����������ֵ;
		for(i=0;i<inImageCol;i++)
			for(j=0;j<inImageRow;j++)
			{
				*(outImage+i*inImageRow+j)=*(tempInImage+i*maxN+j);

			}

			free(tempInImage);

		return ;

}


/************************************************************************/
/* �˲�������filter2 ��ѭmatlab�Ĺ���,��һ������Ϊ���ľ��󣨲��Գɹ�) 
   outImage��ʾ�˲������������;
   inImageA��ʾ����ľ���A��
   inImageB��ʾ����ľ���B��
   inImageARow��ʾ����A��������
   inImageACol��ʾ����A������;
   inImageBRow��ʾ����B������;
   inImageBCol��ʾ����B������;
   type="full","same"
   ���ľ���Ϊ����A����Matlab����һ��*/
/************************************************************************/
void filter2(float *outImage,const float *inImageA,const float *inImageB,int inImageARow,int inImageACol,int inImageBRow,int inImageBCol,const char *type)
{
	//������ת180�Ⱥ��ľ���Ȼ�����conv2����ʵ���˲�������
	float *tempInImageRot90,*tempInImageRot180;
	tempInImageRot90=(float *)malloc(sizeof(float)*inImageACol*inImageARow);
	memset(tempInImageRot90,0,sizeof(float)*inImageACol*inImageARow);
	rot90(tempInImageRot90,inImageA,inImageARow,inImageACol);
	tempInImageRot180=(float*)malloc(sizeof(float)*inImageARow*inImageACol);
	memset(tempInImageRot180,0,sizeof(float)*inImageACol*inImageARow);
	rot90(tempInImageRot180,tempInImageRot90,inImageACol,inImageARow);
	//���þ������;
	conv2(outImage,inImageB,tempInImageRot180,inImageBRow,inImageBCol,inImageARow,inImageACol,type);
	free(tempInImageRot90);
	free(tempInImageRot180);
	return ;
}



/************************************************************************/
/* std2()����������ı�׼��
	inImage��ʾ����ľ���
	inImageRow��ʾͼ����������
	inImageCol��ʾͼ����������;
	����ֵΪfloat�������ݱ�ʾ�õ��ľ���ı�׼��   
    �����Գɹ�  ע�⣺��׼���N=N-1��;�����Ⱑ��                       */
/************************************************************************/
float std2(float *inImage,int inImageRow,int inImageCol)
{
	
	int i,j;
	float average;
	float sum;
	float sum2;
	//����ƽ��ֵ��
	sum=0.0;
	for(i=0;i<inImageRow;i++)
		for (j=0;j<inImageCol;j++)
			sum=sum+*(inImage+i*inImageCol+j);
	average=sum/(inImageRow*inImageCol);
	
	//����ƽ����
	sum2=0;
	for(i=0;i<inImageRow;i++)
		for (j=0;j<inImageCol;j++)
			sum2=sum2+(*(inImage+i*inImageCol+j)-average)*(*(inImage+i*inImageCol+j)-average);

	return (sqrt(sum2/(inImageRow*inImageCol-1)));

}


/************************************************************************/
/* rgb2gray()��������ɫͼ��ת��Ϊ�Ҷ�ͼ��
	outImage��ʾ����ĻҶ�ͼ��;
	rImage��ʾ��ɫͼ��ĺ�ɫͨ��������;
	gImage��ʾ��ɫͼ�����ɫͨ��������;
	bImage��ʾ��ɫͼ�����ɫͨ��������;
	Row��ʾͼ�������;
	Col��ʾͼ�������;
	���Գɹ�;				                                    */
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
/* ��׼��̬�ֲ�����erf()     
    ���Գɹ���
	ʵ���������*/
/************************************************************************/
float erf(float x)
{
	int n;//ѭ���������������ֵĲ���;
	float ga;//����;
	float t;//���ֱ�����
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
/* zig-zag����ת��������������8*8�ֿ�����
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
/* updateO����ʵ�� (������ɣ�ע��matlab��c�������鿪ʼ�±겻һ����
 matlab��1��ʼ��c���Դ�0��ʼ)  ���Գɹ�                                 */
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
/* updateCep����ʵ��(���Գɹ�)                                          */
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
	

	for(i=mstart;i<=mend;i++)//ѭ����ʼ
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
/* dbs���ʵ�ֹ���  halftone��ʾ��ʼ��ɫ��ͼ��,gray��ʾ�Ҷ�ͼ��   
imageRow ��ʾͼ���������inmageCol��ʾͼ������� (���Գɹ�)             */
/************************************************************************/
void dbs(unsigned char *halftoneImage,const unsigned char *grayImage,int imageRow,int imageCol)
{
	//�����Ķ����Լ���ʼ������
	int i,j,ii,jj,istart,jstart,iend,jend;
	int a0,a1;
	int PN;//��ʾ�˲����Ĵ�С��
	int flagNum;
	float *tempHalftoneImage,*tempGrayImage,*eImage;
	float *P;//��ʾ��˹�˲�����Ϊ�˺�matlab�����е�����һ��;
	float *Cpp;//��ʾP������ؾ���;
	float *Cep;//��ʾeImage��P����Ļ���ؾ���;
	float  E;//��ʼ��E;
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


	//ͼ���һ������255
	for(i=0;i<imageRow;i++)
	{
		for (j=0;j<imageCol;j++)
		{
			*(tempHalftoneImage+i*imageCol+j)=(float)(*(halftoneImage+i*imageCol+j)/255.0);
			*(tempGrayImage+i*imageCol+j)=(float)(*(grayImage+i*imageCol+j)/255.0);
			*(eImage+i*imageCol+j)=*(tempGrayImage+i*imageCol+j)-*(tempHalftoneImage+i*imageCol+j);//�Ҷ�ͼ���ȥ���ͼ��;
		}
	
	}
	//�õ���˹�˲���;
	PN=7;
	P=(float *)malloc(sizeof(float)*PN*PN);
	Gauss_DBS(P,PN);
	//�õ�Cpp����;
	Cpp=(float *)malloc(sizeof(float)*(2*PN-1)*(2*PN-1));
	xcorr2(Cpp,P,P,PN,PN,PN,PN);

	//�õ�Cep����;
	Cep=(float *)malloc(sizeof(float)*(imageRow)*(imageCol));
	filter2(Cep,Cpp,eImage,(2*PN-1),(2*PN-1),imageRow,imageCol,"same");
	//�õ�E_matric��
	E_matric=(float *)malloc(sizeof(float)*(imageRow+PN-1)*(imageCol+PN-1));
	filter2(E_matric,P,eImage,PN,PN,imageRow,imageCol,"full");
	//�õ�E;
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
			for (j=0;j<imageCol;j++)//���ѭ����ʼ;
			{
				E_temp_end=E;
				memcpy(tempHalftoneImage_temp_end,tempHalftoneImage,sizeof(float)*imageRow*imageCol);
				memcpy(Cep_temp_end,Cep,sizeof(float)*imageRow*imageCol);

				if (i>=1 &&i<=imageRow-2 && j>=1 &&j<=imageCol-2) //�м䲿�ֵ�Ԫ��;
				{
					istart=i-1;
					iend=i+1;
					jstart=j-1;
					jend=j+1;
				}
				
				if (i==0 && j>=1&&j<=imageCol-2)// ��һ���м䲿�֣�
				{
					istart=i;
					iend=i+1;
					jstart=j-1;
					jend=j+1;
				}

				if (i==imageRow-1 && j>=1&&j<=imageCol-2)//���һ���м䲿��
				{
					istart=i-1;
					iend=i;
					jstart=j-1;
					jend=j+1;

				}

				if (i>=1 && i<=imageRow-2 &&j==0)//��һ���м䲿��
				{
					istart=i-1;
					iend=i+1;
					jstart=j;
					jend=j+1;

				}
				
				if (i>=1 && i<=imageRow-2 &&j==imageCol-1)//���һ���м䲿��
				{
					istart=i-1;
					iend=i+1;
					jstart=j-1;
					jend=j;
				}
				
				if (i==0 && j==0) //��һ�е�һ��Ԫ��;
				{
					istart=i;
					iend=i+1;
					jstart=j;
					jend=j+1;
				}
	            
				if (i==0 &&j==imageCol-1) //��һ�����һ��Ԫ��;
				{
					istart=i;
					iend=i+1;
					jstart=j-1;
					jend=j;
				}
				
				if (i==imageRow-1 && j==0) //���һ�е�һ��Ԫ��;
				{
					istart=i-1;
					iend=i;
					jstart=j;
					jend=j+1;

				}
				
				if (i==imageRow-1 && j==imageCol-1)//���һ�����һ��Ԫ��
				{
					istart=i-1;
					iend=i;
					jstart=j-1;
					jend=j;
				}

				for(ii=istart;ii<=iend;ii++)//�ڲ�ѭ����ʼ
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

					}//�ڲ�ѭ������;


				E=E_temp_end;
				memcpy(tempHalftoneImage,tempHalftoneImage_temp_end,sizeof(float)*imageRow*imageCol);
				memcpy(Cep,Cep_temp_end,sizeof(float)*imageRow*imageCol);

			}//���ѭ������;
			

		
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
/* Ƕ��ˮӡ�ĵ�һ����DCT��Ƕ��ˮӡ���õ�Ƕ��ˮӡ֮�����������M_modify  
   N��ʾ����Ľ�����󣬱�ʾ��������Ƕ��ˮӡ֮ǰ��Ƕ��ˮӡ֮��Ĳ�ֵ;
   I_gray��ʾ��ʼ�Ҷ�ͼ��;
   watermark��ʾҪǶ���ˮӡͼ��;
   I_grayRow��I_grayCol��ʾ�Ҷ�ͼ����к���;
   watermarkRow,watermarkCol��ʾˮӡͼ����к���;
   blockM,blockN��ʾͼ��ֿ��С���ֱ��ʾ�ֿ���к���;
   ���Գɹ�201303142042�����ף��������ͳ�����*/
/************************************************************************/

void get_pixel_change(int *N,const unsigned char* halftone, const unsigned char *I_gray,const unsigned char *watermark,int I_grayRow,int I_grayCol,int watermarkRow,int watermarkCol,int blockM,int blockN)
{
	//��ʼ���Լ���������Ĳ��֣�
	float *M ,*M_modify;
	float *M_halftone;
	float *C;
	int i,j,p,q,ii,jj,flag;//��ʱ����;
	int MRow,MCol;
	float H;//���ڲ���;
	float K;//Ƕ���ˮӡǿ��;
	float para;
	H=255.0;
	K=20;
	MRow=I_grayRow/blockM; //�����������;
	MCol=I_grayCol/blockN; //�����������;
	//�ֿ�ͳ��ÿһ���ƽ��ֵ���õ���ʼ����������;
	M=(float *)malloc(sizeof(float)*MRow*MCol);
	M_modify=(float *)malloc(sizeof(float)*MRow*MCol);
	M_halftone=(float *)malloc(sizeof(float)*MRow*MCol);
	memset(M,0,sizeof(float)*MRow*MCol);
	memset(M_modify,0,sizeof(float)*MRow*MCol);
	memset(M_halftone,0,sizeof(float)*MRow*MCol);

	for(i=0,ii=0;i<I_grayRow;i=i+blockM,ii++)//���ѭ��start
		for (j=0,jj=0;j<I_grayCol;j=j+blockN,jj++)
		{
			for(p=i;p<i+blockM;p++)//�ڲ�ѭ��start��
				for (q=j;q<j+blockN;q++)
				{
					*(M+ii*MCol+jj)=*(M+ii*MCol+jj)+(*(I_gray+p*I_grayCol+q)/255.0);
					*(M_halftone+ii*MCol+jj)=*(M_halftone+ii*MCol+jj)+(*(halftone+p*I_grayCol+q)/255.0);
				//	printf("%.lf   ",*(M+ii*MCol+jj));

				}//�ڲ�ѭ��end;

				*(M+ii*MCol+jj)=(*(M+ii*MCol+jj)/(float)(blockM*blockN))*H;//�õ����ڵľ�ֵ��
				*(M_halftone+ii*MCol+jj)=(*(M_halftone+ii*MCol+jj)/(float)(blockM*blockN))*H;
			//	printf("%lf   ",*(M+ii*MCol+jj));

		}//���ѭ��end

	//������������зֿ�DCT�仯
	C=(float *)malloc(sizeof(float)*MRow*MCol);
//	C_modify=(float *)malloc(sizeof(float)*MRow*MCol);
	dctBlock(C,M,MRow,MCol,8,8);

	//Ƕ��ˮӡ
	flag=0;
	for(i=0;i<MRow;i=i+8)//���ѭ��start;
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



		}//���ѭ��end;

	//��Ƕ��ˮӡ�������������IDCT�仯���õ��޸�֮�����������
	idctBlock(M_modify,C,MRow,MCol,8,8);

    //��������������N 
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
/* new_flip_pixel()������ʵ��
 outImage��ʾ��ת֮���ͼ��;
 halftoneImage��ʾ��ʼ�İ�ɫ��ͼ��
 N��ʾҪ��ת�ľ���
 imageRow imageCol��ʾͼ�������������
 blockRow,blockCol��ʾ�ֿ�Ŀ������������	
 ���Գɹ�201303142345							*/
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
	for(i=0,ii=0;i<imageRow;i=i+blockRow,ii++)//���ѭ��start
		for (j=0,jj=0;j<imageCol;j=j+blockCol,jj++)
		{
		//	for(p=i;p<i+blockRow;p++)//�ڲ�ѭ��start
		//		for (q=j;q<j+blockCol;q++)
		//		{
					aviableNum=0;
					if (*(N+ii*NCol+jj)>0)//��ʾ��Ҫ�Ѱ�ɫ���ر�ɺ�ɫ����(1-->0)
					{
						//1.����ɷ�ת�����صĸ�����
						for(p=i;p<i+blockRow;p++)
							for (q=j;q<j+blockCol;q++)
							{
								if (*(outImage+p*imageCol+q)/255==1)
								{
									aviableNum++;
								}
									
							}//����ɷ�ת���ؽ���;

						if (aviableNum<=*(N+ii*NCol+jj))//��Ҫȫ����ת����
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
						else//��Ҫѡ���Եķ�תһЩ����;
						{	
							flagNum=*(N+ii*NCol+jj);
							//���ѡ����˳��ѡ��ת������
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
					if(*(N+ii*NCol+jj)<0)//��ʾ��Ҫ�Ѻ�ɫ���ر�ɰ�ɫ����(0-->1)
					{
						
						//1.����ɷ�ת�����صĸ�����
						for(p=i;p<i+blockRow;p++)
							for (q=j;q<j+blockCol;q++)
							{
								if (*(outImage+p*imageCol+q)==0)
								{
									aviableNum++;
								}
								
							}//����ɷ�ת���ؽ���;
							
							if (aviableNum<=-(*(N+ii*NCol+jj)))//��Ҫȫ����ת����
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
							else//��Ҫѡ���Եķ�תһЩ����;
							{	
								flagNum=-(*(N+ii*NCol+jj));
								//���ѡ����˳��ѡ��ת������
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

			//	}//�ڲ�ѭ��end
		}//���ѭ��end

		return;

}

/************************************************************************/
/* extractWatermark������ȡˮӡ�ĺ���;
   outImage��ʾ��ȡ��ˮӡͼ��;
   inImage��ʾ����ĺ���ˮӡ��ͼ��
   imageRow��ʾͼ�������;
   imageCol��ʾͼ���������
   blockRow��ʾ�ֿ������;
   blockCol��ʾ�ֿ��������*/
/************************************************************************/
void extractWatermark(unsigned char *outImage,unsigned char *inImage,int imageRow,int imageCol,int blockRow,int blockCol)
{
	//��ʼ���Լ���������Ĳ��֣�
	float *M;
	float *C;
	int i,j,p,q,ii,jj,flag;//��ʱ����;
	int MRow,MCol;
	float H;//���ڲ���;
	H=255.0;
	MRow=imageRow/blockRow; //�����������;
	MCol=imageCol/blockCol; //�����������;
	//�ֿ�ͳ��ÿһ���ƽ��ֵ���õ���ʼ����������;
	M=(float *)malloc(sizeof(float)*MRow*MCol);
	memset(M,0,sizeof(float)*MRow*MCol);
	for(i=0;i<imageRow;i++)
		for (j=0;j<imageCol;j++)
			*(inImage+i*imageCol+j)=*(inImage+i*imageCol+j)/255;

	

	
	for(i=0,ii=0;i<imageRow;i=i+blockRow,ii++)//���ѭ��start
		for (j=0,jj=0;j<imageCol;j=j+blockCol,jj++)
		{
			for(p=i;p<i+blockRow;p++)//�ڲ�ѭ��start��
				for (q=j;q<j+blockCol;q++)
				{
					*(M+ii*MCol+jj)=*(M+ii*MCol+jj)+*(inImage+p*imageCol+q);		
				}//�ڲ�ѭ��end;
				
				*(M+ii*MCol+jj)=(*(M+ii*MCol+jj)/(float)(blockRow*blockCol))*H;//�õ����ڵľ�ֵ��
				
		}//���ѭ��end
		
		//������������зֿ�DCT�仯
		C=(float *)malloc(sizeof(float)*MRow*MCol);
		dctBlock(C,M,MRow,MCol,8,8);
		
		//��ȡˮӡ
		flag=0;
		for(i=0;i<MRow;i=i+8)//���ѭ��start;
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
				
				
				
		}//���ѭ��end;

			free(M);
			free(C);
			return;
}


/************************************************************************/
/* embedWatermark()������ʾˮӡǶ�뺯��
outImage��ʾ����ĺ���ˮӡ��ͼ��;
grayImage��ʾ��ʼ�ĻҶ�ͼ��
watermark��ʾˮӡͼ�����;
imageRow��ʾͼ���������
imageCol��ʾͼ�������;
watermarkRow��ʾˮӡ������;
watermarkCol��ʾˮӡ������;
blockRow��ʾ�ֿ������;
blockCol��ʾ�ֿ������;	*/
/************************************************************************/
void embedWatermark(unsigned char *outImage,const unsigned char *grayImage,const unsigned char *watermark,int imageRow,int imageCol,int watermarkRow,int watermarkCol,int blockRow,int blockCol)
{
	unsigned char *halftoneImage;
	int middleRow;//�м�ͼ������;
	int middleCol;//�м�ͼ��������
	int *N;//��ʾÿһ���ֿ鷴ת�ĸ�����
	unsigned char *tempDbs,*tempGray;
	int i,j,p,q;
	
	middleRow=imageRow/blockRow;
	middleCol=imageCol/blockCol;
	//�Գ�ʼ��ͼ����а�ɫ��;
	halftoneImage=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
	floyd_halftone(halftoneImage,grayImage,imageRow,imageCol);
	N=(int *)malloc(sizeof(int)*middleRow*middleCol);
	get_pixel_change(N,halftoneImage,grayImage,watermark,imageRow,imageCol,watermarkRow,watermarkCol,blockRow,blockCol);
	new_flip_pixel(outImage,halftoneImage,N,imageRow,imageCol,blockRow,blockCol);
	//�ֿ�dbs����;
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
/* dewembed()��������dew�㷨�ڲ�ɫͼ����Ƕ��ˮӡ�ķ���
	inImage ��ʾԭʼ������ͼ���Ƕˮӡ֮���ͼ��;
	watermark��ʾˮӡ����;
	imageRow��ʾ����ͼ�������;
	imageCol��ʾ����ͼ���������
	waterRow��ʾˮӡͼ�������;
	waterCol��ʾˮӡͼ�������;
	ifColor��ʾ����ͼ���Ƿ�Ϊ��ɫͼ��
	���Ϊ��ɫͼ��ifColor==1,����ǻҶ�ͼ��ifColor==0
	ifAlign��ʾ����ͼ��������Ƿ�Ϊ4�ֽڶ�����ʽ��ifAlign==1��ʾ�����ֽڶ���
	ifAlign==0��ʾ�������ֽڶ���*/
/************************************************************************/
void dewWatermarkEmbed(unsigned char *inImage, unsigned char *watermark,int imageRow,int imageCol,int waterRow,int waterCol,unsigned char ifColor,unsigned char ifAlign)
{
	int i,j,p,q;//��ʱ����
	int index;  //ˮӡ��λ��;
	float f; //�������;
	int c;//��ֹ����;
	int imageRowTrue;//�����Ƿ����ֽڶ��룬�ñ���ʼ�յ���ͼ�������;
	int imageColTrue;//��ͼ�������ֽڶ����ʱ�򣬵���ͼ������������ͼ��ʱ���ֽڶ���Ļ����ü���;
	int waterRowTrue;
	int waterColTrue;

	unsigned char* inImageR;//�������ͼ���ǲ�ɫͼ�������洢ͼ��ĺ�ɫͨ��������;
	unsigned char* inImageG;//�������ͼ���ǲ�ɫͼ�������洢ͼ�����ɫͨ�������ݣ�
	unsigned char* inImageB;//�������ͼ���ǲ�ɫͼ�������洢ͼ�����ɫͨ��������;
	unsigned char* inImageGray;//�������ͼ��ʱ�Ҷ�ͼ�������洢����ͼ��;
	unsigned char* watermarkTemp;//��ʱ�洢ˮӡ����Ϣ;


	float *tempR;//��ɫͼ��ʱ��������ʱ�洢��ɫͨ����һ��8*8�ֿ������;
	float *tempB;//��ɫͼ��ʱ��������ʱ�洢��ɫͨ����һ��8*8�ֿ������;
	float *dctR;//��ɫͼ��ʱ��������ʱ�洢��ɫͨ����һ��8*8�ֿ��DCT�任ϵ��;
	float *dctB;//��ɫͼ��ʱ��������ʱ�洢��ɫͨ����һ��8*8�ֿ��DCT�任ϵ��;
	float *idctR;//��ɫͼ��ʱ��������ʱ�洢��ɫͨ����һ��8*8�ֿ��IDCT�任ϵ��;
	float *idctB;//��ɫͼ��ʱ��������ʱ�洢��ɫͨ����һ��8*8�ֿ��IDCT�任ϵ��;

	float *tempFirst;//�Ҷ�ͼ��ʱ�������洢Ƕˮӡ�ֿ�Եĵ�һ��8*8�ֿ�;
	float *tempSecond;//�Ҷ�ͼ��ʱ�������洢Ƕˮӡ�ֿ�Եĵڶ���8*8�ֿ�;
	float *dctFirst;//�Ҷ�ͼ��ʱ�������洢Ƕˮӡ�ֿ�Եĵ�һ��8*8�ֿ��DCT�任��ϵ��;
	float *dctSecond;//�Ҷ�ͼ��ʱ�������洢Ƕˮӡ�ֿ�Եĵڶ���8*8�ֿ��DCT�任��ϵ��;
	float *idctFirst;//�Ҷ�ͼ��ʱ�������洢Ƕˮӡ�ֿ�Եĵ�һ��8*8�ֿ��IDCT�任��ϵ��;
	float *idctSecond;//�Ҷ�ͼ��ʱ�������洢Ƕˮӡ�ֿ�Եĵڶ���8*8�ֿ��IDCT�任��ϵ��;

	unsigned char *iGray;//��ɫͼ��ʱ�����������ɫͼ��ת��Ϊ�Ҷ�ͼ�������;
	float *tempIGray;//��ɫͼ��ʱ����������ת��Ϊ�Ҷ�ͼ���һ��8*8�ֿ������;
	float theta;//���cʱ��Ĳ�����
	struct Corrnidate y;//���ڽ��������ת��;
	int flag;//��־λ��
	theta=4.5;//����ֵ��
	index=0;//Ƕˮӡ��λ��;

	//���ȸ���ˮӡͼ���Ƿ������ֽڶ���,���ˮӡ��ʵ��������Ϣ;
	if (ifAlign==1) // ��ʾˮӡͼ������ݲ��õ������ֽڶ���Ĵ洢��ʽ,��ȡʱ��һ��Ҫע���ȡ�ķ�ʽ
	{
		//������ʵͼ�������������;
		waterRowTrue=waterRow; //�����ֽڵ�����;��������Ҫ�ı䣻
		waterColTrue=((waterCol+3)/4)*4;//�����ֽڵ�����;
	} 
	else//��ʾˮӡͼ������ݲ������ֽڶ���Ĵ洢��ʽ��ֱ�Ӷ�ȡ��OK�ˡ�
	{
		//������ʵ��ͼ���������������
		waterRowTrue=waterRow;
		waterColTrue=waterCol;
	}
	watermarkTemp=(unsigned char *)malloc(sizeof(unsigned char)*waterRowTrue*waterColTrue);
	//��ȡˮӡ���ݣ�
	for(i=0;i<waterRowTrue;i++)
		for (j=0;j<waterColTrue;j++)
		{
			*(watermarkTemp+i*waterColTrue+j)=*(watermark+i*waterColTrue+j); //��ȡˮӡͼ������ݣ�
		}
	//�ԻҶȵ�ˮӡͼ����а�ɫ������
		floyd_halftone(watermark,watermarkTemp,waterRowTrue,waterColTrue);
	//��ˮӡ֮���ͼ��任��01������;
	for(i=0;i<waterRow;i++)
		for (j=0;j<waterCol;j++)
		{
			*(watermark+i*waterCol+j)=*(watermark+i*waterCol+j)/255; //��ȡˮӡͼ������ݣ�
		}

	if (ifColor==1)//��ʾ����ͼ��ʱ��ɫͼ�񣬽��в�ɫͼ���Ƕˮӡ����
	{

		inImageR=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		inImageG=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		inImageB=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);


		if (ifAlign==1) // ��ʾ����ͼ������ݲ��õ������ֽڶ���Ĵ洢��ʽ,��ȡʱ��һ��Ҫע���ȡ�ķ�ʽ
		{
			//������ʵͼ�������������;
			imageRowTrue=imageRow; //�����ֽڵ�����;��������Ҫ�ı䣻
			imageColTrue=((imageCol*3+3)/4)*4;//�����ֽڵ�����;
		} 
		else//��ʾ����ͼ������ݲ������ֽڶ���Ĵ洢��ʽ��ֱ�Ӷ�ȡ��OK�ˡ�
		{
			//������ʵ��ͼ���������������
			imageRowTrue=imageRow;
			imageColTrue=imageCol*3;
		}

		//��ȡͼ������ݵ�RGB����ͨ��������;
		for(i=0;i<imageRowTrue;i++)
			for (j=0;j<=imageColTrue-3;j=j+3)
			{

				*(inImageR+i*imageCol+j/3)=*(inImage+i*imageColTrue+j);//��ȡ��ɫͨ��������;
				*(inImageG+i*imageCol+j/3)=*(inImage+i*imageColTrue+j+1);//��ȡ��ɫͨ��������;
				*(inImageB+i*imageCol+j/3)=*(inImage+i*imageColTrue+j+2);//��ȡ��ɫͨ�������ݣ�
			}


		//���沿�ֵõ���ͼ�����ݵĺ���������ͨ��������;���濪ʼǶ��ˮӡ;
		//�Ӳ�ɫͼ�񵽻Ҷ�ͼ���ת��;
		iGray=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		rgb2gray(iGray,inImageR,inImageG,inImageB,imageRow,imageCol);//��ɫͼ��ת��Ϊ�Ҷ�ͼ��洢��iGray�У�������������Ӷ�;
		//Ƕ��ˮӡ�Ĺ���;
		dctR=(float *)malloc(sizeof(float)*8*8);
		dctB=(float *)malloc(sizeof(float)*8*8);
		idctR=(float *)malloc(sizeof(float)*8*8);
		idctB=(float *)malloc(sizeof(float)*8*8);
		tempR=(float *)malloc(sizeof(float)*8*8);
		tempB=(float *)malloc(sizeof(float)*8*8);
		tempIGray=(float *)malloc(sizeof(float)*8*8);

		for(i=0;i<=imageRow-8;i=i+8)//�������ͼ����������������ܱ�8�����������������ұߺ������������8*8��С�Ŀ�;
			for (j=0;j<=imageCol-8;j=j+8)
			{
				//�������ݵ���ʱ�Ĵ洢�ֿ鵥Ԫ;
				for(p=i;p<i+8;p++)
					for (q=j;q<j+8;q++)
					{
						*(tempR+(p-i)*8+(q-j))=*(inImageR+p*imageCol+q);//��ɫͨ���ķֿ�����;
						*(tempB+(p-i)*8+(q-j))=*(inImageB+p*imageCol+q);//��ɫͨ���ķֿ�����;
						*(tempIGray+(p-i)*8+(q-j))=(float)*(iGray+p*imageCol+q);//�Ҷ�ͼ��ķֿ�����;
						
					}

					//�ֱ���DCT�任��
					dctBlock(dctR,tempR,8,8,8,8);
					dctBlock(dctB,tempB,8,8,8,8);
					//���c�Ĺ���;
					f=std2(tempIGray,8,8);
					c=18*0.5*(1+erf(f/theta/sqrt(2.0)))+0.5;

					//���ҪǶ���ˮӡ����С������ͼƬ�����ˮӡ���������ʾˮӡǶ���Ѿ���ɣ��Ӵ˷���;
					if (index>=waterRowTrue*waterColTrue)
					{
						//������д�뵽���صĲ�ɫͼ����;
						for(i=0;i<imageRowTrue;i++)
							for (j=0;j<=imageColTrue-3;j=j+3)
							{
								*(inImage+i*imageColTrue+j)=*(inImageR+i*imageCol+j/3);//д���ɫͨ��������;
								*(inImage+i*imageColTrue+j+1)=*(inImageG+i*imageCol+j/3);//д����ɫͨ��������;
								*(inImage+i*imageColTrue+j+2)=*(inImageB+i*imageCol+j/3);//д����ɫͨ�������ݣ�
							}	
						return;
					}


					//Ƕ��ˮӡ�Ĺ���;
					if (*(watermark+index)==1)//��ˮӡΪ1ʱ�Ĳ���;
					{
						//���ɫͨ��������Ϊ��ɫͨ��������������ɫͨ������������ɫͨ��������ֵ����Ϊ0;
						for (flag=c;flag<64;flag++)
						{
							y=zig_zag(flag);//�������������ֵ;
							*(dctR+y.row*8+y.col)=(*(dctR+y.row*8+y.col))*(*(dctB+y.row*8+y.col))>=0?(*(dctR+y.row*8+y.col))+(*(dctB+y.row*8+y.col)):(*(dctR+y.row*8+y.col))-(*(dctB+y.row*8+y.col));
							*(dctB+y.row*8+y.col)=0;

						}

					} 
					else//��ˮӡΪ0ʱ�Ĳ���;
					{
						//����ɫͨ��������Ϊ��ɫͨ��������������ɫͨ������������ɫͨ��������ֵ����Ϊ0;
						for (flag=c;flag<64;flag++)
						{
							y=zig_zag(flag);//�������������ֵ;
							*(dctB+y.row*8+y.col)=(*(dctB+y.row*8+y.col))*(*(dctR+y.row*8+y.col))>=0?(*(dctR+y.row*8+y.col))+(*(dctB+y.row*8+y.col)):(*(dctB+y.row*8+y.col))-(*(dctR+y.row*8+y.col));
							*(dctR+y.row*8+y.col)=0;

						}

					}

					index++;//ˮӡ�Լӣ����һλˮӡ��Ƕ�����;
					//����IDCT�任;
					idctBlock(idctR,dctR,8,8,8,8);
					idctBlock(idctB,dctB,8,8,8,8);

					//�������һ���ֿ飬Ȼ�����ݸ��Ƶ�ԭ�����������;
					for(p=i;p<i+8;p++)
						for (q=j;q<j+8;q++)
						{
							*(inImageR+p*imageCol+q)=(int)(*(idctR+(p-i)*8+(q-j))+0.5);//��ɫͨ���ķֿ�����;
							*(inImageB+p*imageCol+q)=(int)(*(idctB+(p-i)*8+(q-j))+0.5);//��ɫͨ���ķֿ�����;
						}


			}//ˮӡ��Ƕ�����������ɣ�

			//��ֵ������������Ƕ��ˮӡ��RGB����ͨ�������ݺϳɲ�ɫͼ�񣬲���д�뵽inImage����;
			for(i=0;i<imageRowTrue;i++)
				for (j=0;j<=imageColTrue-3;j=j+3)
				{

					*(inImage+i*imageColTrue+j)=*(inImageR+i*imageCol+j/3);//д���ɫͨ��������;
					*(inImage+i*imageColTrue+j+1)=*(inImageG+i*imageCol+j/3);//д����ɫͨ��������;
					*(inImage+i*imageColTrue+j+2)=*(inImageB+i*imageCol+j/3);//д����ɫͨ�������ݣ�
				}


	} 
	else//��ʾ����ͼ���ǻҶ�ͼ�񣬽��лҶ�ͼ���Ƕˮӡ����
	{
		inImageGray=(unsigned char *)malloc(sizeof(unsigned char)*imageCol*imageRow);

		if (ifAlign==1)//��ʾ�ô洢ʱ���ֽڶ���ķ�ʽ�洢��;
		{
			// ������ʵͼ�������������;
			imageRowTrue=imageRow; //�����ֽڵ�����;��������Ҫ�ı䣻
			imageColTrue=((imageCol+3)/4)*4;//�����ֽڵ�����
		} 
		else//��ʾ�ô洢�������ֽڶ��뷽ʽ��;
		{
			// ������ʵͼ�������������;
			imageRowTrue=imageRow; //�����ֽڵ�����;��������Ҫ�ı䣻
			imageColTrue=imageCol;//�����ֽڵ�����;		
		}

		//��ȡ���ݣ�
		for(i=0;i<imageRow;i++)
			for (j=0;j<imageCol;j++)
			{
				*(inImageGray+i*imageCol+j)=*(inImage+i*imageColTrue+j); //��ȡ�Ҷ�ͼ������ݣ�
			}

		//�õ��Ҷ�ͼ�������inImageGray��Ȼ�����Ƕˮӡ������
		//Ƕ��ˮӡ�Ĺ���;
		dctFirst=(float *)malloc(sizeof(float)*8*8);
		dctSecond=(float *)malloc(sizeof(float)*8*8);
		idctFirst=(float *)malloc(sizeof(float)*8*8);
		idctSecond=(float *)malloc(sizeof(float)*8*8);
		tempFirst=(float *)malloc(sizeof(float)*8*8);//�ڴ�����������ڴ洢��ɫͨ�����ݵ�һ���ֿ������;
		tempSecond=(float *)malloc(sizeof(float)*8*8);//�ڴ�����������ڴ洢��ɫͨ�����ݵ�һ���ֿ������;

		for(i=0;i<=imageRow-8;i=i+8)//�������ͼ����������������ܱ�8�����������������ұߺ������������8*8��С�Ŀ�;
			for (j=0;j<=imageCol-16;j=j+16)
			{
				//�������ݵ���ʱ�Ĵ洢�ֿ鵥Ԫ;
				for(p=i;p<i+8;p++)
					for (q=j;q<j+8;q++)
						*(tempFirst+(p-i)*8+(q-j))=*(inImageGray+p*imageCol+q);//��һ���ֿ�����;
				for(p=i;p<i+8;p++)
					for (q=j+8;q<j+16;q++)
						*(tempSecond+(p-i)*8+(q-j-8))=*(inImageGray+p*imageCol+q);//�ڶ����ֿ�����;

					//�ֱ���DCT�任��
					dctBlock(dctFirst,tempFirst,8,8,8,8);
					dctBlock(dctSecond,tempSecond,8,8,8,8);
					//���c�Ĺ���;
					f=(std2(tempFirst,8,8)+std2(tempSecond,8,8))/2;
					c=18*0.5*(1+erf(f/theta/sqrt(2.0)))+0.5;

					//���ҪǶ���ˮӡ����С������ͼƬ�����ˮӡ���������ʾˮӡǶ���Ѿ���ɣ��Ӵ˷���;
					if (index>=waterRowTrue*waterColTrue) return;
					
					//Ƕ��ˮӡ�Ĺ���;
					if (*(watermark+index)==1)//��ˮӡΪ1ʱ�Ĳ���;
					{
						//���һ��ͨ��������Ϊ��һ��ͨ�����������ϵڶ���ͨ�����������ڶ���ͨ��������ֵ����Ϊ0;
						for (flag=c;flag<64;flag++)
						{
							y=zig_zag(flag);//�������������ֵ;
							*(dctFirst+y.row*8+y.col)=(*(dctFirst+y.row*8+y.col))*(*(dctSecond+y.row*8+y.col))>=0?(*(dctFirst+y.row*8+y.col))+(*(dctSecond+y.row*8+y.col)):(*(dctFirst+y.row*8+y.col))-(*(dctSecond+y.row*8+y.col));
							*(dctSecond+y.row*8+y.col)=0;
						}

					} 
					else//��ˮӡΪ0ʱ�Ĳ���;
					{
						//����ɫͨ��������Ϊ��ɫͨ��������������ɫͨ������������ɫͨ��������ֵ����Ϊ0;
						for (flag=c;flag<64;flag++)
						{
							y=zig_zag(flag);//�������������ֵ;
							*(dctSecond+y.row*8+y.col)=(*(dctSecond+y.row*8+y.col))*(*(dctFirst+y.row*8+y.col))>=0?(*(dctFirst+y.row*8+y.col))+(*(dctSecond+y.row*8+y.col)):(*(dctSecond+y.row*8+y.col))-(*(dctFirst+y.row*8+y.col));
							*(dctFirst+y.row*8+y.col)=0;
						}

					}

					index++;//ˮӡ�Լӣ����һλˮӡ��Ƕ�����;
					//����IDCT�任;
					idctBlock(idctFirst,dctFirst,8,8,8,8);
					idctBlock(idctSecond,dctSecond,8,8,8,8);
					//�������һ���ֿ飬Ȼ�����ݸ��Ƶ�ԭ�����������;
					for(p=i;p<i+8;p++)
						for (q=j;q<j+8;q++)
							*(inImage+p*imageColTrue+q)=*(idctFirst+(p-i)*8+(q-j));//��һ���ֿ�����;
					for(p=i;p<i+8;p++)
						for (q=j+8;q<j+16;q++)
							*(inImage+p*imageColTrue+q)=*(idctSecond+(p-i)*8+(q-j-8));//�ڶ����ֿ�����;
			}//ˮӡ��Ƕ�����������ɣ�
	}
	
	return;

}



/************************************************************************/
/* 	dewextract()ˮӡ��ȡ����
	outImage��ȡ��ˮӡͼ��;
	inImage��ʾ����ˮӡ������ͼ��;
	imageRowͼ�������;
	imageColͼ�������;
	waterRowˮӡͼ���������
	waterColˮӡͼ�������
	ifColor�Ƿ�Ϊ��ɫͼ��;
	ifAlign�Ƿ������ֽڶ���Ĵ洢��ʽ/
/************************************************************************/
void dewWatermarkExtract(unsigned char *outImage,const unsigned char *inImage,int imageRow,int imageCol,int waterRow,int waterCol,unsigned char ifColor,unsigned char ifAlign)
{
	int i,j,p,q;//��ʱ����������forѭ���ļ������ã�
	int index;//Ƕˮӡʱ��ļ�������;
	int imageRowTrue;//ͼ�����ʵ������;
	int imageColTrue;//ͼ�����ʵ������;
	int waterRowTrue;
	int waterColTrue;

	unsigned char *inR;//��ͼ���ǲ�ɫͼ��ʱ�������洢ͼ��ĺ�ɫͨ��������;
	unsigned char *inG;//��ͼ���ǲ�ɫͼ��ʱ�������洢ͼ�����ɫͨ��������;
	unsigned char *inB;//��ͼ��ʱ��ɫͼ��ʱ�������洢ͼ�����ɫͨ��������;
	float f;//�������;
	int c;//��ֹ����;

	float *tempR;//��ͼ���ǲ�ɫͼ��ʱ�������洢ͼ��ĺ�ɫͨ����һ��8*8�ֿ飻
	float *tempB;//��ͼ��ʱ��ɫͼ��ʱ�������洢ͼ�����ɫͨ����һ��8*8�ֿ�;
	float *dctR;//��ͼ��ʱ��ɫͼ��ʱ�������洢ͼ��ĺ�ɫͨ����һ��8*8�ֿ��DCT�任��ϵ��;
	float *dctB;//��ͼ��ʱ��ɫͼ��ʱ�������洢ͼ�����ɫͨ����һ��8*8�ֿ��DCT�任��ϵ��;
	float energyR;//��ͼ��ʱ��ɫͼ��ʱ�������洢ͼ��ĺ�ɫͨ����һ��8*8�ֿ��DCT����ֵ;
	float energyB;//��ͼ��ʱ��ɫͼ��ʱ�������洢ͼ�����ɫͨ����һ��8*8�ֿ��DCT����ֵ;

	float energyFirst;//��ͼ��ʱ�Ҷ�ͼ��ʱ�������洢ͼ��ĵ�һ��8*8�ֿ��DCT����ֵ��
	float energySecond;//��ͼ��ʱ�Ҷ�ͼ��ʱ�������洢ͼ��ĵڶ���8*8�ֿ��DCT����ֵ��

	float *dctFirst;//��ͼ��ʱ�Ҷ�ͼ��ʱ�������洢ͼ��ĵ�һ��8*8�ֿ��DCTϵ����
	float *dctSecond;//��ͼ��ʱ�Ҷ�ͼ��ʱ�������洢ͼ��ĵڶ���8*8�ֿ��DCTϵ����
	float *tempFirst;//��ͼ��ʱ�Ҷ�ͼ��ʱ�������洢ͼ��ĵ�һ��8*8�ֿ飻
	float *tempSecond;//��ͼ��ʱ�Ҷ�ͼ��ʱ�������洢ͼ��ĵڶ���8*8�ֿ飻

	unsigned char * inImageGray;//��ͼ��ʱ�Ҷ�ͼ��ʱ�������洢ͼ���ʵ�ʵ�ֵ��
	unsigned char *iGray;//��ͼ��ʱ��ɫͼ��ʱ�������洢ͼ��Ĳ�ɫת��Ϊ�Ҷȵ�ͼ��
	float *tempIGray;//��ͼ��ʱ��ɫͼ��ʱ�������洢ͼ���ɫת��Ϊ�Ҷ�ͼ���һ��8*8�ֿ飻
	float theta;//������
	struct Corrnidate y;//���ڽ��������ת��;
	int flag;
	theta=4.5;
	index=0;//��ȡˮӡ��ѭ��������־��

	//����ˮӡ�Ƿ���4�ֽڶ���ģ�����ˮӡͼ�����ʵ��С��
	//���ȸ���ˮӡͼ���Ƿ������ֽڶ���,
	if (ifAlign==1) // ��ʾˮӡͼ������ݲ��õ������ֽڶ���Ĵ洢��ʽ,��ȡʱ��һ��Ҫע���ȡ�ķ�ʽ
	{
		//������ʵͼ�������������;
		waterRowTrue=waterRow; //�����ֽڵ�����;��������Ҫ�ı䣻
		waterColTrue=((waterCol+3)/4)*4;//�����ֽڵ�����;
	} 
	else//��ʾˮӡͼ������ݲ������ֽڶ���Ĵ洢��ʽ��ֱ�Ӷ�ȡ��OK�ˡ�
	{
		//������ʵ��ͼ���������������
		waterRowTrue=waterRow;
		waterColTrue=waterCol;
	}




	//W=(unsigned char *)malloc(sizeof(unsigned char)*waterRow*waterCol);
	if (ifColor==1)//˵������ͼ��Ϊ��ɫͼ�񣬶Բ�ɫͼ����д���;
	{
		
		inR=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		inG=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		inB=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);

		if (ifAlign==1) // ��ʾ����ͼ������ݲ��õ������ֽڶ���Ĵ洢��ʽ,��ȡʱ��һ��Ҫע���ȡ�ķ�ʽ
		{
			// ������ʵͼ�������������;
			imageRowTrue=imageRow; //�����ֽڵ�����;��������Ҫ�ı䣻
			imageColTrue=((imageCol*3+3)/4)*4;//�����ֽڵ�����;
		} 
		else//��ʾ����ͼ������ݲ������ֽڶ���Ĵ洢��ʽ��ֱ�Ӷ�ȡ��OK�ˡ�
		{
			//������ʵ��ͼ���������������
			imageRowTrue=imageRow;
			imageColTrue=imageCol*3;
		} 
		//��ȡͼ������ݵ�RGB����ͨ��������;
		for(i=0;i<imageRowTrue;i++)
			for (j=0;j<=imageColTrue-3;j=j+3)
			{

				*(inR+i*imageCol+j/3)=*(inImage+i*imageColTrue+j);//��ȡ��ɫͨ��������;
				*(inG+i*imageCol+j/3)=*(inImage+i*imageColTrue+j+1);//��ȡ��ɫͨ��������;
				*(inB+i*imageCol+j/3)=*(inImage+i*imageColTrue+j+2);//��ȡ��ɫͨ�������ݣ�
			}

		//��ȡˮӡ�Ĺ���;
		//�Ӳ�ɫͼ�񵽻Ҷ�ͼ���ת��;
		iGray=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		rgb2gray(iGray,inR,inG,inB,imageRow,imageCol);//��ɫͼ��ת��Ϊ�Ҷ�ͼ��洢��iGray�У�������������Ӷ�;
		//Ƕ��ˮӡ�Ĺ���;
		dctR=(float *)malloc(sizeof(float)*8*8);
		dctB=(float *)malloc(sizeof(float)*8*8);
		tempR=(float *)malloc(sizeof(float)*8*8);//�ڴ�����������ڴ洢��ɫͨ�����ݵ�һ���ֿ������;
		tempB=(float *)malloc(sizeof(float)*8*8);//�ڴ�����������ڴ洢��ɫͨ�����ݵ�һ���ֿ������;
		tempIGray=(float *)malloc(sizeof(float)*8*8);
		
		for(i=0;i<=imageRow-8;i=i+8)
			for (j=0;j<=imageCol-8;j=j+8)
			{
				//�������ݵ���ʱ�Ĵ洢�ֿ鵥Ԫ;
				for(p=i;p<i+8;p++)
					for (q=j;q<j+8;q++)
					{
						*(tempR+(p-i)*8+(q-j))=*(inR+p*imageCol+q);//��ɫͨ���ķֿ�����;
						*(tempB+(p-i)*8+(q-j))=*(inB+p*imageCol+q);//��ɫͨ���ķֿ�����;
						*(tempIGray+(p-i)*8+(q-j))=(float)*(iGray+p*imageCol+q);//�Ҷ�ͼ��ķֿ�����;

					}

					//�ֱ���DCT�任��
					dctBlock(dctR,tempR,8,8,8,8);
					dctBlock(dctB,tempB,8,8,8,8);
					//���c�Ĺ���;
					f=std2(tempIGray,8,8);
					c=18*0.5*(1+erf(f/theta/sqrt(2.0)))+0.5;
					//�����ͨ��������ֵ;
					energyR=0.0;
					energyB=0.0;
					for (flag=c;flag<64;flag++)
					{
						y=zig_zag(flag);
						energyR=energyR+*(dctR+y.row*8+y.col)*(*(dctR+y.row*8+y.col));
						energyB=energyB+*(dctB+y.row*8+y.col)*(*(dctB+y.row*8+y.col));
					}

					//��ȡˮӡ����;
					if (index>=waterRowTrue*waterColTrue){ return;}
					*(outImage+index)=energyR>energyB?255:0;
					index++;

			}//ˮӡ����ȡ����������ɣ�


	}
	else//˵������ͼ��Ϊ�Ҷ�ͼ�񣬶ԻҶ�ͼ����д���
	{
		inImageGray=(unsigned char *)malloc(sizeof(unsigned char)*imageRow*imageCol);
		if (ifAlign==1)//��ʾ�ô洢ʱ���ֽڶ���ķ�ʽ�洢��;
		{
			// ������ʵͼ�������������;
			imageRowTrue=imageRow; //�����ֽڵ�����;��������Ҫ�ı䣻
			imageColTrue=((imageCol+3)/4)*4;//�����ֽڵ�����;
		} 
		else//��ʾ�ô洢�������ֽڶ��뷽ʽ��;
		{
			// ������ʵͼ�������������;
			imageRowTrue=imageRow; //�����ֽڵ�����;��������Ҫ�ı䣻
			imageColTrue=imageCol;//�����ֽڵ�����;
		}
		//��ȡ����;
		for(i=0;i<imageRow;i++)
			for (j=0;j<imageCol;j++)
			{
				*(inImageGray+i*imageCol+j)=*(inImage+i*imageColTrue+j); //��ȡ�Ҷ�ͼ������ݣ�
			}

		//��ȡˮӡ����;
		dctFirst=(float *)malloc(sizeof(float)*8*8);
		dctSecond=(float *)malloc(sizeof(float)*8*8);
		tempFirst=(float *)malloc(sizeof(float)*8*8);//�ڴ�����������ڴ洢��ɫͨ�����ݵ�һ���ֿ������;
		tempSecond=(float *)malloc(sizeof(float)*8*8);//�ڴ�����������ڴ洢��ɫͨ�����ݵ�һ���ֿ������;

		for(i=0;i<=imageRow-8;i=i+8)//�������ͼ����������������ܱ�8�����������������ұߺ������������8*8��С�Ŀ�;
			for (j=0;j<=imageCol-16;j=j+16)
			{
				//�������ݵ���ʱ�Ĵ洢�ֿ鵥Ԫ;
				for(p=i;p<i+8;p++)
					for (q=j;q<j+8;q++)
						*(tempFirst+(p-i)*8+(q-j))=*(inImageGray+p*imageCol+q);//��һ���ֿ�����;
				for(p=i;p<i+8;p++)
					for (q=j+8;q<j+16;q++)
						*(tempSecond+(p-i)*8+(q-j-8))=*(inImageGray+p*imageCol+q);//�ڶ����ֿ�����;

				//�ֱ���DCT�任��
				dctBlock(dctFirst,tempFirst,8,8,8,8);
				dctBlock(dctSecond,tempSecond,8,8,8,8);
				//���c�Ĺ���;
				f=(std2(tempFirst,8,8)+std2(tempSecond,8,8))/2;
				c=18*0.5*(1+erf(f/theta/sqrt(2.0)))+0.5;

				//�����ͨ��������ֵ;
				energyFirst=0.0;
				energySecond=0.0;
				for (flag=c;flag<64;flag++)
				{
					y=zig_zag(flag);
					energyFirst=energyFirst+*(dctFirst+y.row*8+y.col)*(*(dctFirst+y.row*8+y.col));
					energySecond=energySecond+*(dctSecond+y.row*8+y.col)*(*(dctSecond+y.row*8+y.col));
				}

				//��ȡˮӡ����;
				if (index>=waterRowTrue*waterColTrue){return;}
				*(outImage+index)=energyFirst>energySecond?255:0;
				index++;
			}
	}

	return;

}

