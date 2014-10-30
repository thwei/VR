#include "transfer_func1d.h"
#include "ui_transfer_func1d.h"
#include "utility.h"

extern Grid g;

Transfer_Func1D::Transfer_Func1D(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Transfer_Func1D)
{
	ui->setupUi(this);	

	ui->binsize_textEdit->setText(QString::number(1));
	binWidth = 1;
	hist_exist = false;

	glwtrf = new GLWidget_TranFunc(hist_exist,this);

	if(*data_exist)
	{
//		stsi = new Statistical_Signature(g.data.volume,g.data.xdim,g.data.ydim,g.data.zdim);
//		stsi->binWidth = binWidth;
//		info = new Info_Theory(g.data.volume,g.data.size); //initial binWidth is 1    
//		compute_histogram();
	}

	
	ui->gridLayout->addWidget(glwtrf, 0, 0, 1, 1);
	glwtrf->show();
	
	//glwtrf->Init_TransFunc(); //compute initial curve
	QObject::connect(glwtrf, SIGNAL(show_value_changed()), this, SLOT(on_changed_value()));	 
	//QObject::connect(this, SIGNAL(scalar_value_item_changed()), glwtrf, SLOT(scalar_value_type_changed())); 
}

Transfer_Func1D::~Transfer_Func1D()
{
	delete glwtrf;
	
    delete ui;
	if(*data_exist)
	{
//		delete info;
//		delete stsi;
	}
}

void Transfer_Func1D::on_binsize_textEdit_textChanged()
{
	QString str;
	str.append(ui->binsize_textEdit->toPlainText());
	binWidth = str.toFloat();
}

void Transfer_Func1D::graph_show()
{
	
	glwtrf->updateGL();
}

void Transfer_Func1D::compute_histogram()
{	
#if 0
	glwtrf->histc = info->histogram(binWidth,g.data.minvalue,g.data.maxvalue);
	glwtrf->hist_gridsize = info->hist_gridsize;
	hist_exist = true;
#endif
}

void Transfer_Func1D::on_Binsize_pushButton_clicked()
{
	if(data_exist)
	{
		compute_histogram();
		glwtrf->updateGL();
	}
}

void Transfer_Func1D::on_hist_scale_verticalSlider_valueChanged(int value)
{
	glwtrf->hist_scale = (float)(value);
	glwtrf->updateGL();
}

void Transfer_Func1D::on_hist_brightness_horizontalSlider_valueChanged(int value)
{
	glwtrf->hist_brightness = (float)value/100.0f; //maximum = 1000;
	glwtrf->updateGL();
}

void Transfer_Func1D::on_Curve_radioButton_clicked(bool checked)
{
	if(checked)
	{
		if((glwtrf->curve_transfeFunc)==false)
		{
			glwtrf->curve_transfeFunc = true;
			glwtrf->Recal_CtrP();
		}
	}
	else
		glwtrf->curve_transfeFunc = false;
	
	//glwtrf->updateGL();

}

void Transfer_Func1D::on_StraightLine_radioButton_clicked(bool checked)
{
	if(checked)
	{
		if(glwtrf->curve_transfeFunc)
		{
			glwtrf->curve_transfeFunc = false;
			glwtrf->Recal_CtrP();
		}
	}
	else
		glwtrf->curve_transfeFunc = true;

	//glwtrf->updateGL();
}

void Transfer_Func1D::on_changed_value()
{
	float minValue = glwtrf->glw->minValue;
	float maxValue = glwtrf->glw->maxValue;
	float ValueRange = glwtrf->glw->ValueRange;

	float iso = (glwtrf->pointx)*ValueRange+minValue;

	iso = iso>=minValue ? iso:minValue;
	iso = iso<=maxValue ? iso:maxValue;

	ui->show_iso_textEdit->setText(QString::number(iso));
	ui->show_alpha_textEdit->setText(QString::number(glwtrf->alphavalue));
}
#if 0
void Transfer_Func1D::compute_skewkurtosis(int option)
{
	stsi->hist_gridsize = glwtrf->hist_gridsize;
	moment_w = 5; //must be odd
	glwtrf->moment_matrx = stsi->construct_moment_matrx(moment_w,option);
	glwtrf->moment_width = stsi->hist_gridsize;
	glwtrf->moment_height = stsi->moment_height;
}

void Transfer_Func1D::compute_moment(int degree)
{
	stsi->hist_gridsize = glwtrf->hist_gridsize;
	moment_w = 5; //must be odd
	glwtrf->moment_matrx = stsi->construct_moment_matrx(moment_w,2,degree);
	glwtrf->moment_width = stsi->hist_gridsize;
	glwtrf->moment_height = stsi->moment_height;
}
#endif

void Transfer_Func1D::on_ScalarV_comboBox_currentIndexChanged(int index)
{
#if 0
	if(data_exist)
	{
		glwtrf->scalarV_type = index;
		switch(index)
		{
		case 0:
			compute_histogram();
			break;
		case 1:
			compute_moment(2);
			break;
		case 2:
			compute_moment(3);
			break;
		case 3:
			compute_moment(4);
			break;
		case 4:
			compute_skewkurtosis(0);
			break;
		case 5:
			compute_skewkurtosis(1);
			break;
		}
		glwtrf->updateGL();
	}
#endif
}

void Transfer_Func1D::closeEvent( QCloseEvent * event )
{
	emit tr1D_window_closed();
}

void Transfer_Func1D::on_comboBox_currentIndexChanged(int index)
{
	glwtrf->color_type = index;
	glwtrf->Init_TransFunc();

	glwtrf->glw->Change_Color();
	glwtrf->updateGL();
	glwtrf->glw->updateGL();
}
