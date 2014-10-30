#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utility.h"

#define WINDOW_SIZE 800

extern "C" void init_volume_Cuda(float *h_volume, cudaExtent volumeSize);
extern "C" void init_normal_Cuda(myvector4 *h_volume, cudaExtent volumeSize);
extern "C" void freeCudaVolumeBuffers();


uint cuda_width = WINDOW_SIZE, cuda_height = WINDOW_SIZE;

Grid g;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	DataDir = "D:/WorkingData";
	tr1D = NULL;
	glw = NULL;
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{	
	if(tr1D!=NULL)
		delete tr1D;
    delete ui;
}


void MainWindow::on_actionRead_Data_triggered()
{
	QString fn = QFileDialog::getOpenFileName(this, tr("Open File"), DataDir, tr("raw data(*.raw *.dat)")); 
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
		//initGL();	

		extern dim3 blockSize;
		extern dim3 gridSize;

		if(tr1D!=NULL)
			delete tr1D;
		tr1D = new Transfer_Func1D();
		tr1D->glwtrf->glw = glw;
		tr1D->glwtrf->Init_TransFunc();


/*		//if(volume_mapper==1) //gpu based
		cudaExtent volumeSize = make_cudaExtent(g.data.xdim, g.data.ydim, g.data.zdim);
		init_volume_Cuda(g.data.volume, volumeSize);
		gridSize = dim3(iDivUpM(cuda_width, blockSize.x), iDivUpM(cuda_height, blockSize.y));

		//iCuda();
		glw->updateGL();
		glw->show();

		QObject::connect(glw, SIGNAL(glw_window_closed()), this, SLOT(Close_Render_Checked()));
		QObject::connect(tr1D, SIGNAL(tr1D_window_closed()), this, SLOT(Close_TR1D_Checked()));
		*/
		//UpdateUi();
}

