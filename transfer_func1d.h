#ifndef TRANSFER_FUNC1D_H
#define TRANSFER_FUNC1D_H

#include <QWidget>
#include "glwidget_tranfunc.h"
//#include "Info_Theory.h"
#include "curve.h"
//#include "Statistical_Signature.h"

namespace Ui {
class Transfer_Func1D;
}

class Transfer_Func1D : public QWidget
{
    Q_OBJECT
    
public:
    explicit Transfer_Func1D(QWidget *parent = 0);
    ~Transfer_Func1D();
	void graph_show();
	GLWidget_TranFunc *glwtrf;
	bool hist_exist;
	
signals:
	void scalar_value_item_changed();
	void tr1D_window_closed();

public slots:
	void on_changed_value();

private slots:
    void on_binsize_textEdit_textChanged();

    void on_Binsize_pushButton_clicked();

    void on_hist_scale_verticalSlider_valueChanged(int value);

    void on_hist_brightness_horizontalSlider_valueChanged(int value);

    void on_Curve_radioButton_clicked(bool checked);

    void on_StraightLine_radioButton_clicked(bool checked);	

    void on_ScalarV_comboBox_currentIndexChanged(int index);

    void on_comboBox_currentIndexChanged(int index);

protected:
	void closeEvent( QCloseEvent * event );

private:
    Ui::Transfer_Func1D *ui;
	
//	Info_Theory *info;
	float binWidth;
	void compute_histogram();
	//void compute_moment(int degree);
	//void compute_skewkurtosis(int option);
	//Statistical_Signature *stsi;
	int moment_w;
	
	bool *data_exist;
	
};

#endif // TRANSFER_FUNC1D_H
