#include "glwidget_tranfunc.h"
#include <math.h>
#include <QtGui>
#include <QColorDialog>


GLWidget_TranFunc::GLWidget_TranFunc(bool &_hist_exist, QWidget *parent) :
    hist_exist(&_hist_exist), QGLWidget(parent)
{

	FOV = 60;
	hist_scale = 1;
	it_select=-1;
	pt_interval_boundary = 5;
	hist_brightness = 0.2;
	// fix the stepsize between each point in order to 
	// get a correct and uniform 1D vector to 
	// make correct texture mapping
	// Interp_Color Function and Interp_Alpha function
	interp_stepsize = 0.01f; 
	scalarV_type = 0;
	color_type = 0;

	curve_transfeFunc = false;

	curv_ver.clear();
	ctrl_ver.clear();
	temp_ctrp.clear();
	CtrlP_color.clear();
	temp_color.clear();
	temp_alpha.clear();

	TransferFunc_alpha = NULL;
	TransferFunc_color = NULL;
	crv = NULL;
	//this->setMouseTracking(true);// track mouse move;
	QObject::connect(this, SIGNAL(ctrlP_changed()), this, SLOT(Recal_CtrP())); 
}

GLWidget_TranFunc::~GLWidget_TranFunc()
{
	if(crv!=NULL)
		delete crv;
#ifdef WIN32
	curv_ver.swap(vector<float> ());
	ctrl_ver.swap(vector<float> ());
	temp_ctrp.swap(vector<float> ());
	CtrlP_color.swap(vector<float> ());
	temp_color.swap(vector<float> ());
	temp_alpha.swap(vector<float> ());
#else
    curv_ver = vector<float>();
    ctrl_ver = vector<float> ();
    temp_ctrp = vector<float> ();
    CtrlP_color = vector<float> ();
    temp_color = vector<float> ();
    temp_alpha = vector<float> ();
#endif
	/*
	if(TransferFunc_alpha!=NULL)
		delete [] TransferFunc_alpha;
	if(TransferFunc_color!=NULL)
		delete [] TransferFunc_color;
		*/
}

void GLWidget_TranFunc::Interp_Color(float head_r, float head_g, float head_b,float tail_r, float tail_g, float tail_b,float dist)
{
	float r,g,b;
	int stepnum = dist/interp_stepsize;
	float diff_r = (head_r-tail_r)/stepnum;
	float diff_g = (head_g-tail_g)/stepnum;
	float diff_b = (head_b-tail_b)/stepnum;

	temp_color.push_back(tail_r);
	temp_color.push_back(tail_g);
	temp_color.push_back(tail_b);
	temp_color.push_back(0);

	for(int i=1;i<stepnum;i++)
	{
		r = tail_r + i*diff_r;
		g = tail_g + i*diff_g;
		b = tail_b + i*diff_b;

		temp_color.push_back(r);
		temp_color.push_back(g);
		temp_color.push_back(b);
		temp_color.push_back(0);
	}
}

void GLWidget_TranFunc::Interp_Alpha(float head_alpha, float tail_alpha,float dist)
{
	float alpha;
	int stepnum = dist/interp_stepsize;
	float diff_alpha = (head_alpha-tail_alpha)/stepnum;

	temp_alpha.push_back(tail_alpha);

	for(int i=1;i<stepnum;i++)
	{
		alpha = tail_alpha + i*diff_alpha;

		temp_alpha.push_back(alpha);
	}
}

void GLWidget_TranFunc::TnasferF_Color_Comp()
{
	//vector<float> new_color(temp_color); //buffer

	//if(temp_color.size()>0)
	//	TransferFunc_color = &new_color[0];

	temp_color.clear();
	for(int i=0;i<(ctrl_ver.size()/2)-1;i++)
	{
		/*Interp_Color(CtrlP_color[(i+1)*3],CtrlP_color[(i+1)*3+1],CtrlP_color[(i+1)*3+2],
			CtrlP_color[i*3],CtrlP_color[i*3+1],CtrlP_color[i*3+2],
			ctrl_ver[(i+1)*2]-ctrl_ver[i*2]);*/
		Interp_Color(CtrlP_color[(i+1)*4],CtrlP_color[(i+1)*4+1],CtrlP_color[(i+1)*4+2],
			CtrlP_color[i*4],CtrlP_color[i*4+1],CtrlP_color[i*4+2],
			ctrl_ver[(i+1)*2]-ctrl_ver[i*2]);
	}
	TransferFunc_color = &temp_color[0];
	//new_color.swap(vector<float> ());
}
void GLWidget_TranFunc::TnasferF_Alpha_Comp()
{	
	//vector<float> new_alpha(temp_alpha); //buffer
	//if(temp_alpha.size()>0)
	//	TransferFunc_alpha = &new_alpha[0];

	float tempv;
	temp_alpha.clear();	
	if(curve_transfeFunc)
	{
		crv->ctrP = ctrl_ver;
		temp_ctrp = crv->curve_interp((ctrl_ver.size()/2)-2); // get interpolation-used control point
		crv->ctrP = temp_ctrp;
		curv_ver = crv->UniCubBSP(false);
		
		tempv = curv_ver[1]>=0 ? curv_ver[1]:0;
		tempv = tempv<=1 ? tempv:1;
		temp_alpha.push_back(tempv);
		int k=1;
		for(int i=2;i<curv_ver.size()-1;i=i+2)
		{
			if(curv_ver[i]>=(k*interp_stepsize))
			{
				tempv = curv_ver[i+1]>=0 ? curv_ver[i+1]:0;
				tempv = tempv<=1 ? tempv:1;
				temp_alpha.push_back(tempv);
				k++;
			}
		}
		
	}
	else
	{
		for(int i=0;i<(ctrl_ver.size()/2)-1;i++)
			Interp_Alpha(ctrl_ver[(i+1)*2+1],ctrl_ver[i*2+1],ctrl_ver[(i+1)*2]-ctrl_ver[i*2]);
	}

/*
	////////////////TESTTTTTTTTTT///////////////
	temp_alpha.clear();
	for(int i=0;i<(ctrl_ver.size()/2)-1;i++)
		temp_alpha.push_back(0);

	int head = temp_alpha.size()*0.3;
	int tail = temp_alpha.size()*0.4;

	for(int i=head;i<=tail;i++)
		temp_alpha[i] = 1;

	/////////////////TESTTTTTTTTTTTT////////////
	*/
	TransferFunc_alpha = &temp_alpha[0];
	//new_alpha.swap(vector<float> ());
}
void GLWidget_TranFunc::Preset_Color_Assign()
{
	switch(color_type)
	{
	case 0:
		{
			CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.5); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(1.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			//CtrlP_color.push_back(0.5); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			//CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.5); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.5); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0);
		}
		break;
	case 1:
		{
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.5); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0);
			//CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.5); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			//CtrlP_color.push_back(0.5); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(1.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.5); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
		}
		break;
	case 2:
		{
			CtrlP_color.push_back(1.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0);
			//CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.5); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0);			
			CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(1.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.0);
			//CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(0.5); CtrlP_color.push_back(0.0);
			CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0); CtrlP_color.push_back(1.0); CtrlP_color.push_back(0.0);

		}
		break;
	default:
		break;
	}
}

void GLWidget_TranFunc::Init_TransFunc()
{	
	curv_ver.clear();
	ctrl_ver.clear();
	temp_ctrp.clear();
	CtrlP_color.clear();
	temp_color.clear();
	temp_alpha.clear();

	int initial_num_ctrp = 5;
	 
	float x_step = 1.0f/(float)(initial_num_ctrp+1);
	float y_step = 1.0f/(float)(initial_num_ctrp+1);

	for(int i=0;i<initial_num_ctrp+2;i++)
	{
		float x = i*x_step;
		float y = pow(i*y_step,3);

		ctrl_ver.push_back(x);
		ctrl_ver.push_back(y);
	}

	if(crv!=NULL)
		delete crv;

	crv = new curve(ctrl_ver);	
	
	Preset_Color_Assign();

	TnasferF_Alpha_Comp();	
	
	TnasferF_Color_Comp();

	glw->TransferFunc_alpha = TransferFunc_alpha;
	glw->TransferFunc_color = TransferFunc_color;
	glw->num_alpha = temp_alpha.size();
	glw->num_color = temp_color.size()/4;

	//for(int i=0,j=0;i<curv_ver.size()-1;i=i+2,j++)
		//TransferFunc_alpha[j] = curv_ver[i+1];	
	
	//TransferFunc_Update();
	//glw->hist_gridsize = hist_gridsize;
}

void GLWidget_TranFunc::Recal_CtrP()
{	
	/*
	crv->ctrP = ctrl_ver;
	temp_ctrp = crv->curve_interp((ctrl_ver.size()/2)-2);
	crv->ctrP = temp_ctrp;
	curv_ver = crv->UniCubBSP(false);

	if(ctrlp_change)
	{
		//delete [] TransferFunc_alpha;
		//TransferFunc_alpha = new float [curv_ver.size()/2];

		//delete [] TransferFunc_color;
		//TransferFunc_color = new float [ctrl_ver.size()/2*3];

		//TransferFunc_color = &CtrlP_color[0];
	}
	for(int i=0,j=0;i<curv_ver.size()-1;i=i+2,j++)
	{
		TransferFunc_alpha[j] = curv_ver[i+1]>=0 ? curv_ver[i+1]:0;
		TransferFunc_alpha[j] = TransferFunc_alpha[j]<=1 ? TransferFunc_alpha[j]:0;
	}
	*/
	TnasferF_Alpha_Comp();

	TnasferF_Color_Comp();

	glw->TransferFunc_alpha = TransferFunc_alpha;
	glw->TransferFunc_color = TransferFunc_color;
	glw->num_alpha = temp_alpha.size();
	//glw->num_color = temp_color.size()/3;
	glw->num_color = temp_color.size()/4;

	//glw->hist_gridsize = hist_gridsize;

#if 0
	if(glw->volume_mapper!=1)
	{
		glw->Change_Alpha();
		glw->Change_Color();
	}
#endif

	updateGL();
	glw->updateGL();
}

void GLWidget_TranFunc::show_moment()
{
	glPointSize(5);
	glColor3f(hist_brightness,hist_brightness,hist_brightness);
	glBegin(GL_POINTS);
	for(int i=0;i<=screen_width;i++)
	{
		int x = (float)i/(float)screen_width*(float)(moment_width-1)+0.5;
		for(int j=0;j<=screen_height;j++)
		{
			int y = (float)j/(float)screen_height*(float)(moment_height-1)+0.5;
			if(moment_matrx[x][y]>0)
				glVertex2f(i,j);
		}
	}
	glEnd();
}

void GLWidget_TranFunc::show_histogram()
{
	glPointSize(5);
	glColor3f(hist_brightness,hist_brightness,hist_brightness);
	glBegin(GL_POINTS);
	for(int i=0;i<=screen_width;i++)
	{
		
		float x = (float)i/(float)screen_width*(float)(hist_gridsize-1);
		float y_bound = (histc[(int)ceil(x)]-histc[(int)floor(x)])*(x-floor(x))+histc[(int)floor(x)];
		y_bound *= hist_scale;

		for(int j=0;j<=screen_height;j++)
		{
			float y = (float)j/((float)screen_height);
			if(y<=y_bound)
				glVertex2f(i,j);			
			
		}
		
	}
	glEnd();
}

void GLWidget_TranFunc::show_transferfunc_curv()
{
	glLineWidth(1);
	glColor3f(1.0,1.0,1.0);
	glBegin(GL_LINES);
	if(curve_transfeFunc)
	{
		for(int i=0;i<curv_ver.size()-2;i=i+2)
		{					
			glVertex2f(curv_ver[i]*screen_width,curv_ver[i+1]*screen_height);
			glVertex2f(curv_ver[i+2]*screen_width,curv_ver[i+3]*screen_height);
		}
	}
	else
	{
		for(int i=0;i<ctrl_ver.size()-2;i=i+2)
		{
			glVertex2f(ctrl_ver[i]*screen_width,ctrl_ver[i+1]*screen_height);
			glVertex2f(ctrl_ver[i+2]*screen_width,ctrl_ver[i+3]*screen_height);
		}
	}
	glEnd();
}

void GLWidget_TranFunc::show_ctrlP()
{
	glPointSize(15);	
	glBegin(GL_POINTS);	
	for(int i=0,j=0;i<ctrl_ver.size();i=i+2,j=j+4)
	{
		glColor3f(CtrlP_color[j],CtrlP_color[j+1],CtrlP_color[j+2]);
		glVertex2f(ctrl_ver[i]*screen_width,ctrl_ver[i+1]*screen_height);	
	}
	glEnd();
	
	/*
	glColor3f(0.5,0.0,0.0);
	glBegin(GL_POINTS);	
	for(int i=0;i<temp_ctrp.size();i=i+2)
		glVertex2f(temp_ctrp[i]*screen_width,temp_ctrp[i+1]*screen_height);	
	glEnd();

	
	glBegin(GL_LINES);
	for(int i=0;i<temp_ctrp.size()-2;i=i+2)
	{
		glVertex2f(temp_ctrp[i]*screen_width,temp_ctrp[i+1]*screen_height);
		glVertex2f(temp_ctrp[i+2]*screen_width,temp_ctrp[i+3]*screen_height);	
	}
	glEnd();
	*/
}

void GLWidget_TranFunc::draw_ctlPLine()
{
	glLineWidth(1);
	glColor3f(0.5,0.5,0.5);
	glBegin(GL_LINES);
	for(int i=0;i<ctrl_ver.size()-2;i=i+2)
	{					
		glVertex2f(ctrl_ver[i]*screen_width,0);
		glVertex2f(ctrl_ver[i]*screen_width,screen_height);
	}
	glEnd();
}
void GLWidget_TranFunc::draw_background_color()
{
	int x_stepnum = (float)screen_width/(temp_color.size()/4)+1;

	float r,g,b;
	float alpha;

	glPointSize(1);
	glBegin(GL_POINTS);
	for(int i=0;i<screen_width;i++)
	{
		//float x = (float)i/(float)screen_width;
		int v= i/x_stepnum;		
		for(int j=0;j<screen_height;j++)
		{
			float ap = (float)j/(float)screen_height;
			glColor4f(temp_color[v*4],temp_color[v*4+1],temp_color[v*4+2],ap*0.01);
			glVertex2f(i,j);

		}
	}
	glEnd();
}
void GLWidget_TranFunc::mouseReleaseEvent( QMouseEvent * event )
{	
	 	
	 it_select = -1;
	  
}

void GLWidget_TranFunc::mouseMoveEvent(QMouseEvent *event)
{
     float dx = event->x();// - lastPos.x();
     float dy = event->y();// - lastPos.y();
	 
	 //printf("%d\t%d\n",event->x(),screen_height-event->y());

	 if(it_select!=-1)
	 {
		 //width boundary
		if(it_select == 0)
			ctrl_ver[it_select] = 0;
		else if(it_select == (ctrl_ver.size()-2))
			ctrl_ver[it_select] = 1;
		else
		{
			ctrl_ver[it_select] = dx/screen_width;
			if(ctrl_ver[it_select]>ctrl_ver[it_select+2]-(pt_interval_boundary/screen_width))
				ctrl_ver[it_select] = ctrl_ver[it_select+2]-(pt_interval_boundary/screen_width);

			if(ctrl_ver[it_select]<ctrl_ver[it_select-2]+(pt_interval_boundary/screen_width))
				ctrl_ver[it_select] = ctrl_ver[it_select-2]+(pt_interval_boundary/screen_width);
		}
			
		ctrl_ver[it_select+1] = (screen_height-dy)/screen_height;		

		// height boundary
		if(ctrl_ver[it_select+1]>1)
			ctrl_ver[it_select+1] = 1;
		if(ctrl_ver[it_select+1]<0)
			ctrl_ver[it_select+1] = 0;

		show_value(dx,screen_height-dy);

		//Recal_CtrP();
		emit ctrlP_changed();
	 }

	 /*
     if (event->buttons() & Qt::LeftButton) {
         setYRotation(yRot + (360 * (float)dy / (float)screen_height));
         setXRotation(xRot + (360 * (float)dx / (float)screen_width));
     }
	 */
     //lastPos = event->pos();
 }

void GLWidget_TranFunc::mousePressEvent(QMouseEvent *event)
 {
	 

     lastPos = event->pos();
	
	 if(event->buttons() & Qt::LeftButton )
	 {
		// printf("%d\t%d\n",event->x(),screen_height-event->y());
		 if(ctrl_ver.size()>0)
		 {
			 int match[] = {event->x(),screen_height-event->y()};

			 for(int i=0;i<ctrl_ver.size();i=i+2)
				 if((ctrl_ver[i]*screen_width)<=match[0]+pt_interval_boundary && (ctrl_ver[i]*screen_width)>=match[0]-pt_interval_boundary)
					 if((ctrl_ver[i+1]*screen_height)<=match[1]+pt_interval_boundary && (ctrl_ver[i+1]*screen_height)>=match[1]-pt_interval_boundary)
					 {
						 it_select = i;
						 break;
						 //updateGL();						 
					 }			
		 }
		  
		 show_value(lastPos.x(),screen_height-lastPos.y());
	 }

	 if(event->buttons() & Qt::RightButton )
     {
        //int x, y;
        //x= lastPos.x();
		//y= lastPos.y();
		//printf("%d\t%d\n",x,y);
		float x,y,x1;
		if(ctrl_ver.size()>0)
		{
			 int match[] = {event->x(),screen_height-event->y()};
			 for(int i=0;i<ctrl_ver.size();i=i+2)
			 {
				 x = ctrl_ver[i]*screen_width;				 
				 y = ctrl_ver[i+1]*screen_height;
				 if(i<ctrl_ver.size()-2)
				 {
					 x1 = ctrl_ver[i+2]*screen_width; //next x
					 if((x-match[0])*(x1-match[0])<0)
						 edit_select = i/2;
				 }

				 if(x<=match[0]+pt_interval_boundary && x>=match[0]-pt_interval_boundary)
				 {
					 
					 if(y<=match[1]+pt_interval_boundary && y>=match[1]-pt_interval_boundary)
					 {
						 it_select = i;
						 break;
						 					 
					 }			
				 }
			 }
		}
		if(it_select==-1)
		{
			QMenu* contextMenu = new QMenu ( this );
			Q_CHECK_PTR ( contextMenu );
			contextMenu->addAction  ( "Add New Control Point" , this , SLOT (Add_CtrP()) );
			contextMenu->exec ( QCursor::pos() );
			delete contextMenu;
			contextMenu = 0;
		}
		else
		{
			QMenu* contextMenu = new QMenu ( this );
			Q_CHECK_PTR ( contextMenu );
			if(it_select!=0 && it_select!=ctrl_ver.size()-2)
				contextMenu->addAction  ( "Delete Control Point" , this , SLOT (Delete_CtrP()) );
			contextMenu->addAction  ( "Change Color" , this , SLOT (Change_Color()) );
			contextMenu->exec ( QCursor::pos() );
			delete contextMenu;
			contextMenu = 0;
		}
        //DumpPixel( x, y );
     }
	 
}


void GLWidget_TranFunc::show_value(int px, int py)
{	
	pointx = (float)px/(float)screen_width;
	
	alphavalue = pointx*(float)temp_alpha.size()+0.5;
	alphavalue = temp_alpha[(int)alphavalue];

	emit show_value_changed();
	
}

void GLWidget_TranFunc::Add_CtrP()
{
	ctrl_ver.insert(ctrl_ver.begin()+(edit_select+1)*2,1,(float)lastPos.x()/(float)screen_width);
	ctrl_ver.insert(ctrl_ver.begin()+(edit_select+1)*2+1,1,(float)(screen_height-lastPos.y())/(float)screen_height);
	
	float colorinterp_r,colorinterp_g,colorinterp_b;
	colorinterp_r = (CtrlP_color[edit_select*4]+CtrlP_color[(edit_select+1)*4])/2;
	colorinterp_g = (CtrlP_color[edit_select*4+1]+CtrlP_color[(edit_select+1)*4+1])/2;
	colorinterp_b = (CtrlP_color[edit_select*4+2]+CtrlP_color[(edit_select+1)*4+2])/2;

	colorinterp_r = colorinterp_r>=0 ? colorinterp_r:0;
	colorinterp_r = colorinterp_r<=1 ? colorinterp_r:1;
	colorinterp_g = colorinterp_g>=0 ? colorinterp_g:0;
	colorinterp_g = colorinterp_g<=1 ? colorinterp_g:1;
	colorinterp_b = colorinterp_b>=0 ? colorinterp_b:0;
	colorinterp_b = colorinterp_b<=1 ? colorinterp_b:1;

	CtrlP_color.insert(CtrlP_color.begin()+(edit_select+1)*4,1,colorinterp_r);	
	CtrlP_color.insert(CtrlP_color.begin()+(edit_select+1)*4+1,1,colorinterp_g);	
	CtrlP_color.insert(CtrlP_color.begin()+(edit_select+1)*4+2,1,colorinterp_b);
	CtrlP_color.insert(CtrlP_color.begin()+(edit_select+1)*4+3,1,0);
		
	//Recal_CtrP();
	emit ctrlP_changed();
	//updateGL();
}
void GLWidget_TranFunc::Delete_CtrP()
{
	int idx = it_select/2;
	ctrl_ver.erase(ctrl_ver.begin()+idx*2);
	ctrl_ver.erase(ctrl_ver.begin()+idx*2);

	for(int i=0;i<4;i++)
		CtrlP_color.erase(CtrlP_color.begin()+idx*4);

	//Recal_CtrP();
	//updateGL();
	emit ctrlP_changed();
}
void GLWidget_TranFunc::Change_Color()
{
	QColorDialog* mypal = new QColorDialog (this);
	QColor rgb = mypal->getColor();
	
	if(rgb.isValid())
	{
		int idx = (it_select/2)*4;
		//printf("%f\t%f\t%f\n",rgb.redF(),rgb.greenF(),rgb.blueF());
		CtrlP_color[idx] = rgb.redF();
		CtrlP_color[idx+1] = rgb.greenF();
		CtrlP_color[idx+2] = rgb.blueF();
		CtrlP_color[idx+3] = 0;

		TnasferF_Color_Comp();

		glw->TransferFunc_color = TransferFunc_color;
		glw->Change_Color();

		updateGL();
		glw->updateGL();
	}
}

void GLWidget_TranFunc::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, screen_width-1, 0, screen_height-1);
	
	glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable ( GL_COLOR_MATERIAL );

	if(*hist_exist)
	{
		if(scalarV_type==0)
			show_histogram();
		else
			show_moment();
	}
	show_transferfunc_curv();
	show_ctrlP();
	draw_ctlPLine();
	//draw_background_color();
}

void GLWidget_TranFunc::resizeGL(int width, int height) {

	if(height == 0) height=1;
    screen_width=width;
	screen_height=height;	
    glViewport(0, 0, width, height);
	//printf("width:%d\theight:%d\n",width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(FOV,((GLfloat)width)/((GLfloat)height),0.2f,1000.0f);
    glMatrixMode(GL_MODELVIEW);    
}



void GLWidget_TranFunc::initializeGL() {
	
#if use_glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		printf("Error: %s\n", glewGetErrorString(err));
	}
#endif
   // int mode=GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH;
    glClearColor(0.05f, 0.05f, 0.1f, 0.0f);     // Background => dark blue
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	//glMatrixMode(GL_PROJECTION); 
	//glLoadIdentity(); 
	
	/*
	glEnable(GL_LIGHTING);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
    GLfloat LightAmbient[]=		{ 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat LightDiffuse[]=		{ 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat LightSpecular[]=	{ 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat LightPosition[]=    { 40.0f, 40.0f, 40.0f ,0.0f};
    //GLfloat LightPosition2[]=   { 0.0f, 0.0f, -1.0f ,0.0f};

    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);
    glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);		// Setup The Ambient Light
    glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);		// Setup The Diffuse Light
    glLightfv(GL_LIGHT1, GL_SPECULAR,LightSpecular);	// Setup The Specular Light
	glEnable(GL_LIGHT1);
	*/
    //glLightfv(GL_LIGHT2, GL_POSITION, LightPosition2);
    //glLightfv(GL_LIGHT2, GL_AMBIENT, LightAmbient);		// Setup The Ambient Light
    //glLightfv(GL_LIGHT2, GL_DIFFUSE, LightDiffuse);		// Setup The Diffuse Light
    //glLightfv(GL_LIGHT2, GL_SPECULAR,LightSpecular);	// Setup The Specular Light
    //glEnable(GL_LIGHT2);


#if use_glew
   	init();
#endif
	
} 
