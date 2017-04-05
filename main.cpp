#include<stdlib.h>
#include<cmath>
#include<iostream>
#include<string>
#include<tiffio.h>

using namespace std;
typedef unsigned short uint16;
typedef unsigned char byte;


size_t getBufSize(TIFF* tif)
{
    int width,height;
    TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&width);
    TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&height);
    int depth=TIFFNumberOfDirectories(tif);
    return width*height*depth;
}


bool isLegal(TIFF* tif)
{
    uint16 samples_per_pixel,bits_per_sample;
    TIFFGetField(tif,TIFFTAG_SAMPLESPERPIXEL,&samples_per_pixel);
    TIFFGetField(tif,TIFFTAG_BITSPERSAMPLE,&bits_per_sample);
    return samples_per_pixel==1 && bits_per_sample==8;
}


TIFF* readTiffFile(const char* file_name)
{
    return TIFFOpen(file_name,"r");
}


byte* tiff2BytesArray(TIFF* tif)
{
    size_t buf_size=getBufSize(tif);
    byte* buf=(byte*)malloc(sizeof(byte)*buf_size),*pbuf=buf;
    int width,height;
    TIFFGetField(tif,TIFFTAG_IMAGEWIDTH,&width);
    TIFFGetField(tif,TIFFTAG_IMAGELENGTH,&height);
    
    do
    {
        for(int row=0;row<height;++row)
        {
            TIFFReadScanline(tif,pbuf,row);
            pbuf+=width;
        }
        
    }while(TIFFReadDirectory(tif));
    
    return buf;
    
}


double inline log2(double x)
{
    static const double l2=log(2); 
    return log(x)/l2;
}

double normalDistance(double ma[256],double mb[256],double mab[256][256])
{
    
    double a=0.0,b=0.0,c=0.0;
    for(int i=0;i<256;++i)
    {
        double p=ma[i];
        if(p==0.0)
        {
            continue;
        }
        for(int j=0;j<256;++j)
        {
            double q=mb[j],pq=mab[i][j];
            if(q!=0.0 && pq!=0.0)
            {
                a-=pq*log2(pq/p);
                b-=pq*log2(pq/q);
                c-=pq*log2(pq);
            }
            
        }
    }
    
    return (a+b)/c;
}


double calSimilarity(byte* ba,byte* bb,size_t bytes)
{
    byte* end=ba+bytes;
    double ma[256]={0};
    double mb[256]={0};
    double mab[256][256]={0};
    for(byte* p=ba,*q=bb;p!=end;++p,++q)
    {
        ma[*p]++,mb[*q]++,mab[*p][*q]++;
    }
    for(int i=0;i<256;++i)
    {
        ma[i]/=bytes;
        mb[i]/=bytes;
    }
    for(int i=0;i<256;++i)
    {
        for(int j=0;j<256;++j)
        {
            mab[i][j]/=bytes;
        }
    }
    double distance=normalDistance(ma,mb,mab);
    return 1.0-distance;
}


int main(int argc,char* argv[])
{
    TIFFSetWarningHandler(0);
    //stderr=0;
    if(argc!=3)
    {
        cout<<"two tiff-image-file names required!"<<endl;
        return 1;
    }
    
    TIFF *ta=readTiffFile(argv[1]),*tb=readTiffFile(argv[2]);
    if(!ta || !tb)
    {
        cout<<"fail to read tiff file,please check the file name!"<<endl;
        return 1;
    }
    
    if(!isLegal(ta) || !isLegal(tb))
    {
        cout<<"not support this kind of tiff image yet!"<<endl;
        return 1;
    }
    
    int bytes=getBufSize(ta);
    if(bytes!=getBufSize(tb))
    {
        cout<<"these two tiff images has not the same size!"<<endl;
        return 1;
    }
    
    byte *ba=tiff2BytesArray(ta),*bb=tiff2BytesArray(tb);
    
    cout<<calSimilarity(ba,bb,bytes)<<endl;
    
    free(ba);
    free(bb);
    
    TIFFClose(ta);
    TIFFClose(tb);
    
    return 0;
}
