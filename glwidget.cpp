#include "glwidget.h"
#include <QtGui>
#include <QtOpenGL>
#include <vector>
#include <iostream>
#include "Vector3.h"
#include <windows.h>
//#include "ShaderSetup.h"
#include <algorithm>

using namespace std;

#define NO_MOTION			0
#define ZOOM_MOTION			1
#define ROTATE_MOTION		2
#define TRANSLATE_MOTION	3
#define use_glew 1

////////////////////////////////test
#define VOLUME_TEX_SIZE 128
#define MAX_KEYS 256
#define WINDOW_SIZE 800
#define normal_compute 0

bool toggle_visuals = true;

GLuint renderbuffer; 
GLuint framebuffer; 
GLuint volume_texture; // the volume texture
GLuint normal_texture; // the volume texture
GLuint normal_texture2; // the volume texture
GLuint backface_buffer; // the FBO buffers
GLuint final_image;
GLuint transfer_alpha;
GLuint transfer_color;
GLfloat LightDiffuse2[]=	{ 0.0f, 0.0f, 1.0f, 0.0f };
GLfloat LightSpecular2[]=	{ 1.0f, 1.0f, 1.0f, 0.0f };
//GLfloat LightPosition[]=    { 5.0f, 0.0f, 5.0f, 0.0f};


GLubyte readImage[400][400][4]; 
GLubyte texImage[256][256][4]; 
GLubyte texImage2[256][512][4];
GLuint programObject;
//GLuint SetupGLSL(char*);

GLuint g_LocShaderDebugSqueezeFactor;
GLfloat g_DebugSqueezeFactor = 1.0f;
extern Grid g;
float point[8]={0.0,0.0,1.0,0.0,1.0,1.0,0.0,1.0};
GLfloat projmat[16];
GLfloat transferFunc[9*4] =
{
	0.0, 0.0, 0.0, 0.0, 
	1.0, 0.0, 0.0, 1.0, 
	1.0, 0.5, 0.0, 1.0, 
	1.0, 1.0, 0.0, 1.0, 
	0.0, 1.0, 0.0, 1.0, 
	0.0, 1.0, 1.0, 1.0, 
	0.0, 0.0, 1.0, 1.0, 
	1.0, 0.0, 1.0, 1.0, 
	0.0, 0.0, 0.0, 0.0 
};

/////////////////gpu use////////////

struct cudaGraphicsResource *cuda_pbo_resource; // CUDA Graphics Resource (to transfer PBO)
float invViewMatrix[12];
GLfloat modelView[16];
GLfloat Projection[16];
float brightness = 1.0f;
float transferOffset = 0.0f;
float transferScale = 1.0f;

extern "C" void render_kernel(dim3 gridSize, dim3 blockSize, uint *d_output, uint imageW, uint imageH,
                              float density, float brightness, float transferOffset, float transferScale, 
							  cudaExtent volumeSize, float min_value, float max_value, float gridScale_X, float gridScale_Y, float gridScale_Z,float tstep);
extern "C" void copyInvViewMatrix(float *invViewMatrix, size_t sizeofMatrix);
extern "C" void TransferFunc(float *transf_color, int num_color, float *transf_alpha, int num_alpha);
extern "C" void freeCudaVolumeBuffers();
extern "C" void freeCudaTransferFuncBuffers();
extern "C" void gmm_compute_kernel(dim3 gridSize, dim3 blockSize,int size, float *gmm_V,float *gmm_P, cudaExtent volumeSize, int xdim, int ydim, int zdim, int distib_num);

dim3 blockSize(32, 32);
dim3 gridSize;

float maxpos_z;
float minpos_z;

int iDivUp(int a, int b)
{
    return (a % b != 0) ? (a / b + 1) : (a / b);
}
//////////////////////////////

GLWidget::GLWidget(const QGLFormat& format, QWidget *parent) :
    QGLWidget(format,parent)
	/*
GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(parent)
	*/
{
	bounding_box = true;
	grid_line = false;
	gmm_render = false;
	FOV = 60;
    zoom			= 50;
	gpu_zoom = zoom;
    swing_angle		= 0;
    elevate_angle	= 0;
    motion_mode		= NO_MOTION;
    mouse_x			= 0;
    mouse_y			= 0;
    xRot            = 0;
    yRot			= 0;
    setFocusPolicy(Qt::WheelFocus);
    R=0.0f;
    G=1.0f;
    B=1.0f;
	opacity_density = 1.0f;
	screen_width = WINDOW_SIZE;
	screen_height = WINDOW_SIZE;
	frameCount = 0;
	fpsCount = 0;
	fpsLimit = 1;
	//timer = 0;
	pbo = 0;     // OpenGL pixel buffer object
    tex = 0;     // OpenGL texture object
	ka = 0.1;
	kd = 0.8;
	ks = 0.7;
	shininess = 10.0f;
	LightPosition[0] =  50.0f;
	LightPosition[1] =  0.0f;
	LightPosition[2] =  -50.0f;
	LightPosition[3] =  0.0f;
	LightPosition2[0] =  -50.0f;
	LightPosition2[1] =  0.0f;
	LightPosition2[2] =  50.0f;
	LightPosition2[3] =  0.0f;
	LightPosition3[0] =  0.0f;
	LightPosition3[1] =  50.0f;
	LightPosition3[2] =  0.0f;
	LightPosition3[3] =  0.0f;
	LightPosition4[0] =  0.0f;
	LightPosition4[1] =  -50.0f;
	LightPosition4[2] =  0.0f;
	LightPosition4[3] =  0.0f;

	LightAmbient[0] = 1.0f;
	LightAmbient[1] = 1.0f;
	LightAmbient[2] = 1.0f;
	LightAmbient[3] = 0.0f;

	LightDiffuse[0] = 1.0f;
	LightDiffuse[1] = 0.0f;
	LightDiffuse[2] = 0.0f;
	LightDiffuse[3] = 0.0f;

	LightSpecular[0] = 1.0f;
	LightSpecular[1] = 1.0f;
	LightSpecular[2] = 0.0f;
	LightSpecular[3] = 0.0f;

	option = 0;
	color_option = 1;
	mousemove = false;
	QObject::startTimer(500); 
	lighting = 0;
	//CV = new computer_vision();
	//gmm_output = fopen("gmm__random_number.raw","wb");
	//gmm_count = 0;

}

GLWidget::~GLWidget()
{
	//fclose(gmm_output);
	//glDeleteFramebuffers( 1, &framebuffer );
	//glDeleteRenderbuffers(1, &renderbuffer );

	//sdkDeleteTimer(&timer);
#if 1
	if (pbo)
    {
        cudaGraphicsUnregisterResource(cuda_pbo_resource);
        glDeleteBuffersARB(1, &pbo);
        glDeleteTextures(1, &tex);
    }

//	if(volume_mapper==1) 
	{
		freeCudaVolumeBuffers();
		freeCudaTransferFuncBuffers();
		cudaFree(gmm_volum_gpu);
	}
#endif
#if 0
	if(volume_mapper==0)
		delete [] cube_obj;
#endif


	
}

inline float* cross_product(float a1, float a2, float a3, float b1, float b2, float b3)
{
	float *cross = new float[3];
	cross[0] = (a2*b3-a3*b2);
	cross[1] = (a3*b1-a1*b3);
	cross[2] = (a1*b2-a2*b1);

	float mag = (float) sqrt( (double)(cross[0] * cross[0] + cross[1] * cross[1] + cross[2] * cross[2])); 
	mag = 1.0/mag;
	cross[0] *= mag;
	cross[1] *= mag;
	cross[2] *= mag;

	return cross;
}
void GLWidget::draw_arrow(float x,float y, float z, float pout_x, float pout_y, float pout_z, float length, float width)
{
	glLineWidth(width); 
	float origin[3];
	origin[0] = x+length*pout_x;
	origin[1] = y+length*pout_y;
	origin[2] = z+length*pout_z;

	glBegin(GL_LINES);
	glVertex3f(x,y,z);
	glVertex3f(origin[0],origin[1],origin[2]);
	glEnd();

	glPushMatrix();
	glTranslatef(origin[0],origin[1],origin[2]);

	float up1=0,up2=1,up3=0;
	if(pout_x==0 && pout_y==1 && pout_z==0)
	{
		up1=0;up2=0;up3=-1;
	}
	float *y_dir=new float[3];
	float *z_dir=new float[3];

	z_dir = cross_product(pout_x,pout_y,pout_z,up1,up2,up3);
	y_dir = cross_product(z_dir[0],z_dir[1],z_dir[2],pout_x,pout_y,pout_z);

	float arrow_length = length*0.1;
	float shift[3];
	float quad_point[4][3];
	shift[0] = -pout_x*arrow_length;
	shift[1] = -pout_y*arrow_length;
	shift[2] = -pout_z*arrow_length;
	
	for(int i=0;i<3;i++)
		quad_point[0][i] = shift[i]+(-y_dir[i]+z_dir[i])*arrow_length*0.5;
	for(int i=0;i<3;i++)
		quad_point[1][i] = shift[i]+(-y_dir[i]-z_dir[i])*arrow_length*0.5;
	for(int i=0;i<3;i++)
		quad_point[2][i] = shift[i]+(y_dir[i]-z_dir[i])*arrow_length*0.5;
	for(int i=0;i<3;i++)
		quad_point[3][i] = shift[i]+(y_dir[i]+z_dir[i])*arrow_length*0.5;
	//float x_length = arrow_length/x_length;

	glBegin(GL_QUADS);
	glVertex3f(quad_point[0][0],quad_point[0][1],quad_point[0][2]);
	glVertex3f(quad_point[1][0],quad_point[1][1],quad_point[1][2]);
	glVertex3f(quad_point[2][0],quad_point[2][1],quad_point[2][2]);
	glVertex3f(quad_point[3][0],quad_point[3][1],quad_point[3][2]);
	glEnd();
	
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0,0,0);
	glVertex3f(quad_point[0][0],quad_point[0][1],quad_point[0][2]);
	glVertex3f(quad_point[1][0],quad_point[1][1],quad_point[1][2]);	
	glVertex3f(quad_point[2][0],quad_point[2][1],quad_point[2][2]);
	glVertex3f(quad_point[3][0],quad_point[3][1],quad_point[3][2]);
	glEnd();

	glPopMatrix();
	delete [] z_dir;
	delete [] y_dir;
}
void GLWidget::Reset_View()
{

	half_xdim = xdim*0.5;
	half_ydim = ydim*0.5;
	half_zdim = zdim*0.5;

	//zoom = maxdim*1.5;
	zoom = 5;
	FOV = 60;
	xRot = 0;
    yRot = 0;
	cam_pos[0] = half_xdim;
	cam_pos[1] = half_ydim;
	cam_pos[2] = 0;
	look_at_pos[0] = half_xdim;
	look_at_pos[1] = half_ydim;
	look_at_pos[2] = 0;

	//LightPosition[0] =  half_xdim+(zoom/2.0)*sqrtf(3.0);
	//LightPosition[1] =  0.0f;
	//LightPosition[2] =  zoom/2.0;
	
	/*
	gridScale_X = (float)xdim/(float)mindim;
	gridScale_Y = (float)ydim/(float)mindim;
	gridScale_Z = (float)zdim/(float)mindim;
	*/
	
	gridScale_X = (float)xdim/(float)maxdim;
	gridScale_Y = (float)ydim/(float)maxdim;
	gridScale_Z = (float)zdim/(float)maxdim;
	
	gpu_zoom = zoom;

}

void GLWidget::wheelEvent(QWheelEvent *event)
{
	float zoom_delta;
	if(event->delta()>0)
		zoom_delta = 0.1;
	else if(event->delta())
		zoom_delta = -0.1;

    zoom += zoom_delta;
	gpu_zoom += zoom_delta;
    if(zoom<0.3)
        zoom=0.3;
	if(gpu_zoom<0.3)
        gpu_zoom=0.3;
    resizeGL(screen_width, screen_height);
    updateGL();
}
void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
	mousemove = false;
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
	 mousemove = true;
     int dx = event->x() - lastPos.x();
     int dy = event->y() - lastPos.y();

     if (event->buttons() & Qt::LeftButton) {
         setYRotation(yRot + (360 * (float)dy / (float)screen_height));
         setXRotation(xRot + (360 * (float)dx / (float)screen_width));
     }

     lastPos = event->pos();
 }

void GLWidget::mousePressEvent(QMouseEvent *event)
 {
     lastPos = event->pos();
	
	 if(event->buttons() & Qt::RightButton )
     {
        int x, y;
        x= lastPos.x();
		y= lastPos.y();
		printf("%d\t%d\n",x,y);
        DumpPixel( x, y );
     }
 }

static void qNormalizeAngle(int &angle)
{
    while (angle < 0)
        angle += 360 * 16;
    while (angle > 360 * 16)
        angle -= 360 * 16;
}

void GLWidget::setXRotation(int angle)
{
    //qNormalizeAngle(angle);
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
   // qNormalizeAngle(angle);
    if (angle != yRot) {
/*		if(angle> 90)
            angle =  90;
        else if(angle<-90)
            angle = -90;
*/
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    qNormalizeAngle(angle);
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{

    switch(event->key())
    {

        case Qt::Key_Escape:
            exit(0);

        case Qt::Key_A:
            zoom-=0.1;
            if(zoom<0.3) zoom=0.3;
            resizeGL(screen_width, screen_height);
            break;

        case Qt::Key_Z:
            zoom+=0.1;
            resizeGL(screen_width, screen_height);
            break;

        case Qt::Key_P:
            if(!polygonfill)
            {
                glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
                polygonfill = true;
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
                polygonfill = false;
            }
            break;

        case Qt::Key_B:
            if(!blend)
            {
                blend=true;
                glEnable(GL_BLEND);
                glDisable(GL_DEPTH_TEST);

            }
            else
            {
                blend=false;
                glEnable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);

            }		
            break;

		case Qt::Key_Up:
			FOV++;
			resizeGL(screen_width, screen_height);
			break;

		case Qt::Key_Down:
			FOV--;
			resizeGL(screen_width, screen_height);
			break;

		case Qt::Key_L:
			option = (option+1)%4;
			break;

		case Qt::Key_C:
			if(lighting)
				color_option = !color_option;
			break;
			
    }
	//printf("FOV:%f\n",FOV);
    updateGL();


}

void GLWidget::timerEvent(QTimerEvent *timerEvent)
{
	if(!mousemove)// && gmm_count<1331)
	{
		//cout << gmm_count << endl;
#if 0
		if(gmm_render)
		{
			cudaExtent volumeSize;
			volumeSize = make_cudaExtent(g.data.xdim, g.data.ydim, g.data.zdim);
			gmm_compute_kernel(gridSize2,blockSize2,g.data.size, gmm_volum_gpu ,gmm_parameter_gpu, volumeSize,g.data.xdim,g.data.ydim,g.data.zdim,g.data.gmm2.mixtured_numer);
		}

		if(gmm_render)
		{
			CV->generate_random(g.data.size,uni_rdnumber);

			for(int i=0;i<g.data.size;i++)
			{
				float sum=g.data.gmm[i].parameter[2]; //weighted of first gaussian 
				bool flag=false;
				for(int j=0;j<g.data.gmm[i].mixtured_numer-1;j++)
				{
					if(uni_rdnumber[i]<sum)
					{
						gmm_index[i] = j;
						flag = true;
						break;
					}
					else
						sum += g.data.gmm[i].parameter[(j+2)*3-1];
				}
				if(!flag)
					gmm_index[i] = g.data.gmm[i].mixtured_numer-1;

				g.data.volume[i] = CV->generate_gaussian_random(g.data.gmm[i].parameter[gmm_index[i]*3],g.data.gmm[i].parameter[gmm_index[i]*3+1]);


			}

			//fwrite(g.data.volume,sizeof(float),g.data.size,gmm_output);
			//gmm_count++;

			g.data.minvalue = *min_element(g.data.volume,g.data.volume + g.data.size);
			g.data.maxvalue = *max_element(g.data.volume,g.data.volume + g.data.size);

			create_volumetexture();

		}
#endif
		updateGL();
	}
	//else
		//cout << "stop count" <<endl; 
		
}

void GLWidget::Change_Color()
{
#if 0
	glActiveTexture(GL_TEXTURE4);
	glGenTextures(1, &transfer_color);
	glBindTexture(GL_TEXTURE_1D, transfer_color);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA, num_color , 0, GL_RGBA, GL_FLOAT, TransferFunc_color);
#endif
}

void GLWidget::Change_Alpha()
{
#if 0
	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, &transfer_alpha);
	glBindTexture(GL_TEXTURE_1D, transfer_alpha);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	
	glTexImage1D(GL_TEXTURE_1D, 0,GL_RED, num_alpha , 0, GL_RED, GL_FLOAT, TransferFunc_alpha);
#endif
}
void GLWidget::create_volumetexture()
{
	int size;
	
#if 0
	if(volume_rendering)
	{
		xsize = g.data.xdim;
		ysize = g.data.ydim;
		zsize = g.data.zdim;
	}


	size = xsize*ysize*zsize;
	

	GLfloat *data = new GLfloat[size];
	GLfloat *normal = new GLfloat[size*3];
	//int step = Transfer_Func.size()/(2*hist_gridsize);
	//step = step>0 ? step:1;	

	if(volume_rendering)
	{
		minValue = g.data.minvalue;
		maxValue = g.data.maxvalue;
		ValueRange = maxValue - minValue;
		
		for(int k1=0;k1<g.data.zdim;k1++)
			for(int j=0;j<g.data.ydim;j++)
				for(int i=0;i<g.data.xdim;i++)
				{
					int k = g.data.zdim-1-k1;
					int index = i+j*g.data.xdim+k*g.data.xdim*g.data.ydim;

					float value = (g.data.volume[index]-minValue)/ValueRange;  //[0,1]
					data[index] = value;

				}
	}

#if 0
	else
	{
		if(distrib_mode)
		{
			minValue = *min_element(g.data.iso_likelihood_field,g.data.iso_likelihood_field+size);
			maxValue = *max_element(g.data.iso_likelihood_field,g.data.iso_likelihood_field+size);
			ValueRange =  maxValue - minValue;

			if(ValueRange<=0.0000001)
			{

				for(int k1=0;k1<zsize;k1++)
					for(int j=0;j<ysize;j++)
						for(int i=0;i<xsize;i++)
						{
							int k = zsize-1-k1;
							int index = i+j*xsize+k*xsize*ysize;

							data[index] = 0;
						}
			}
			else
			{
				//FILE *fp2 = fopen("datasee_header.txt","w");
				//FILE *fp = fopen("datasee.dat","wb");
				for(int k1=0;k1<zsize;k1++)
					for(int j=0;j<ysize;j++)
						for(int i=0;i<xsize;i++)
						{
							int k = zsize-1-k1;
							int index = i+j*xsize+k*xsize*ysize;

							data[index] = (g.data.iso_likelihood_field[index]-minValue)/ValueRange;

						}
				//fprintf(fp2,"%d\t",xsize);
				//fprintf(fp2,"%d\t",ysize);
				//fprintf(fp2,"%d\t",zsize);
				//fwrite(data,sizeof(float),zsize*ysize*xsize,fp);
				//fclose(fp);
				//fclose(fp2);
			}
			//hixel->Fuzzy_IsoSusrface(xsize,ysize,zsize,IsoValue,g.data.histogram,data,etype);
		}
			/*
			float minV = *min_element(data,data + size);
			float maxV = *max_element(data,data + size);

			value_range = maxV - minV;
	for(int k1=0;k1<g.data.zdim;k1++)
		for(int j=0;j<g.data.ydim;j++)
			for(int i=0;i<g.data.xdim;i++)
			{
				int k = g.data.zdim-1-k1;
				int index = i+j*g.data.xdim+k*g.data.xdim*g.data.ydim;

				float value = (data[index]-minV)/value_range;
				data[index] = value;
				
			}
			*/
	}
#endif 

		
	glActiveTexture(GL_TEXTURE0);
	glPixelStorei(GL_UNPACK_ALIGNMENT,4);
	glGenTextures(1, &volume_texture);
	glBindTexture(GL_TEXTURE_3D, volume_texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	//glTexImage3D(GL_TEXTURE_3D, 0,GL_RGBA, VOLUME_TEX_SIZE, VOLUME_TEX_SIZE,VOLUME_TEX_SIZE,0, GL_RGBA, GL_UNSIGNED_BYTE,data);
	glTexImage3D(GL_TEXTURE_3D, 0,GL_RED, xsize, ysize, zsize,0, GL_RED, GL_FLOAT,data);


#if normal_compute
	if(lighting)
	{
		for(int k1=0;k1<g.data.zdim;k1++)
			for(int j=0;j<g.data.ydim;j++)
				for(int i=0;i<g.data.xdim;i++)
				{
					int k = g.data.zdim-1-k1;
					int index = i+j*g.data.xdim+k*g.data.xdim*g.data.ydim;

					normal[index*3] = (g.gradient[index].x+1.0)/2.0;
					normal[index*3+1] = (g.gradient[index].y+1.0)/2.0;
					normal[index*3+2] = (g.gradient[index].z+1.0)/2.0;
				}


				glActiveTexture(GL_TEXTURE5);
				glPixelStorei(GL_UNPACK_ALIGNMENT,1);
				glGenTextures(1, &normal_texture);
				glBindTexture(GL_TEXTURE_3D, normal_texture);
				glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
				glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
				//glTexImage3D(GL_TEXTURE_3D, 0,GL_RGBA, VOLUME_TEX_SIZE, VOLUME_TEX_SIZE,VOLUME_TEX_SIZE,0, GL_RGBA, GL_UNSIGNED_BYTE,data);
				glTexImage3D(GL_TEXTURE_3D, 0,GL_RGB, xsize, ysize, zsize,0, GL_RGB, GL_FLOAT,normal);
	}
#endif

	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	delete [] data;
	delete [] normal;

#endif
	//cout << "volume texture created" << endl;

}
void read_Image() 
{
  FILE* in = fopen("buck.ppm", "r"); 

  int height, width, ccv; 
  char header[100]; 
  fscanf(in, "%s %d %d %d", header, &width, &height, &ccv); 

  printf("%s %d %d %d\n", header, width, height, ccv);
  int r, g, b; 

  for (int i=height-1; i>=0; i--)
     for (int j=0; j<width; j++)
{
      fscanf(in, "%d %d %d", &r, &g, &b); 
      readImage[i][j][0] = (GLubyte)r; 
      readImage[i][j][1] = (GLubyte)g; 
      readImage[i][j][2] = (GLubyte )b; 
      readImage[i][j][3] = 255; 
    }

  for (int i=0; i<256; i++)
    for ( int j=0; j<256; j++) {
      if (i<height && j <width) {
	texImage[i][j][0] = readImage[i][j][0]; 
	texImage[i][j][1] = readImage[i][j][1];
	texImage[i][j][2] = readImage[i][j][2];
	texImage[i][j][3] = 255; 
      }
      else {
      	texImage[i][j][0] = 0; 
	texImage[i][j][1] = 0; 
	texImage[i][j][2] = 0; 
	texImage[i][j][3] = 255; 
      }
    }
  
  fclose(in); 
}
void read_Image2() 
{
  FILE* in = fopen("qb.ppm", "r"); 

  int height, width, ccv; 
  char header[100]; 
  fscanf(in, "%s %d %d %d", header, &width, &height, &ccv); 

  printf("%s %d %d %d\n", header, height, width, ccv);
  int r, g, b; 
  
 for (int i=height-1; i>=0; i--)
	 for (int j=0; j<width; j++)
{
      fscanf(in, "%d %d %d", &r, &g, &b); 
      readImage[i][j][0] = (GLubyte)r; 
      readImage[i][j][1] = (GLubyte)g; 
      readImage[i][j][2] = (GLubyte )b; 
      readImage[i][j][3] = 255; 
    }

  for (int i=0; i<256; i++)
    for ( int j=0; j<512; j++) {
      if (i<height && j <width) {
	texImage2[i][j][0] = readImage[i][j][0]; 
	texImage2[i][j][1] = readImage[i][j][1];
	texImage2[i][j][2] = readImage[i][j][2];
	texImage2[i][j][3] = 255; 
      }
      else {
      	texImage2[i][j][0] = 0; 
	texImage2[i][j][1] = 0; 
	texImage2[i][j][2] = 0; 
	texImage2[i][j][3] = 255; 
      }
    }
  
  fclose(in); 
}


void vertex(float x, float y, float z)
{
	glColor3f(x,y,z);
	glVertex3f(x,y,z);
}

void drawQuads(float x, float y, float z)
{
	glBegin(GL_QUADS);
	/* Back side */
	glNormal3f(0.0, 0.0, -1.0);
	vertex(0.0, 0.0, 0.0);
	vertex(0.0, y, 0.0);
	vertex(x, y, 0.0);
	vertex(x, 0.0, 0.0);

	/* Front side */
	glNormal3f(0.0, 0.0, 1.0);
	vertex(0.0, 0.0, z);
	vertex(x, 0.0, z);
	vertex(x, y, z);
	vertex(0.0, y, z);

	/* Top side */
	glNormal3f(0.0, 1.0, 0.0);
	vertex(0.0, y, 0.0);
	vertex(0.0, y, z);
    vertex(x, y, z);
	vertex(x, y, 0.0);

	/* Bottom side */
	glNormal3f(0.0, -1.0, 0.0);
	vertex(0.0, 0.0, 0.0);
	vertex(x, 0.0, 0.0);
	vertex(x, 0.0, z);
	vertex(0.0, 0.0, z);

	/* Left side */
	glNormal3f(-1.0, 0.0, 0.0);
	vertex(0.0, 0.0, 0.0);
	vertex(0.0, 0.0, z);
	vertex(0.0, y, z);
	vertex(0.0, y, 0.0);

	/* Right side */
	glNormal3f(1.0, 0.0, 0.0);
	vertex(x, 0.0, 0.0);
	vertex(x, y, 0.0);
	vertex(x, y, z);
	vertex(x, 0.0, z);
	glEnd();
}
void GLWidget::render_backface()
{		
#if 0
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0,0,WINDOW_SIZE,WINDOW_SIZE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	drawQuads(1.0,1.0,1.0);
	glDisable(GL_CULL_FACE);
#endif
	
}
void GLWidget::render_frontface()
{	
#if 0
	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glViewport(0,0,WINDOW_SIZE,WINDOW_SIZE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	drawQuads(1.0,1.0,1.0);
	glDisable(GL_CULL_FACE);
#endif	
}

void GLWidget::raycasting_pass()
{
#if 0
	glUseProgram(programObject);	
	
	GLuint c0 = glGetAttribLocation(programObject, "position");  
	//GLuint c1 = glGetAttribLocation(programObject, "tex"); 
	//glUniform1f(c0, point); 
	
	glEnableVertexAttribArray(c0);
	//glEnableVertexAttribArray(c1);
	glBindBuffer(GL_ARRAY_BUFFER, cube_obj[0].vboHandle[0]); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_obj[0].indexVBO[0]);
	glVertexAttribPointer(c0,4,GL_FLOAT, GL_FALSE, sizeof(Vertex),(char*) NULL+0); 	
	//glVertexAttribPointer(c1,4,GL_FLOAT, GL_FALSE, sizeof(Vertex),(char*) NULL+16); 	
	

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, volume_texture);
	GLuint texloc1 = glGetUniformLocation(programObject, "volume_tex");
	glUniform1i(texloc1, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, backface_buffer);
	GLuint texloc = glGetUniformLocation(programObject, "back_Image");
	glUniform1i(texloc, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, final_image);
	GLuint texloc2 = glGetUniformLocation(programObject, "front_Image");
	glUniform1i(texloc2, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_1D, transfer_alpha);
	GLuint texloc3 = glGetUniformLocation(programObject, "transfer_alpha");
	glUniform1i(texloc3, 3);

	if(color_option)
	{
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_1D, transfer_color);
		GLuint texloc4 = glGetUniformLocation(programObject, "transfer_color");
		glUniform1i(texloc4, 4);
	}

#if normal_compute	
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_3D, normal_texture);
		GLuint texloc5 = glGetUniformLocation(programObject, "normal_vec");
		glUniform1i(texloc5, 5);
#endif
	
	GLuint step = glGetUniformLocation(programObject, "stepsize");

	if(mousemove && lighting)
		glUniform1f(step, stepsize*5); 
	else
		glUniform1f(step, stepsize); 

	GLuint light_pos = glGetUniformLocation(programObject, "light_pos");
	glUniform4f(light_pos, LightPosition[0], LightPosition[1], LightPosition[2], LightPosition[3]);

	GLuint light_pos2 = glGetUniformLocation(programObject, "light_pos2");
	glUniform4f(light_pos2, LightPosition2[0], LightPosition2[1], LightPosition2[2], LightPosition2[3]); 

	GLuint light_pos3 = glGetUniformLocation(programObject, "light_pos3");
	glUniform4f(light_pos3, LightPosition3[0], LightPosition3[1], LightPosition3[2], LightPosition3[3]); 

	GLuint light_pos4 = glGetUniformLocation(programObject, "light_pos4");
	glUniform4f(light_pos4, LightPosition4[0], LightPosition4[1], LightPosition4[2], LightPosition4[3]);
	
	GLuint light_amb = glGetUniformLocation(programObject, "la");
	glUniform4f(light_amb, LightAmbient[0], LightAmbient[1], LightAmbient[2], LightAmbient[3]);

	GLuint light_diff = glGetUniformLocation(programObject, "ld");
	glUniform4f(light_diff, LightDiffuse[0], LightDiffuse[1], LightDiffuse[2], LightDiffuse[3]);

	GLuint light_diff2 = glGetUniformLocation(programObject, "ld2");
	glUniform4f(light_diff2, LightDiffuse2[0], LightDiffuse2[1], LightDiffuse2[2], LightDiffuse2[3]);

	GLuint light_spec = glGetUniformLocation(programObject, "ls");
	glUniform4f(light_spec, LightSpecular[0], LightSpecular[1], LightSpecular[2], LightSpecular[3]);

	GLuint light_spec2 = glGetUniformLocation(programObject, "ls2");
	glUniform4f(light_spec2, LightSpecular2[0], LightSpecular2[1], LightSpecular2[2], LightSpecular2[3]);

	GLuint coeff_amb = glGetUniformLocation(programObject, "ka");
	glUniform1f(coeff_amb, ka);

	GLuint coeff_dif = glGetUniformLocation(programObject, "kd");
	glUniform1f(coeff_dif, kd);

	GLuint coeff_spec = glGetUniformLocation(programObject, "ks");
	glUniform1f(coeff_spec, ks);

	GLuint light_shin = glGetUniformLocation(programObject, "shininess");
	glUniform1f(light_shin, shininess);

	GLuint density_shader = glGetUniformLocation(programObject, "density");
	glUniform1f(density_shader, opacity_density); 

	GLuint light_bool = glGetUniformLocation(programObject, "lighting");
	glUniform1i(light_bool, lighting); 

	GLuint display_option = glGetUniformLocation(programObject, "option");
	glUniform1i(display_option, option);

	GLuint color_opt = glGetUniformLocation(programObject, "color_option");
	glUniform1i(color_opt, color_option);

	GLuint double_light = glGetUniformLocation(programObject, "double_lighting");
	glUniform1i(double_light, double_lighting);

	g_LocShaderDebugSqueezeFactor = glGetUniformLocation( programObject, "fDebugSqueezeFactor" );
	glUniform1f( g_LocShaderDebugSqueezeFactor, g_DebugSqueezeFactor );
	//GLuint c1 = glGetAttribLocation(programObject, "texCoord");
	glDrawElements(GL_QUADS, cube_obj[0].nindices, GL_UNSIGNED_INT, (char*) NULL+0);	

	glUseProgram(0);
#endif
}
void enable_renderbuffers()
{
//	glBindFramebuffer (GL_FRAMEBUFFER, framebuffer);
	//glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
} 
void disable_renderbuffers()
{
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLWidget::draw_picture()
{
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256,256,0, GL_RGBA, GL_UNSIGNED_BYTE,texImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
					  GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
					  GL_NEAREST); 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glColor4f(1,1,1,1); 
  //glPushMatrix(); 
  //glScalef(3, 0.01, 4); 
	  glBegin(GL_QUADS);       
	  glTexCoord2d(0,0);
	  glVertex3f(half_xdim,-half_ydim,-half_zdim);
	  glTexCoord2d(199/255.0,0);
	  glVertex3f(-half_xdim,-half_ydim,-half_zdim);
	  glTexCoord2d(199/255.0,221/255.0); 
	  glVertex3f(-half_xdim,half_ydim,-half_zdim);
	  glTexCoord2d(0, 221/255.0); 
	  glVertex3f(half_xdim,half_ydim,-half_zdim);
	  glEnd(); 
	  glDisable(GL_TEXTURE_2D);
  //glPopMatrix(); 
}
void GLWidget::draw_picture2()
{
	glEnable(GL_TEXTURE_2D);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512,256,0, GL_RGBA, GL_UNSIGNED_BYTE,texImage2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
					  GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
					  GL_NEAREST); 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glColor4f(1,1,1,1); 
  //glPushMatrix(); 
  //glScalef(3, 1, 4); 
	glBegin(GL_QUADS); 
  
	glTexCoord2f(0,0);
	glVertex3f(-half_xdim,-half_ydim,half_zdim);
	glTexCoord2d(299/511.0,0);
    glVertex3f(half_xdim,-half_ydim,half_zdim);
	glTexCoord2d(299/511.0,205/255.0);
    glVertex3f(half_xdim,half_ydim,half_zdim);
	glTexCoord2d(0,205/255.0); 
    glVertex3f(-half_xdim,half_ydim,half_zdim);
	glEnd(); 
	glDisable(GL_TEXTURE_2D);
  //glPopMatrix(); 
}
void GLWidget::draw_picture3()
{
#if 0
	glEnable(GL_TEXTURE_3D);
	float count_num=512;
	glBegin(GL_QUADS);
	for(int xx=0;xx<(int)count_num;xx++)
	{
		glTexCoord3f(0,0,1-xx*(1.0/(count_num-1)));
		glVertex3f(-half_xdim,-half_ydim,half_zdim-xx*(zdim/(count_num-1)));
		glTexCoord3f(1,0,1-xx*1.0/(count_num-1));
		glVertex3f(half_xdim,-half_ydim,half_zdim-xx*(zdim/(count_num-1)));
		glTexCoord3f(1,1,1-xx*1.0/(count_num-1));
		glVertex3f(half_xdim,half_ydim,half_zdim-xx*(zdim/(count_num-1)));
		glTexCoord3f(0,1,1-xx*1.0/(count_num-1)); 
		glVertex3f(-half_xdim,half_ydim,half_zdim-xx*(zdim/(count_num-1)));
	}
	glEnd();

	glDisable(GL_TEXTURE_3D);
#endif
}

void GLWidget::DumpPixel( int x, int y )
{
    int winWidth, winHeight;
   // glfwGetWindowSize( &winWidth, &winHeight );
	winWidth = WINDOW_SIZE;
	winHeight = WINDOW_SIZE;
    int capturePositionX = x;
    int capturePositionY = winHeight - y;

    GLfloat *data = new GLfloat[4];
    glReadPixels( capturePositionX, capturePositionY, 1, 1, GL_RGBA, GL_FLOAT, data );

    //GLfloat base = g_DebugSqueezeFactor / 0.5f;
	GLfloat base = g_DebugSqueezeFactor / 0.5f;
    GLfloat vx = data[0], vy = data[1], vz = data[2], vw = data[3];

    // X
    if( (vx - 0.5f)>0.00000000001 )
        vx = -(( vx  - 0.5f ) * base);
    else
        vx *= base;

    // Y
    if( (vy - 0.5f)>0.00000000001 )
        vy = -(( vy  - 0.5f ) * base);
    else
        vy *= base;

    // Z
    if( (vz - 0.5f)>0.00000000001 )
        vz = -(( vz  - 0.5f ) * base);
    else
        vz *= base;

    // W
    if( (vw-0.5f) > 0.00000000001 )
        vw = -(( vw  - 0.5f ) * base);
    else
        vw *= base;


    qDebug() << "DEBUG: Capturing at [" << capturePositionX << "," << capturePositionY  << "] with squeeze factor " << g_DebugSqueezeFactor << endl;
    //qDebug() << "   RAW        :  X = " << data[0] << " Y = " << data[1] << " Z = " << data[2] << " W = " << data[3] << endl;
    //qDebug() << "   INTERPRETED:  X = " << vx << " Y = " << vy << " Z = " << vz << " W = " << vw << endl;
	printf("   RAW        :  X = %f Y = %f Z = %f W = %f\n",data[0],data[1],data[2],data[3]);
	printf("   INTERPRETED:  X = %f Y = %f Z = %f W = %f\n",vx,vy,vz,vw);
	/*
	int verx,very,verz;
	float datax,datay,dataz;
	verx = vx*(xsize-1);
	very = vy*(ysize-1);
	verz = vz*(zsize-1);
	for(int i=0;i<=1;i++)
		for(int j=0;j<=1;j++)
			for(int k=0;k<=1;k++)
			{
				int index = verx+k + (very+j)*g.data.xdim + (verz+i)*g.data.xdim*g.data.ydim;
				//printf("[%d %d %d] = %f\n",verx+k,very+j,verz+i,g.data.volume[verx+k + (very+j)*g.data.xdim + (verz+i)*g.data.xdim*g.data.ydim]/ValueRange);
				//printf("[%d %d %d] = %f %f %f\n",verx+k,very+j,verz+i,g.gradient[index].x,g.gradient[index].y,g.gradient[index].z);
			}
			*/
     delete[] data;
}

void GLWidget::render()
{
    copyInvViewMatrix(invViewMatrix, sizeof(float4)*3);

    uint *d_output;
    // map PBO to get CUDA device pointer
    checkCudaErrors(cudaGraphicsMapResources(1, &cuda_pbo_resource, 0));
    size_t num_bytes;
    checkCudaErrors(cudaGraphicsResourceGetMappedPointer((void **)&d_output, &num_bytes,
                                                         cuda_pbo_resource));
    //printf("CUDA mapped PBO: May access %ld bytes\n", num_bytes);

    // clear image
    checkCudaErrors(cudaMemset(d_output, 0, WINDOW_SIZE*WINDOW_SIZE*4));

	cudaExtent volumeSize;

	volumeSize = make_cudaExtent(g.data.xdim, g.data.ydim, g.data.zdim);
	minValue = g.data.minvalue;
	maxValue = g.data.maxvalue;

	TransferFunc(TransferFunc_color, num_color, TransferFunc_alpha, num_alpha);
    // call CUDA kernel, writing results to PBO
    render_kernel(gridSize, blockSize, d_output, WINDOW_SIZE, WINDOW_SIZE, 
		          opacity_density, brightness, transferOffset, transferScale, volumeSize, minValue, maxValue, gridScale_X,gridScale_Y,gridScale_Z,stepsize);

    getLastCudaError("kernel failed");

    checkCudaErrors(cudaGraphicsUnmapResources(1, &cuda_pbo_resource, 0));
}

void GLWidget::paintGL() {    

	//printf("%d\n",fpsLimit);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();
    //gluLookAt(0, 0, zoom, 0, 0, 0, 0, 1, 0);
	//
	glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable ( GL_COLOR_MATERIAL );

	//GLfloat modelView[16];
	//sdkStartTimer(&timer);
	//printf("zoom:%f\n",zoom);
	//if(volume_mapper == 1)
	{

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		glRotatef(-yRot, 1, 0, 0);
		glRotatef(-xRot, 0, 1, 0);
		glTranslatef(0, 0, gpu_zoom);
		//printf("xRot:%d\tyRot:%d\n",xRot,yRot);
		glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
		glPopMatrix();

		invViewMatrix[0] = modelView[0];
		invViewMatrix[1] = modelView[4];
		invViewMatrix[2] = modelView[8];
		invViewMatrix[3] = modelView[12];
		invViewMatrix[4] = modelView[1];
		invViewMatrix[5] = modelView[5];
		invViewMatrix[6] = modelView[9];
		invViewMatrix[7] = modelView[13];
		invViewMatrix[8] = modelView[2];
		invViewMatrix[9] = modelView[6];
		invViewMatrix[10] = modelView[10];
		invViewMatrix[11] = modelView[14];

		render();

		// display results
		glClear(GL_COLOR_BUFFER_BIT);

		// draw image from PBO
		glDisable(GL_DEPTH_TEST);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		// draw using texture

		// copy from pbo to texture
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, WINDOW_SIZE, WINDOW_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

		// draw textured quad
		//glTranslatef(0.5-(float)xdim/((float)maxdim*2.0),0.5-(float)ydim/((float)maxdim*2.0),0);
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glTexCoord2f(1, 0);
		glVertex2f(1, 0);
		glTexCoord2f(1, 1);
		glVertex2f(1, 1);
		glTexCoord2f(0, 1);
		glVertex2f(0, 1);
		glEnd();

		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

#if 0
	else
	{
	//1.gluLookAt(cam_pos[0], cam_pos[1], cam_pos[2]+zoom, look_at_pos[0], look_at_pos[1], look_at_pos[2], 0, 1, 0);
	
	gluLookAt(0, 0, zoom, 0, 0, 0, 0, 1, 0);
	//printf("%f\n",zoom);
	//printf("%f\t%f\t%f\n%f\t%f\t%f\n",cam_pos[0],cam_pos[1],cam_pos[2]+zoom,look_at_pos[0],look_at_pos[1],look_at_pos[2]);


	//glMatrixMode(GL_MODELVIEW);   
	//2.glTranslatef(half_xdim,half_ydim,0); //origin is in (half_xdim,half_ydim,0)
	glTranslatef(0,0,0);
	//glTranslatef(1,1,0);
    glRotatef(yRot, 1, 0, 0);
    glRotatef(xRot, 0, 1, 0);
	//glRotatef(180, 0, 1, 0);
	//2D texture drawing
	//draw_picture2();
	//draw_picture();
	////// end of 2D texture drawing /////////////////

	//3D texture drawing
	//draw_picture3();
	////// end of 3D texture drawing /////////////////

	////// draw_box /////////////
	/*
	if(bounding_box)
	{
		glLineWidth(3); 
		glColor4f(1,1.0,1.0,1.0f);

		glBegin(GL_LINE_LOOP);
		glVertex3f(-half_xdim,-half_ydim,half_zdim);
		glVertex3f(half_xdim,-half_ydim,half_zdim);
		glVertex3f(half_xdim,half_ydim,half_zdim);
		glVertex3f(-half_xdim,half_ydim,half_zdim);
		glEnd();
	
		glBegin(GL_LINE_LOOP);
		glVertex3f(-half_xdim,-half_ydim,-half_zdim);
		glVertex3f(half_xdim,-half_ydim,-half_zdim);
		glVertex3f(half_xdim,half_ydim,-half_zdim);
		glVertex3f(-half_xdim,half_ydim,-half_zdim);
		glEnd();

		glBegin(GL_LINES);
		glVertex3f(-half_xdim,-half_ydim,half_zdim);
		glVertex3f(-half_xdim,-half_ydim,-half_zdim);
	
		glVertex3f(half_xdim,-half_ydim,half_zdim);
		glVertex3f(half_xdim,-half_ydim,-half_zdim);

		glVertex3f(half_xdim,half_ydim,half_zdim);
		glVertex3f(half_xdim,half_ydim,-half_zdim);

		glVertex3f(-half_xdim,half_ydim,half_zdim);
		glVertex3f(-half_xdim,half_ydim,-half_zdim);
		glEnd();
	}
	*/
	////// draw_box end/////////////
	
	////////draw XYZ arrows //////////////
	//glPushMatrix();  //origin is in (0,0,0)
	
	//3.glTranslatef(-half_xdim,-half_ydim,half_zdim);
	
	/*
    glColor4f(1,0,0,0.3f);
	float arrow_loc_scale = xdim/10.0f;
	draw_arrow(-arrow_loc_scale,-arrow_loc_scale,-arrow_loc_scale,1,0,0,arrow_loc_scale,1);

    glColor4f(0,1,0,0.3f);
	draw_arrow(-arrow_loc_scale,-arrow_loc_scale,-arrow_loc_scale,0,1,0,arrow_loc_scale,1);
   
    glColor4f(0,0,1,0.3f);
	draw_arrow(-arrow_loc_scale,-arrow_loc_scale,-arrow_loc_scale,0,0,1,arrow_loc_scale,1);
	*/
	glLineWidth(1); 
	float arrow_len = 0.8;
	
//	glEnable(GL_MULTISAMPLE);
	//////////////////////////// Grid //////////////////////////////////////////
	// draw grid_line ////////////////////////

	if(grid_line)
	{
		glColor4f(1.0,1.0,1.0,1.0f);
		glLineWidth(0.1); 
		for(int i=0;i<=xdim;i=i+8)
		{
			for(int j=0;j<=ydim;j=j+8)
			{			
				glBegin(GL_LINES);
				glVertex3f(i,j,0);
				glVertex3f(i,j,-zdim);
				glEnd();
			}
		}
		
		for(int i=0;i<=xdim;i=i+8)
		{
			for(int j=0;j<=zdim;j=j+8)
			{
				glBegin(GL_LINES);
				glVertex3f(i,0,-j);
				glVertex3f(i,ydim,-j);
				glEnd();
			}
		}

		for(int i=0;i<=ydim;i=i+8)
		{
			for(int j=0;j<=zdim;j=j+8)
			{
				glBegin(GL_LINES);
				glVertex3f(0,i,-j);
				glVertex3f(xdim,i,-j);
				glEnd();
			}
		}	
	}
	//glPopMatrix();


////////////////////////////////////////////////////////////////////////////

	
	//4.glTranslatef(0,0,-zdim);	
	
	//glPushMatrix();	
	//glTranslatef(-0.5,-0.5,-0.5);
	glTranslatef(-gridScale_X*0.5,-gridScale_Y*0.5,-gridScale_Z*0.5);
	glScalef(gridScale_X,gridScale_Y,gridScale_Z);
/*	
		glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
		glGetFloatv(GL_PROJECTION_MATRIX,projmat);
	printf("////////////////////////////////////////  \n");
	for(int i=0;i<4;i++)
	{
		printf("\t");
		for(int j=0;j<4;j++)
			printf("%f\t",modelView[i+j*4]);
		printf("\n");
	}	
	printf("\n");
	for(int i=0;i<4;i++)
	{
		printf("\t");
		for(int j=0;j<4;j++)
			printf("%f\t",projmat[i+j*4]);
		printf("\n");
	}
	printf("\n\n");

	*/
	/*
	glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
	printf("\n\n");
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			printf("%f\t",modelView[i*4+j]);
		}
		printf("\n");
	}

	glGetFloatv(GL_PROJECTION_MATRIX, Projection);
	printf("\n\n");
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++)
		{
			printf("%f\t",Projection[i*4+j]);
		}
		printf("\n");
	}

	printf("\n\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\n\n");
	*/
	glDisable(GL_LIGHTING);
	enable_renderbuffers();	
	render_backface();
	render_frontface();
	disable_renderbuffers();		
	//glPopMatrix();
	glEnable(GL_LIGHTING);
	
	raycasting_pass();
	
	
	
	}
#endif
	QGLWidget:swapBuffers();
	
	//sdkStopTimer(&timer);
    //computeFPS();
//////////////////////////////////////////////////////
    //QMetaObject::invokeMethod(this,"updateGL",Qt::QueuedConnection);
}
void GLWidget::init()
{
	glEnable(GL_CULL_FACE);
	glClearColor(0.0, 0.0, 0.0, 0);	
#if 0	
	programObject = SetupGLSL("raycasting_shader");
	create_volumetexture();
	//// use for FrameBuffer
	glGenFramebuffers( 1, &framebuffer );
	glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);	
	
	glActiveTexture(GL_TEXTURE1);
	glGenTextures(1, &backface_buffer);
	glBindTexture(GL_TEXTURE_2D, backface_buffer);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backface_buffer, 0);

	glActiveTexture(GL_TEXTURE2);
	glGenTextures(1, &final_image);
	glBindTexture(GL_TEXTURE_2D, final_image);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, final_image, 0);

	if ( glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_UNSUPPORTED)
                {
                    // todo: handle errors as exceptions and cleanup everything gracefully
                    qDebug() << "framebuffer.begin: your video card doesn't seem to support framebuffers" << endl;
                    exit(0);
                }

                qDebug() << "framebuffer.begin: failed to initialize framebuffer | error " << glCheckFramebufferStatus(GL_FRAMEBUFFER) << endl;
                exit(0);
            }
	else
		qDebug() << glCheckFramebufferStatus(GL_FRAMEBUFFER) <<endl;


	/*

	/////use for renderbuffer
	glGenRenderbuffers(1, &renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_SIZE, WINDOW_SIZE);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
	*/
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	

	/*
	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, &transferF);
	glBindTexture(GL_TEXTURE_1D, transferF);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA, 9 , 0, GL_RGBA, GL_FLOAT, transferFunc);
	*/
	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, &transfer_alpha);
	glBindTexture(GL_TEXTURE_1D, transfer_alpha);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage1D(GL_TEXTURE_1D, 0,GL_RED, num_alpha , 0, GL_RED, GL_FLOAT, TransferFunc_alpha);

	glActiveTexture(GL_TEXTURE4);
	glGenTextures(1, &transfer_color);
	glBindTexture(GL_TEXTURE_1D, transfer_color);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA, num_color , 0, GL_RGBA, GL_FLOAT, TransferFunc_color);
	/*
	read_Image();  
	read_Image2();
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
					  GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
					  GL_NEAREST); 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); 
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE, GL_REPLACE);
	*/
#endif
}
void GLWidget::resizeGL(int width, int height) {

	if(height == 0) height=1;

    screen_width=width;
	screen_height=height;	
	
	//if(volume_mapper == 1)
	{
		initPixelBuffer();
		// calculate new grid size
		gridSize = dim3(iDivUp(screen_width, blockSize.x), iDivUp(screen_height, blockSize.y));

		
	}

    glViewport(0, 0, screen_width, screen_height);
	//printf("width:%d\theight:%d\n",width,height);
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
	//if(volume_mapper==1)
	glOrtho(0.0, 1, 0.0, 1.0, 0.0, 1.0);
	//else
		//gluPerspective(FOV,((GLfloat)width)/((GLfloat)height),0.2f,10000.0f);
	
	
       
}
#if 0
void InitVBO(GLuint *vboHandle,GLuint *indexVBO, Vertex *verts, int nvertices, GLuint *indices, int nindices)
{
	
  //int nvertices = nslices * nstacks; 
  //nindices = (nstacks-1)*2*(nslices+1); 

  glGenBuffers(1, vboHandle);   // create an interleaved VBO object    
  glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]);   // bind the first handle 
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*nvertices, verts, GL_STATIC_DRAW); // allocate space and copy the position data over
  glBindBuffer(GL_ARRAY_BUFFER, 0);   // clean up 

  glGenBuffers(1, indexVBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVBO[0]); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*nindices, indices, GL_STATIC_DRAW);  // load the index data 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);  // clean up 
} 

void GLWidget::Initial_Cube(Cube *cube_obj, int cube_idx)
{

	cube_obj[cube_idx].scale.x = xdim;
	cube_obj[cube_idx].scale.y = ydim;
	cube_obj[cube_idx].scale.z = zdim;
	cube_obj[cube_idx].nvertices = 8;
	cube_obj[cube_idx].ver = new Vertex[cube_obj[cube_idx].nvertices];
	cube_obj[cube_idx].nindices = 24;
	cube_obj[cube_idx].indices = new GLuint[cube_obj[cube_idx].nindices]; 

	Cube_Vertex(cube_obj,cube_idx);
	InitVBO(cube_obj[cube_idx].vboHandle,cube_obj[cube_idx].indexVBO,
	      cube_obj[cube_idx].ver,cube_obj[cube_idx].nvertices,
		  cube_obj[cube_idx].indices, cube_obj[cube_idx].nindices
		  ); 		  
}
#endif
void GLWidget::initPixelBuffer()
{
    if (pbo)
    {
        // unregister this buffer object from CUDA C
        checkCudaErrors(cudaGraphicsUnregisterResource(cuda_pbo_resource));

        // delete old buffer
        glDeleteBuffersARB(1, &pbo);
        glDeleteTextures(1, &tex);
    }

    // create pixel buffer object for display
    glGenBuffersARB(1, &pbo);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
    glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, screen_width*screen_height*sizeof(GLubyte)*4, 0, GL_STREAM_DRAW_ARB);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    // register this buffer object with CUDA
    checkCudaErrors(cudaGraphicsGLRegisterBuffer(&cuda_pbo_resource, pbo, cudaGraphicsMapFlagsWriteDiscard));

    // create texture for display
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
	
}

void GLWidget::computeFPS()
{
#if 0
    frameCount++;
    fpsCount++;

    if (fpsCount == fpsLimit)
    {
        char fps[256];
        float ifps = 1.f / (sdkGetAverageTimerValue(&timer) / 1000.f);
        sprintf(fps, "Volume Render: %3.1f fps", ifps);
		//printf(fps, "Volume Render: %3.1f fps", ifps);

        setWindowTitle(fps);
        fpsCount = 0;

        fpsLimit = (int)MAX(1.f, ifps);
        sdkResetTimer(&timer);
    }
#endif
}
void GLWidget::closeEvent ( QCloseEvent * event ) 
{
	emit glw_window_closed();
}
void GLWidget::initializeGL() {
	
	//cudaGLSetGLDevice(cutGetMaxGflopsDeviceId() );
	//glutInit(argc, argv);
#if use_glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		printf("Error: %s\n", glewGetErrorString(err));
	}
#endif
   // int mode=GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH;
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);     // Background => dark blue
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
	
	glMatrixMode(GL_PROJECTION); 
	glLoadIdentity(); 
	glEnable(GL_LIGHT0);
	if(lighting)
	{
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
		//glEnable(GL_LIGHT0);
		/*
		glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
		glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);		// Setup The Ambient Light
		glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);		// Setup The Diffuse Light
		glLightfv(GL_LIGHT0, GL_SPECULAR,LightSpecular);	// Setup The Specular Light
		//GLfloat LightPosition2[]=   { 0.0f, 0.0f, -1.0f ,0.0f};
		*/
	}

	//sdkCreateTimer(&timer);
	

   	initPixelBuffer();

 
} 