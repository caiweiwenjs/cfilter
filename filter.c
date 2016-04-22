#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "watermark.h"

//bmp头
typedef struct tagBITMAPFILEHEADER
{
	unsigned short  bfType;
	unsigned int    bfSize;
	unsigned short  bfReserved1;
	unsigned short  bfReserved2;
	unsigned int    bfOffBits;
}__attribute__((packed))BITMAPFILEHEADER;
 
typedef struct tagBITMAPINFOHEADER
{
	unsigned int    biSize;
	unsigned int    biWidth;
	unsigned int    biHeight;
	unsigned short  biPlanes;
	unsigned short  biBitCount;
	unsigned int    biCompression;
	unsigned int    biSizeImage;
	unsigned int    biXPelsPerMeter;
	unsigned int    biYPelsPerMeter;
	unsigned int    biClrUsed;
	unsigned int    biClrImportant;
}__attribute__((packed))BITMAPINFOHEADER;

/* 
 * typedef struct tagRGBQUAD
 * {
 * 	unsigned char   rgbBlue;
 * 	unsigned char   rgbGreen;
 * 	unsigned char   rgbRed;
 * 	unsigned char   rgbReserved;
 * 	}__attribute__((packed))RGBQUAD;
 */


void	Setup(void);
void	StartPage(ppd_file_t *ppd, cups_page_header2_t *header);
void	Shutdown(void);
int 	Rle_Encode_O(unsigned char *inbuf, int inSize, unsigned char *outbuf, int outBufSize);
int 	Cups_Compress(unsigned char *inbuf,int inSize, unsigned char *outbuf);
int 	RLE_AA(unsigned char *inbuf,int inSize, unsigned char *outbuf);

/*
 *  * 'Setup()' - Prepare the printer for printing.
 */

void	Setup(void)
{
 /*
  *   * Send a PCL reset sequence.
  */
	putchar(0x1B);
        putchar(0x25);
	putchar(0x2d);
	printf("12345X");
	putchar(0x1b);
	putchar('E');
}


/*
 *  * 'StartPage()' - Start a page of graphics.
 */

void	StartPage(	ppd_file_t         *ppd,		/* I - PPD file */
					cups_page_header2_t *header)	/* I - Page header */
{
	/*
  	 * Show page device dictionary...
  	 */
/*
	fprintf(stderr, "DEBUG: StartPage...\n");
	fprintf(stderr, "DEBUG: MediaClass = \"%s\"\n", header->MediaClass);
	fprintf(stderr, "DEBUG: MediaColor = \"%s\"\n", header->MediaColor);
	fprintf(stderr, "DEBUG: MediaType = \"%s\"\n", header->MediaType);
	fprintf(stderr, "DEBUG: OutputType = \"%s\"\n", header->OutputType);

	fprintf(stderr, "DEBUG: AdvanceDistance = %d\n", header->AdvanceDistance);
	fprintf(stderr, "DEBUG: AdvanceMedia = %d\n", header->AdvanceMedia);
	fprintf(stderr, "DEBUG: Collate = %d\n", header->Collate);
	fprintf(stderr, "DEBUG: CutMedia = %d\n", header->CutMedia);
	fprintf(stderr, "DEBUG: Duplex = %d\n", header->Duplex);
	fprintf(stderr, "DEBUG: HWResolution = [ %d %d ]\n", header->HWResolution[0],
				header->HWResolution[1]);
	fprintf(stderr, "DEBUG: ImagingBoundingBox = [ %d %d %d %d ]\n",
				header->ImagingBoundingBox[0], header->ImagingBoundingBox[1],
				header->ImagingBoundingBox[2], header->ImagingBoundingBox[3]);
	fprintf(stderr, "DEBUG: InsertSheet = %d\n", header->InsertSheet);
	fprintf(stderr, "DEBUG: Jog = %d\n", header->Jog);
	fprintf(stderr, "DEBUG: LeadingEdge = %d\n", header->LeadingEdge);
	fprintf(stderr, "DEBUG: Margins = [ %d %d ]\n", header->Margins[0],
				header->Margins[1]);
	fprintf(stderr, "DEBUG: ManualFeed = %d\n", header->ManualFeed);
	fprintf(stderr, "DEBUG: MediaPosition = %d\n", header->MediaPosition);
	fprintf(stderr, "DEBUG: MediaWeight = %d\n", header->MediaWeight);
	fprintf(stderr, "DEBUG: MirrorPrint = %d\n", header->MirrorPrint);
	fprintf(stderr, "DEBUG: NegativePrint = %d\n", header->NegativePrint);
	fprintf(stderr, "DEBUG: NumCopies = %d\n", header->NumCopies);
	fprintf(stderr, "DEBUG: Orientation = %d\n", header->Orientation);
	fprintf(stderr, "DEBUG: OutputFaceUp = %d\n", header->OutputFaceUp);
	fprintf(stderr, "DEBUG: PageSize = [ %d %d ]\n", header->PageSize[0],
				header->PageSize[1]);
	fprintf(stderr, "DEBUG: Separations = %d\n", header->Separations);
	fprintf(stderr, "DEBUG: TraySwitch = %d\n", header->TraySwitch);
	fprintf(stderr, "DEBUG: Tumble = %d\n", header->Tumble);
	fprintf(stderr, "DEBUG: cupsWidth = %d\n", header->cupsWidth);
	fprintf(stderr, "DEBUG: cupsHeight = %d\n", header->cupsHeight);
	fprintf(stderr, "DEBUG: cupsMediaType = %d\n", header->cupsMediaType);
	fprintf(stderr, "DEBUG: cupsBitsPerColor = %d\n", header->cupsBitsPerColor);
	fprintf(stderr, "DEBUG: cupsBitsPerPixel = %d\n", header->cupsBitsPerPixel);
	fprintf(stderr, "DEBUG: cupsBytesPerLine = %d\n", header->cupsBytesPerLine);
	fprintf(stderr, "DEBUG: cupsColorOrder = %d\n", header->cupsColorOrder);
	fprintf(stderr, "DEBUG: cupsColorSpace = %d\n", header->cupsColorSpace);
	fprintf(stderr, "DEBUG: cupsCompression = %d\n", header->cupsCompression);
*/
	/*
  	 * Setup printer/job attributes...
  	 */

	printf("\033&l%dH",header->MediaPosition);       /* Input Slot */
	printf("\033&l%dX",header->NumCopies);           /* Copies */

	switch (header->PageSize[1])
    	{
		case 540 : /* Monarch Envelope */
			printf("\033&l80A");			/* Set page size */
		break;

		case 595 : /* A5 */
			printf("\033&l25A");			/* Set page size */
		break;

		case 624 : /* DL Envelope */
			printf("\033&l90A");			/* Set page size */
		break;

		case 649 : /* C5 Envelope */
			printf("\033&l91A");			/* Set page size */
		break;

		case 684 : /* COM-10 Envelope */
			printf("\033&l81A");			/* Set page size */
		break;

		case 709 : /* B5 Envelope */
			printf("\033&l100A");			/* Set page size */
		break;

		case 756 : /* Executive */
			printf("\033&l1A");			/* Set page size */
		break;

		case 792 : /* Letter */
			printf("\033&l2A");			/* Set page size */
		break;

		case 842 : /* A4 */
			printf("\033&l26A");			/* Set page size */
		break;

		case 1008 : /* Legal */
			printf("\033&l3A");			/* Set page size */
		break;

		case 1191 : /* A3 */
			printf("\033&l27A");			/* Set page size */
		break;

		case 1224 : /* Tabloid */
			printf("\033&l6A");			/* Set page size */
		break;
	}
	printf("\033*t%dR",header->HWResolution[1]);     /* Vertical Resolution */
	printf("\033*r%dS",header->cupsWidth);           /* Width */
	printf("\033*r%dT",header->cupsHeight);          /* Height */
	printf("\033*r1U");                             /* Color */

}

/*
 *  * 'Shutdown()' - Shutdown the printer.
 */

void	Shutdown(void)
{
	/*
  	 * Send a PCL reset sequence.
  	 */
	putchar(0x1b);
	putchar('E');
	putchar(0x1B);
	putchar(0x25);
	putchar(0x2d);	
	printf("12345X");
}


/*
 *  * Rle_Encode_O inbuf-上一级处理完成的数据。inSize-inbuf的大小 outbuf-压缩完成后的数据 outBufSize-outbuf的大小
 */
int Rle_Encode_O(unsigned char *inbuf, int inSize, unsigned char *outbuf, int outBufSize)
{
	unsigned char *src = inbuf;
	int i;
	int encSize = 0;
	int srcLeft = inSize;
	unsigned char num, *temp;

	while((*src != 128)&&(srcLeft>0)) //判断是否完成压缩，128为标志
	{
	
		int count = 0;
		num = *src;
		temp = src;

		if ((*(src+1) == num) && (*(src+2) == num)) //是否有连续三个相同数据
		{
			
			while((*(temp+1) == num) && (*(temp+2) == num)&&(count<125))
			{
				temp++;
				count++;
			}
			count=count+2;
			if((encSize + 2) > outBufSize) 
			{
				return -1;
			}

			outbuf[encSize++] = count | 0x80; //加标志位后写入重复数据的个数
			outbuf[encSize++] = *src; //写入重复数据
			src += count;
			srcLeft -= count;
		} 
		else //不重复的
		{
			while ((count<srcLeft)&&(count<125))
			{
				if (*(temp+1) == *(temp)&&(*temp !=128))
				{
					if (*(temp+2) != *(temp)&&(*temp !=128))
					{
						temp+=2;
						count+=2;
					}
					else break;
				}
				else
				{
					temp++;
					count++;
				}
			}
			
			if((encSize + count + 1) > outBufSize)
			{
				return -1;
			}
			outbuf[encSize++] = count; //写入重复数据的个数
			for(i = 0; i < count; i++) //依次写入不重复数据
			{
				outbuf[encSize++] = *src++;
			}
			srcLeft -= count;
		}
	}
	
	return encSize; //返回outbuf的大小
}

/*
 *  * Cups_Compress inbuf-上一级处理完成的数据。inSize-inbuf的大小 outbuf-压缩完成后的数据
 */
int Cups_Compress(unsigned char *inbuf,int inSize, unsigned char *outbuf)
{
	unsigned char	*src=inbuf;
	int	i;
	int 	encSize = 0,
		srcLeft=inSize;
	unsigned char *temp;
		
	while(srcLeft>0) //数据是否压缩完成
	{

		int count = 0;
		temp = src;

		if (*(temp+1) == *temp) //连续两个数据相同
		{
			while((*(temp+1) == *temp) &&(count<127)&&(count+1<srcLeft))
			{
				temp++;
				count++;
			}
			outbuf[encSize++] = count; //写入重复数据的个数
			outbuf[encSize++] = *src; //写入重复数据
			src+=count+1;
			srcLeft -= count+1;
		}
		else //不重复的
		{
			while((*(temp+1) != *temp) &(count<128)&&(srcLeft>0))
			{
				temp++;
				count++;
				srcLeft--;
			}
			outbuf[encSize++] = 257-count; //将（257-重复数据的个数）写入
			for(i = 0; i < count; i++) //依次写入不重复数据
			{
				outbuf[encSize++] = *src++;
			}
			//srcLeft -= count;
		}
	}
	return encSize; //返回outbuf的大小
}

/*
 *  * RLE_AA inbuf-上一级处理完成的数据。inSize-inbuf的大小 outbuf-压缩完成后的数据
 */
int RLE_AA(unsigned char *inbuf,int inSize, unsigned char *outbuf)
{
	unsigned char	*src= inbuf;
	int	i;
	int encSize = 0,
		srcLeft=inSize;
	unsigned char *temp;
		
	while(srcLeft > 0)//数据是否压缩完成
	{
		int count = 0;
		temp = src;

		if ((*(temp+1) == *temp)&&(*(temp+2)==*temp)&&(srcLeft>2)) //连续三个数据相同
		{
			while((*(temp+1) == *temp)&&(*(temp+2)==*temp)&&(count<125)&&((count+2)<srcLeft))
			{
				temp++;
				count++;
			}
			if (*src==0xAA) //重复数据是AA的情况
			{
				outbuf[encSize++] = 0xAA;
				outbuf[encSize++] = (count+2)|0x80;
				outbuf[encSize++] = 0xAA;
			} 
			else
			{
				outbuf[encSize++] = 0xAA;
				outbuf[encSize++] = (count+2)|0x00;
				outbuf[encSize++] = *src;
			}
			src+=count+2;
			srcLeft-=count+2;
		}
		else if(*temp==0xAA) //只有一个数据为AA
		{
			if(*(temp+1)==0xAA)
			{
				outbuf[encSize++] = 0xAA;
				outbuf[encSize++] = 0x82;
				outbuf[encSize++] = 0xAA;
				src+=2;
				srcLeft-=2;
			}
			else //连续两个数据为AA
			{
				outbuf[encSize++] = 0xAA;
				outbuf[encSize++] = 0x81;
				outbuf[encSize++] = 0xAA;
				src++;
				srcLeft--;
			}
		}
		else if((*(temp+1) == *temp)&&(*(temp+2) == *temp)&&(srcLeft<3)) //只有两个连续数据相同
		{
			if(srcLeft==2)
			{
				outbuf[encSize++] = *src++;
				outbuf[encSize++] = *src++;
				srcLeft-=2;
			}
			else if(srcLeft==1)
			{
				outbuf[encSize++] = *src++;
				srcLeft--;
			}
		}
		else //不重复的
		{
			while((srcLeft>0)&&(*temp!=0xAA))
			{
				if (*(temp+1) == *temp)
				{
					if(*(temp+2) != *temp)
					{	
						temp+=2;
						count+=2;
						srcLeft-=2;
					}
					else break;
				}
				else
				{
					temp++;
					count++;
					srcLeft--;
				}
			}
			for(i = 0; i < count; i++)
			{
				outbuf[encSize++] = *src++;
			}
		}
	}
	return encSize;
}

int main(int argc,char *argv[])
{
	cups_raster_t           *ras;

	cups_page_header2_t     header;
	ppd_file_t							*ppd;		/* PPD file */
	int                     i , j , q = 0,offbit=0, page=0, flag=1, fd;
	int                     x, y, length;
	unsigned char           *buffer,*buffertemp,*buf, *halftonebuf, *halftonebuftemp, *buf_wm, *buf_head, *buf_ex, rgb[1024], *userbuf;
	char										path[20], path_ex[20];
	FILE                    *fp, *fp_user, *fp_wm, *fp_bug, *fp_ex;
	clock_t 								start_16, end_16;

	int			num_options;	/* Number of print options */
	cups_option_t		*options;	/* Print options */
	const char		*val;		/* Option value */ 

	//redirect stderr
	if (freopen("/tmp/err.txt", "wb", stderr)==NULL) 
		fprintf(stderr, "error redirecting stderr\n");

	/*	
	userbuf=(unsigned char*)malloc(50);

	fp_user=fopen("/tmp/user.txt","r");
	if(fp_user==NULL)
	{
		fprintf(stderr,"user.txt cannot read\n");
		exit(0);
	}
	fread(userbuf,1,50,fp_user);

	char* str1;
	str1=strstr((char *)userbuf,argv[2]);
	if(str1==NULL)
	{
		fprintf(stderr, "Not a legal user!\n");	
		fflush(stderr);
		exit(1);
	}
	*/

	if (argc < 6 || argc > 7)
	{
		/*
  	 	 * We don't have the correct number of arguments; write an error message and return.
   	 	 */
		fprintf(stderr, "Input like this: job-id user title copies options [file]\n");
		exit(1);
	}

	/*
   	 * Open the page stream...
   	 */

	if (argc == 7)
	{
		if ((fd = open(argv[6], O_RDONLY)) == -1)
		{
			fprintf(stderr, "Unable to open raster file.\n");
			exit(1);
		}
	}
	else
		fd = 0;
 	

 	
//	ras=cupsRasterOpen(fd, CUPS_RASTER_READ);

	ppd = ppdOpenFile(getenv("PPD"));
	if (!ppd)
	{
		ppd_status_t	status;		/* PPD error */
		int				linenum;	/* Line number */

		fprintf(stderr, "The PPD file could not be opened.\n");

		status = ppdLastError(&linenum);

		fprintf(stderr, "DEBUG: %s on line %d.\n", ppdErrorString(status), linenum);

		return (1);
	}

	Setup();

	ras=cupsRasterOpen(0, CUPS_RASTER_READ);

	while (cupsRasterReadHeader2(ras, &header))
	{
		options     = NULL;
		num_options = cupsParseOptions(argv[5], 0, &options);
		
		/* 
  	 	 * Setup this page
   	 	 */
		//page ++;
		//fprintf(stderr, "PAGE: %d %d\n", page, header.NumCopies);
		//fflush(stderr);	
	
		/*
  	 	 * Start the page...
  	 	 */
		if (flag==1)
		{
			StartPage(ppd, &header);
			flag=0;
		}
		printf("\033*r0A");					/* Start raster image */

		
		//sprintf(path,"/tmp/%d.bmp",page);
		/*
		fp=fopen("/tmp/origin.bmp","wb+");
		if(fp==NULL)
		{
			fprintf(stderr,"%d.txt cannot open\n",page);
			exit(1);
		}
					
		BITMAPFILEHEADER        bmpfileheader;
		BITMAPINFOHEADER        bmpinfoheader;
		//RGBQUAD                 *bmprgb;
		*/
		/*
		 * Define BITMAPFILEHEADER
		 */
		/*
		bmpfileheader.bfType=0x4D42;
		bmpfileheader.bfSize=header.cupsHeight*header.cupsWidth;
		bmpfileheader.bfReserved1=0x0000;
		bmpfileheader.bfReserved2=0x0000;
		bmpfileheader.bfOffBits=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER);
		*/
		/*
		 * Define BITMAPINFOHEADER
		 */
		/*
		bmpinfoheader.biSize=sizeof(BITMAPINFOHEADER);
		bmpinfoheader.biWidth=header.cupsWidth;
		bmpinfoheader.biHeight=-header.cupsHeight;
		bmpinfoheader.biPlanes=0x0001;
		bmpinfoheader.biBitCount = header.cupsBitsPerPixel;  
		bmpinfoheader.biCompression = 0;  
		bmpinfoheader.biSizeImage = 0;  
		bmpinfoheader.biXPelsPerMeter = header.HWResolution[0]*3.2808398950131;  
		bmpinfoheader.biYPelsPerMeter = header.HWResolution[1]*3.2808398950131;  
		bmpinfoheader.biClrUsed = 0;  
		bmpinfoheader.biClrImportant = 0;  
		*/
		
		/*
		 * Define RGBQUAD
		 */
		/*
		j=0;
		for ( i = 255; i>=0; i-- ) 
		{
			rgb[j] = i;
			rgb[++j] = i;
			rgb[++j] = i;
			rgb[++j] = 0;
			++j;
		}
	
		//fwrite BITMAPFILEHEADER & BITMAPINFOHEADER & RGBQUAD
		fwrite(&bmpfileheader,1,sizeof(BITMAPFILEHEADER),fp);
		fwrite(&bmpinfoheader,1,sizeof(BITMAPINFOHEADER),fp);
		fwrite(&rgb,1,1024,fp);
		*/
		
		/*
		 * Read Pixels
		 */
		if(header.cupsBytesPerLine%8!=0)
			length=header.cupsBytesPerLine/8+1;
		else   
			length=header.cupsBytesPerLine/8;
		
		buf		= (unsigned char*)malloc(header.cupsBytesPerLine);
		buffer		= (unsigned char*)malloc(header.cupsBytesPerLine*header.cupsHeight);
		halftonebuf	= (unsigned char*)malloc(length*header.cupsHeight);
		
		offbit=0;
		for (y = 0; y < header.cupsHeight; y ++)
		{
			cupsRasterReadPixels(ras, buf, header.cupsBytesPerLine);
			//fwrite(buf,1,4960,fp);
			memcpy(buffer+offbit, buf , header.cupsBytesPerLine);
			offbit		= offbit + header.cupsBytesPerLine;
		}
		
		/*
		 * Watermark
		 */

		if ((val = cupsGetOption("dewwm", num_options, options)) != NULL)
		{
	
			int result = strcmp( val, "true" );
		
			if(result==0)
			{
				//embed watermark
				buf_wm	= (unsigned char*)malloc(4096);
				buf_head= (unsigned char*)malloc(1078);
			//	buf_ex	= (unsigned char*)malloc(4096);
		
				fp_wm	= fopen("/tmp/watermark.bmp","rb+");
				if(fp_wm==NULL)
				{                         
					fprintf(stderr,"watermark cannot open\n");
					exit(1);
				}
		
				fread(buf_head,1,1078,fp_wm);
		
				int seekerr =fseek(fp_wm,1078,0);
				if(seekerr)
				{
					fprintf(stderr,"Fseek failed\n");
					exit(1);
				}
		
				fread(buf_wm,1,4096,fp_wm);
		
				dewWatermarkEmbed(buffer, buf_wm, header.cupsHeight, length, 64, 64, 0, 1);
				
				//***************************************************************************EXTRACT
			/*	
				sprintf(path_ex,"/tmp/ex%d.bmp",page);
		
				fp_ex=fopen(path_ex,"wb+");
				if(fp_ex==NULL)
				{
					fprintf(stderr,"ex%d cannot open\n",page);
					exit(1);
				}
		
				dewWatermarkExtract(buf_ex, buffer, header.cupsHeight, length, 64, 64, 0, 1);
				fwrite(buf_head,1,1078,fp_ex);
				fwrite(buf_ex,1,4096,fp_ex);
			*/	
				free(buf_wm);	
				free(buf_head);	
			//	free(buf_ex);
				fclose(fp_wm);
			//	fclose(fp_ex);
			}
		}	

		//fprintf(stderr,"pass dewwm\n");fflush(stderr);
		
		/*
		 * * HalfTone
		 */ 
		 
		start_16 = clock();
		
		if ((val = cupsGetOption("halftone", num_options, options)) != NULL)
		{
			int bayer = strcmp( val, "bayer" );
			int floyd = strcmp( val, "floyd" );
			
			unsigned char	num,a[8];
			float		e,f;
			int		i=0,j=1;
			buffertemp	=buffer;
			halftonebuftemp	=halftonebuf;
			*halftonebuftemp	=0;

			//fprintf(stderr,"pass val\n");fflush(stderr);	

			if(bayer==0)
			{
				/*
			 	 * * Bayer
		 		 */ 	
				char	BayerPattern[8][8]={	0,32,8,40,2,34,10,42,
								48,16,56,24,50,18,58,26,
								12,44,4,36,14,46,6,38,
								60,28,52,20,62,30,54,22,
								3,35,11,43,1,33,9,41,
								51,19,59,27,49,17,57,25,
								15,47,7,39,13,45,5,37,
								63,31,55,23,61,29,53,21};
			
				for(y = 0; y< header.cupsHeight; y ++)
				{
					for(x = 0; x< header.cupsBytesPerLine; x ++)
				  	{
						num=(unsigned char)*buffertemp++;
						i=x&7;
		
					   	 if ( (num>>2) > BayerPattern[y&7][x&7]) 
							a[i]=(unsigned char)1; 
						else 
							a[i]=(unsigned char)0;
						
						a[i]=a[i]<<(7-i);
						*halftonebuftemp = (*halftonebuftemp)|a[i];
						if((i==7)&&(j< header.cupsHeight*length))	
						{
							j++;
							halftonebuftemp++;
							*halftonebuftemp=0;
						}
					}
					if(j<header.cupsHeight*length)
					{	
						j++;
						halftonebuftemp++;
						*halftonebuftemp=0;
					}
					//fprintf(stderr,"pass bayer\n");fflush(stderr);
				}
			}
			else if(floyd==0)
			{
				/*
		 		 * * Floyd
		 		 */ 
				//fprintf(stderr,"pass memcpy\n");fflush(stderr);

				for(y=0;y<header.cupsHeight;y++)
				{
					for(x=0;x<header.cupsBytesPerLine;x++)
					{
						num=(unsigned char)*buffertemp++;
						i=x&7;
						e=0.0;
						f=0.0;					
	
			      			if ( num > 128 )
			      			{	 
					   		a[i]=(unsigned char)1;
						   	e=(float)(num-255.0);
					      	}
					     	else
			   			{
			      				a[i]=(unsigned char)0;
			      				e=(float)num;
			      			}
			      			if(x<header.cupsBytesPerLine-1)
			     		 	{
			      				f=(float)*(buffertemp+1);
			      				f+=(float)( (7.0/16.0) * e);
			        			*(buffertemp+1)=(int)f;
						}	
						if((x<header.cupsBytesPerLine-1)&&(y<header.cupsHeight-1))
						{
							f=(float)*(buffertemp+header.cupsBytesPerLine+1);
							f+=(float)( (1.0/16.0) * e);
							*(buffertemp+header.cupsBytesPerLine+1)=(int)f;
						}
						if(y<header.cupsHeight-1)
						{
							f=(float)*(buffertemp+header.cupsBytesPerLine);
							f+=(float)( (3.0/16.0) * e);
							*(buffertemp+header.cupsBytesPerLine)=(int)f;
							
							f=(float)*(buffertemp+header.cupsBytesPerLine-1);
							f+=(float)( (3.0/16.0) * e);
							*(buffertemp+header.cupsBytesPerLine-1)=(int)f;
						}
						
						a[i]=a[i]<<(7-i);
						*halftonebuftemp = (*halftonebuftemp)|a[i];
						if((i==7)&&(j< header.cupsHeight*length))	
						{
							j++;
							halftonebuftemp++;
							*halftonebuftemp=0;
						}
					}	
					if(j<header.cupsHeight*length)
					{
						j++;
						halftonebuftemp++;
						*halftonebuftemp=0;
					}
		  	}
				//fprintf(stderr,"pass floyd\n");fflush(stderr);
			}
			else
				fprintf(stderr,"Halftone Option get wrong\n");
		}	
		
		end_16	= clock();
		fprintf(stderr, "16 pages halftone time is:%f\n",(double)(end_16 - start_16) / CLOCKS_PER_SEC);
	//	fwrite(halftonebuf,1,header.cupsHeight*length,fp);
		
		//fprintf(stderr,"pass halftone\n");fflush(stderr);
		
		/*
		 * * Compress
		 */ 
		if ((val = cupsGetOption("compress", num_options, options)) != NULL)
		{
			int rle = strcmp( val, "rle" );
			int cups = strcmp( val, "cups" );
			
			unsigned char *compressbuf,*compresstemp;
			int size;
			compressbuf	=(unsigned char*)malloc(length*2);
			compresstemp =(unsigned char*)malloc(length);
			offbit=0;
			clock_t start_rle, end_rle;

			//FILE *pcl;
			//pcl=fopen("/tmp/pcl1.txt","wb");
			
			for(y = 0; y < header.cupsHeight; y ++)
			{
				memcpy(compresstemp, halftonebuf+offbit , length);
				//*(compresstemp+length)=128;
				offbit          = offbit + length;
				
				if(rle==0)
				{
					size=Rle_Encode_O(compresstemp, length, compressbuf,length*2);
				}
				else if(cups==0)
				{
					size=Cups_Compress(compresstemp, length, compressbuf);
				}
				else
				fprintf(stderr,"Compress Option get wrong\n");
				
				printf("\033*b%dW",size);	/* Raster image data */
				fwrite(compressbuf,1,size,stdout);
				//fwrite(compressbuf,1,size,pcl);
			}
			
		}		
		//fprintf(stderr,"pass compress\n");fflush(stderr);	

		printf("\033*rC");		/* Raster image data end */
		
		
		free(buf);
		free(buffer);
		free(halftonebuf);
		//fclose(fp);
		
	}
		
	/*
	 * Shutdown the printer...
	 */
	Shutdown();
		
	if (ppd)
		ppdClose(ppd);
	
	cupsRasterClose(ras);
	
	if (fd != 0)
		close(fd);
	/*	
	free(buf);
	free(buffer);
	free(halftonebuf);
	fclose(fp);
	*/
	fclose(stderr);
	fclose(stdout);	
	return(0);
}

