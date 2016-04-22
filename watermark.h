#ifndef H_WATERMARK
#define H_WATERMARK

#define NO_WATERMARK 0
#define BP_WATERMARK 1
#define DEW_WATERMARK 2

void dewWatermarkEmbed(unsigned char *inImage, unsigned char *watermark,int imageRow,int imageCol,int waterRow,int waterCol,unsigned char ifColor,unsigned char ifAlign);
void dewWatermarkExtract(unsigned char *outImage,const unsigned char *inImage,int imageRow,int imageCol,int waterRow,int waterCol,unsigned char ifColor,unsigned char ifAlign);
void bpWatermarkEmbed(unsigned char *imgPixelStream, unsigned char *wmPixelStream, int imgHeight, int imgWidth, int wmHeight, int wmWidth, int ifColor, int flag);
void bpWatermarkExtract(unsigned char *wmPixelStream, unsigned char *wmImgPixelStream, int wmImgHeight, int wmImgWidth, int wmHeight, int wmWidth, int ifColor, int flag);
#endif