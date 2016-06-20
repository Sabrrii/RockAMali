#include "CImg.h"

using namespace cimg_library;

int main()
{
  int width=256;
  int height=128;
  int zoom=6;int step=zoom;
  CImg<unsigned char> image(width,height,1,4);
  const unsigned char red[] = { 255,0,0,255 }, green[] = { 0,255,0,255 }, blue[] = { 0,0,255,255 },
    cP128[] = { 64,64,64,255 }, cP64[] = { 128,128,128,255 }, cP32[] = { 196,196,196,255 }, cP16[] = { 222,222,222,255 }, cP8[] = { 242,242,242,255 },
    black[] = { 0,0,0,255 }, white[] = { 222,222,222,255 };
  //partition color
  {
  image.fill(255);//Pall
  int i=0,j=127;
  image.draw_rectangle(i,0,j,height,cP128);
  i=j+1;j+=64;
  image.draw_rectangle(i,0,j,height,cP64);
  i=j+1;j+=32;
  image.draw_rectangle(i,0,j,height,cP32);
  i=j+1;j+=16;
  image.draw_rectangle(i,0,j,height,cP16);
  i=j+1;j+=8;
  image.draw_rectangle(i,0,j,height,cP8);
  }
  //partition sum
  image.draw_line(0,0,image.width()-1,0,black);
  image.draw_line(0,0,0,image.height()-1,black);
  //set few points (random and a double for error)
  int nb=10;
  CImg<unsigned char> ti(nb);
  CImg<unsigned char> tj(nb);
  ti.rand(1,width-1);
  tj.rand(1,height-1);
  //introduce double (LUT error)
  tj(1)=tj(4);
  ti.print("ti");
  tj.print("tj w 1 dlb");
  cimg_forX(ti,n)
  {
    int i=ti(n);
    int j=tj(n);
    CImg<unsigned char>  color(1,1,1,4);color.draw_point(0,0,green);
    color(3)=j;
    image.draw_point(i,j,color.data());//IP
    if(image(i,0,0,0)==black[0]&&image(i,0,0,1)==black[1]&&image(i,0,0,2)==black[2]) image.draw_point(i,0,color.data()); else {image.draw_point(i,0,red);image.draw_point(0,0,red);}//sum
    if(image(0,j,0,0)==black[0]&&image(0,j,0,1)==black[1]&&image(0,j,0,2)==black[2]) image.draw_point(0,j,color.data()); else {image.draw_point(0,j,red);image.draw_point(0,0,red);}//sum
  }
  //GUI
  //set other point(s) by mouse
  CImgDisplay disp(image.width()*zoom,image.height()*zoom);
  disp.set_title("(un)select point(s), exit on any key or window closing.");
  while(true)
  {
    //get point
    CImg<int> point=image.get_select(disp,0,0,true);
    point.print("point");
    int i=point(0);
    int j=point(1);
    //break on key pressed or display close
    if(i==-1||j==-1) break;
    //check point
    if(image(i,j,0,0)==green[0]&&image(i,j,0,1)==green[1]&&image(i,j,0,2)==green[2])
    {//remove point
      image.draw_point(i,j,black);
      continue;
    }
    //else add point
    unsigned char color[4];strcpy((char*)color,(char*)green);
    color[3]=j;
    image.draw_point(i,j,color);
    if(image(i,0,0,0)==black[0]&&image(i,0,0,1)==black[1]&&image(i,0,0,2)==black[2]) image.draw_point(i,0,color); else {image.draw_point(i,0,red);image.draw_point(0,0,red);}//sum
    if(image(0,j,0,0)==black[0]&&image(0,j,0,1)==black[1]&&image(0,j,0,2)==black[2]) image.draw_point(0,j,color); else {image.draw_point(0,j,red);image.draw_point(0,0,red);}//sum
  }//point selection by hand
  disp.close();
  //zoom
  image.resize(-zoom*100,-zoom*100);
  //draw separations
  int stepx=step;
  int stepy=step;
  //draw horizontal lines
  for(int y=0; y<(int)(image.height()); y+=stepy)
    image.draw_line(0,y,image.width()-1,y,black);
  //draw vertical lines
  for(int x=0; x<(int)(image.width()); x+=stepx)
    image.draw_line(x,0,x,image.height()-1,black);
  image.display("zoom 6x");
  image.save("claLUT.png");
  return 0;
}
