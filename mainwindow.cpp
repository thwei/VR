#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utility.h"

#define WINDOW_SIZE 800

extern "C" void init_volume_Cuda(float *h_volume, cudaExtent volumeSize);
extern "C" void init_normal_Cuda(myvector4 *h_volume, cudaExtent volumeSize);
extern "C" void freeCudaVolumeBuffers();
extern "C" void init_visibility_Cuda(float *h_volume, cudaExtent volumeSize);

uint cuda_width = WINDOW_SIZE, cuda_height = WINDOW_SIZE;
QGLFormat glFormat;
Grid g, h;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	DataDir = "D:/WorkingData";

	qDebug() << "OpenGL Versions Supported: " << QGLFormat::openGLVersionFlags();	
	glFormat.setVersion( 3, 1 );	    
    glFormat.setProfile( QGLFormat::CoreProfile ); 
    glFormat.setSampleBuffers( true );
	QGLFormat::setDefaultFormat(glFormat);

	init_Grid(g);
	tr1D = NULL;
	glw = NULL;
    ui->setupUi(this);

    h.data.volume = NULL;
#if 1
    g.data.volume = read_volume("/data/flow/plume.nhdr", g);
    ui->statusBar->showMessage("Read Data Property Successfully!");

    // create cubic visibility
    h.data.size = g.xdim*g.ydim*g.zdim;
    h.data.volume = new float[h.data.size];
    h.data.xdim = g.xdim;
    h.data.ydim = g.ydim;
    {
        int x,y,z,i=0;
        for (z=0; z<g.zdim; z++)
            for (y=0; y<g.ydim; y++)
                for (x=0; x<g.xdim; x++){
                    h.data.volume[i++] = z>g.zdim/3 && z<g.zdim*2/3 && y>g.ydim/3 && y<g.ydim*2/3 && x>g.xdim/3 && x<g.xdim*2/3;
                }
    }
#endif
}

MainWindow::~MainWindow()
{	
	if(tr1D!=NULL)
		delete tr1D;
    delete ui;
}


void MainWindow::on_actionRead_Data_triggered()
{
	QString fn = QFileDialog::getOpenFileName(this, tr("Open File"), DataDir, tr("header(*.nhdr)")); 
	ui->statusBar->showMessage("");
	
	if ( !fn.isEmpty() ) 
	{
		string filename = fn.toStdString();
		destroy_Grid(g);
		g.data.volume = read_volume(filename, g);

		ui->statusBar->showMessage("Read Data Property Successfully!");
	}
}

int iDivUpM(int a, int b)
{
    return (a % b != 0) ? (a / b + 1) : (a / b);
}

void MainWindow::on_Draw_PushButton_clicked()
{
		///////initGL	
	    if(glw!=NULL)
			delete glw;
		glw = new GLWidget(glFormat);
		glw->resize(WINDOW_SIZE,WINDOW_SIZE);

		int maxdim,mindim;

		maxdim = max(max(g.xdim,g.ydim),g.zdim);		
		mindim = min(min(g.xdim,g.ydim),g.zdim);

		glw->xdim = g.xdim;
		glw->ydim = g.ydim;
		glw->zdim = g.zdim;
		glw->maxdim = maxdim;
		glw->mindim = mindim;
		glw->Reset_View();	
		
        glw->stepsize = (1.0/(maxdim));
		//glw->stepsize = glw->stepsize*2

		extern dim3 blockSize;
		extern dim3 gridSize;

		if(tr1D!=NULL)
			delete tr1D;
		tr1D = new Transfer_Func1D();
		tr1D->glwtrf->glw = glw;
		tr1D->glwtrf->Init_TransFunc();
		tr1D->graph_show();
		tr1D->show();

		////initial CUDA
		cudaExtent volumeSize = make_cudaExtent(g.data.xdim, g.data.ydim, g.data.zdim);
		init_volume_Cuda(g.data.volume, volumeSize);
		gridSize = dim3(iDivUpM(cuda_width, blockSize.x), iDivUpM(cuda_height, blockSize.y));

        // initialize visibility array in CUDA
        if (h.data.volume) {
            init_visibility_Cuda(h.data.volume, volumeSize);
        }

		glw->updateGL();
		glw->show();

		
}


void MainWindow::on_actionDistRead_Data_triggered()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Open File"), DataDir, tr("header(*.nhdr)"));
    ui->statusBar->showMessage("");

    if ( !fn.isEmpty() )
    {
        string filename = fn.toStdString();
        destroy_Grid(h);
        h.data.volume = read_volume(filename, h);

        ui->statusBar->showMessage("Read Data Property Successfully!");
    }
}
