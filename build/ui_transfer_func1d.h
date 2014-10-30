/********************************************************************************
** Form generated from reading UI file 'transfer_func1d.ui'
**
** Created: Thu Oct 30 15:06:32 2014
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TRANSFER_FUNC1D_H
#define UI_TRANSFER_FUNC1D_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Transfer_Func1D
{
public:
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox;
    QGroupBox *hist_groupBox;
    QPushButton *Binsize_pushButton;
    QLabel *label;
    QTextEdit *binsize_textEdit;
    QLabel *hist_label;
    QSlider *hist_brightness_horizontalSlider;
    QGroupBox *TransferFunc_groupBox;
    QRadioButton *Curve_radioButton;
    QRadioButton *StraightLine_radioButton;
    QLabel *alpha_label;
    QTextEdit *show_alpha_textEdit;
    QLabel *isovalue_label;
    QTextEdit *show_iso_textEdit;
    QGroupBox *groupBox_2;
    QRadioButton *ScalarV_radioButton;
    QComboBox *ScalarV_comboBox;
    QComboBox *comboBox;
    QGridLayout *gridLayout;
    QSlider *hist_scale_verticalSlider;
    QSpacerItem *verticalSpacer;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QWidget *Transfer_Func1D)
    {
        if (Transfer_Func1D->objectName().isEmpty())
            Transfer_Func1D->setObjectName(QString::fromUtf8("Transfer_Func1D"));
        Transfer_Func1D->resize(1131, 599);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Transfer_Func1D->sizePolicy().hasHeightForWidth());
        Transfer_Func1D->setSizePolicy(sizePolicy);
        gridLayout_2 = new QGridLayout(Transfer_Func1D);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        groupBox = new QGroupBox(Transfer_Func1D);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Ignored, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy1);
        hist_groupBox = new QGroupBox(groupBox);
        hist_groupBox->setObjectName(QString::fromUtf8("hist_groupBox"));
        hist_groupBox->setGeometry(QRect(0, 20, 161, 141));
        QFont font;
        font.setPointSize(10);
        hist_groupBox->setFont(font);
        Binsize_pushButton = new QPushButton(hist_groupBox);
        Binsize_pushButton->setObjectName(QString::fromUtf8("Binsize_pushButton"));
        Binsize_pushButton->setGeometry(QRect(10, 60, 141, 23));
        label = new QLabel(hist_groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 24, 51, 21));
        label->setFont(font);
        binsize_textEdit = new QTextEdit(hist_groupBox);
        binsize_textEdit->setObjectName(QString::fromUtf8("binsize_textEdit"));
        binsize_textEdit->setGeometry(QRect(90, 20, 61, 31));
        QFont font1;
        font1.setPointSize(12);
        binsize_textEdit->setFont(font1);
        binsize_textEdit->setLayoutDirection(Qt::LeftToRight);
        hist_label = new QLabel(hist_groupBox);
        hist_label->setObjectName(QString::fromUtf8("hist_label"));
        hist_label->setGeometry(QRect(10, 90, 141, 16));
        hist_brightness_horizontalSlider = new QSlider(hist_groupBox);
        hist_brightness_horizontalSlider->setObjectName(QString::fromUtf8("hist_brightness_horizontalSlider"));
        hist_brightness_horizontalSlider->setGeometry(QRect(10, 110, 141, 20));
        hist_brightness_horizontalSlider->setMaximum(100);
        hist_brightness_horizontalSlider->setValue(20);
        hist_brightness_horizontalSlider->setOrientation(Qt::Horizontal);
        TransferFunc_groupBox = new QGroupBox(groupBox);
        TransferFunc_groupBox->setObjectName(QString::fromUtf8("TransferFunc_groupBox"));
        TransferFunc_groupBox->setGeometry(QRect(0, 160, 161, 81));
        TransferFunc_groupBox->setFont(font);
        Curve_radioButton = new QRadioButton(TransferFunc_groupBox);
        Curve_radioButton->setObjectName(QString::fromUtf8("Curve_radioButton"));
        Curve_radioButton->setGeometry(QRect(10, 20, 91, 17));
        Curve_radioButton->setChecked(false);
        StraightLine_radioButton = new QRadioButton(TransferFunc_groupBox);
        StraightLine_radioButton->setObjectName(QString::fromUtf8("StraightLine_radioButton"));
        StraightLine_radioButton->setGeometry(QRect(10, 40, 121, 17));
        StraightLine_radioButton->setChecked(true);
        alpha_label = new QLabel(groupBox);
        alpha_label->setObjectName(QString::fromUtf8("alpha_label"));
        alpha_label->setGeometry(QRect(10, 521, 51, 21));
        alpha_label->setFont(font);
        show_alpha_textEdit = new QTextEdit(groupBox);
        show_alpha_textEdit->setObjectName(QString::fromUtf8("show_alpha_textEdit"));
        show_alpha_textEdit->setGeometry(QRect(70, 520, 81, 31));
        show_alpha_textEdit->setFont(font);
        isovalue_label = new QLabel(groupBox);
        isovalue_label->setObjectName(QString::fromUtf8("isovalue_label"));
        isovalue_label->setGeometry(QRect(10, 450, 71, 21));
        isovalue_label->setFont(font);
        show_iso_textEdit = new QTextEdit(groupBox);
        show_iso_textEdit->setObjectName(QString::fromUtf8("show_iso_textEdit"));
        show_iso_textEdit->setGeometry(QRect(10, 481, 141, 31));
        show_iso_textEdit->setFont(font);
        groupBox_2 = new QGroupBox(groupBox);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(0, 240, 151, 81));
        groupBox_2->setFont(font);
        ScalarV_radioButton = new QRadioButton(groupBox_2);
        ScalarV_radioButton->setObjectName(QString::fromUtf8("ScalarV_radioButton"));
        ScalarV_radioButton->setGeometry(QRect(10, 20, 111, 17));
        ScalarV_radioButton->setChecked(true);
        ScalarV_comboBox = new QComboBox(groupBox_2);
        ScalarV_comboBox->setObjectName(QString::fromUtf8("ScalarV_comboBox"));
        ScalarV_comboBox->setGeometry(QRect(20, 40, 121, 22));
        comboBox = new QComboBox(groupBox);
        comboBox->setObjectName(QString::fromUtf8("comboBox"));
        comboBox->setGeometry(QRect(10, 330, 131, 22));

        gridLayout_2->addWidget(groupBox, 0, 0, 1, 1);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));

        gridLayout_2->addLayout(gridLayout, 0, 1, 1, 1);

        hist_scale_verticalSlider = new QSlider(Transfer_Func1D);
        hist_scale_verticalSlider->setObjectName(QString::fromUtf8("hist_scale_verticalSlider"));
        hist_scale_verticalSlider->setMinimum(1);
        hist_scale_verticalSlider->setMaximum(100);
        hist_scale_verticalSlider->setValue(1);
        hist_scale_verticalSlider->setOrientation(Qt::Vertical);

        gridLayout_2->addWidget(hist_scale_verticalSlider, 0, 2, 1, 1);

        verticalSpacer = new QSpacerItem(20, 551, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer, 0, 3, 1, 1);

        horizontalSpacer = new QSpacerItem(152, 18, QSizePolicy::Fixed, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer, 1, 0, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(875, 18, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 1, 1, 1, 1);


        retranslateUi(Transfer_Func1D);

        QMetaObject::connectSlotsByName(Transfer_Func1D);
    } // setupUi

    void retranslateUi(QWidget *Transfer_Func1D)
    {
        Transfer_Func1D->setWindowTitle(QApplication::translate("Transfer_Func1D", "1D Transfer Function", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("Transfer_Func1D", "GroupBox", 0, QApplication::UnicodeUTF8));
        hist_groupBox->setTitle(QApplication::translate("Transfer_Func1D", "Histogram", 0, QApplication::UnicodeUTF8));
        Binsize_pushButton->setText(QApplication::translate("Transfer_Func1D", "Change Bin Size", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("Transfer_Func1D", "Bin Size", 0, QApplication::UnicodeUTF8));
        hist_label->setText(QApplication::translate("Transfer_Func1D", "Histogram Brightness", 0, QApplication::UnicodeUTF8));
        TransferFunc_groupBox->setTitle(QApplication::translate("Transfer_Func1D", "Transfer Function", 0, QApplication::UnicodeUTF8));
        Curve_radioButton->setText(QApplication::translate("Transfer_Func1D", "Curve", 0, QApplication::UnicodeUTF8));
        StraightLine_radioButton->setText(QApplication::translate("Transfer_Func1D", "Straight Line", 0, QApplication::UnicodeUTF8));
        alpha_label->setText(QApplication::translate("Transfer_Func1D", "alpha:", 0, QApplication::UnicodeUTF8));
        isovalue_label->setText(QApplication::translate("Transfer_Func1D", "Isovalue:", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("Transfer_Func1D", "GroupBox", 0, QApplication::UnicodeUTF8));
        ScalarV_radioButton->setText(QApplication::translate("Transfer_Func1D", "Scalar Value", 0, QApplication::UnicodeUTF8));
        ScalarV_comboBox->clear();
        ScalarV_comboBox->insertItems(0, QStringList()
         << QApplication::translate("Transfer_Func1D", "Frequency", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Transfer_Func1D", "Moment 2", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Transfer_Func1D", "Moment 3", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Transfer_Func1D", "Moment 4", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Transfer_Func1D", "Skewness", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Transfer_Func1D", "Kurtosis", 0, QApplication::UnicodeUTF8)
        );
        comboBox->clear();
        comboBox->insertItems(0, QStringList()
         << QApplication::translate("Transfer_Func1D", "Red to Blue  Rainbow", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Transfer_Func1D", "Blue to Red  Rainbow", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("Transfer_Func1D", "Rainbow Blended White", 0, QApplication::UnicodeUTF8)
        );
    } // retranslateUi

};

namespace Ui {
    class Transfer_Func1D: public Ui_Transfer_Func1D {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TRANSFER_FUNC1D_H
