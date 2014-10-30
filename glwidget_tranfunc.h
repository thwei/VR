#ifndef GLWIDGET_TRANFUNC_H
#define GLWIDGET_TRANFUNC_H

#include "glwidget.h"
#include <QGLWidget>
#include <vector>
#include "curve.h"


using namespace std;

class GLWidget_TranFunc : public QGLWidget
{
 Q_OBJECT
public:
	explicit GLWidget_TranFunc(bool &_hist_exist, QWidget* parent = 0);
    ~GLWidget_TranFunc();
	void Init_TransFunc();

	
	int hist_gridsize;
	int moment_width;
	int moment_height;
	int scalarV_type;
	int color_type;

	int **moment_matrx;
	float *histc;

	float alphavalue;
	float hist_scale;
	float hist_brightness;
	float pointx;

	bool curve_transfeFunc;

	GLWidget *glw;

public slots:
	void Recal_CtrP();
	void Add_CtrP();
	void Delete_CtrP();
	void Change_Color();

signals:
	void ctrlP_changed();
	void show_value_changed();

protected:
	void mouseReleaseEvent( QMouseEvent * event );
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void paintGL();
	void resizeGL(int width, int height);
	void initializeGL();

private:
	void Interp_Color(float head_r, float head_g, float head_b,float tail_r, float tail_g, float tail_b,float dist);
	void Interp_Alpha(float head_alpha, float tail_alpha,float dist);
	void TnasferF_Color_Comp();
	void TnasferF_Alpha_Comp();
	void show_moment();
	void show_histogram();
	void show_transferfunc_curv();
	void show_ctrlP();
	void draw_ctlPLine();
	void draw_background_color();
	void show_value(int px, int py);
	void Preset_Color_Assign();

    float FOV;
	float pt_interval_boundary;
	float interp_stepsize; 
	int it_select;
	int edit_select;
	int screen_width,screen_height;

	QPoint lastPos;

	vector<float> curv_ver;
	vector<float> ctrl_ver;
	vector<float> temp_ctrp;
	vector<float> CtrlP_color;
	vector<float> temp_color;
	vector<float> temp_alpha;

	float *TransferFunc_alpha;
	float *TransferFunc_color;
	
	curve *crv;

	bool *hist_exist;
	
};
#endif // GLWIDGET_TRANFUNC_H