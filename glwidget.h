#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <GL/glew.h>
#include <QMessageBox>
#include <QGLWidget>
#include <vector>
//#include "primitive.h"
//#include "hixel.h"
#include "utility.h"
//#include "computer_vision.h"

// CUDA Runtime, Interop, and includes
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <vector_types.h>
#include <vector_functions.h>
#include <driver_functions.h>

// CUDA utilities
#include <helper_cuda.h>
#include <helper_cuda_gl.h>

// Helper functions
//#include <helper_cuda.h>
//#include <helper_functions.h>
//#include <helper_timer.h>

using namespace std;

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(const QGLFormat& format, QWidget* parent = 0);
	//explicit GLWidget(QWidget* parent = 0);
    ~GLWidget();
     void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);
	 void mouseReleaseEvent(QMouseEvent *event);
     void keyPressEvent(QKeyEvent *event);
     void wheelEvent(QWheelEvent *event);
	 void draw_arrow(float x,float y, float z, float pout_x, float pout_y, float pout_z, float length, float width);
	 void Reset_View();
	 void Change_Alpha();
	 void Change_Color();
	 void initPixelBuffer();
	 void create_volumetexture();

     int  screen_width, screen_height;
	 int  xdim,ydim,zdim,maxdim,mindim;	 
	 int  motion_mode, mouse_x, mouse_y;
	 int  num_alpha;
	 int  num_color;
	 int  volume_mapper;
	 int  lighting;
	 int  double_lighting;

	 float cam_pos[3],look_at_pos[3];
     float R,G,B;
	 float opacity_density;
	 float stepsize;
	 float gridScale_X; 
	 float gridScale_Y; 
	 float gridScale_Z;
	 float IsoValue;
	 float minValue,maxValue,ValueRange;
	 float ka;
	 float kd;
	 float ks;
	 float shininess;
	 
	 float *uni_rdnumber;
	 int *gmm_index;
	 float* gmm_volum_gpu;
	 float* gmm_parameter_gpu;

	 GLfloat LightPosition[4];
	 GLfloat LightPosition2[4];
	 GLfloat LightPosition3[4];
	 GLfloat LightPosition4[4];
	 GLfloat LightAmbient[4];
	 GLfloat LightDiffuse[4];
	 GLfloat LightSpecular[4];

	 bool blend;
	 bool grid_line;
	 bool bounding_box;
	 bool volume_rendering;
	 bool distrib_mode;
	 bool gmm_render;
	 
	 
	 GLfloat *TransferFunc_alpha;
	 GLfloat *TransferFunc_color;

public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);
	void glw_window_closed();

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();
	void init();
	void draw_picture2();
	void draw_picture();
	void draw_picture3();
	void timerEvent(QTimerEvent *event);
	//void Initial_Cube(Cube *cube_obj, int cube_idx);
	void DumpPixel( int x, int y );
	void raycasting_pass();
	void render();
	void computeFPS();
	void closeEvent( QCloseEvent * event );
	void render_backface();
	void render_frontface();
	

private:
     int xRot;
     int yRot;
     int zRot;
	 int fpsCount;
	 int fpsLimit;
	 int xsize,ysize,zsize;
	 int option;
	 int color_option;

	 float FOV;
	 float half_xdim,half_ydim,half_zdim;
	 float zoom, swing_angle, elevate_angle, gpu_zoom;   

	 //FILE* gmm_output;
	 //int gmm_count;

     QPoint lastPos;
     QMessageBox msgBox;
     bool polygonfill;
	 bool mousemove;
	 unsigned int frameCount;
	 
	// Cube *cube_obj;
	 GLuint pbo;     // OpenGL pixel buffer object
	 GLuint tex;     // OpenGL texture object

	 //computer_vision *CV;
	 //StopWatchInterface *timer;
};

#endif // GLWIDGET_H
