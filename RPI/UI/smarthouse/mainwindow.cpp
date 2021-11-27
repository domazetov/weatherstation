#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <inttypes.h>

#include <QMouseEvent>
#include <QtWidgets>

#define MAX_DEVICES_PER_ROW 4
#define MAX_DEVICES         12

static bool toggle_fullscreen = true;
static uint8_t device_counter = 0;
static int frame_1_counter = 0;
static int frame_2_counter = 0;
static int frame_3_counter = 0;
int m_nMouseClick_X_Coordinate;
int m_nMouseClick_Y_Coordinate;

typedef struct QSensor
{
    QLabel* sensor_label;
    QProgressBar* sensor_bar;
}QSensor;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QObject::connect(ui->add_device, &QPushButton::clicked, this, &MainWindow::onAddWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    m_nMouseClick_X_Coordinate = event->x();
    m_nMouseClick_Y_Coordinate = event->y();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(!toggle_fullscreen)
    {
        move(event->globalX()-m_nMouseClick_X_Coordinate,event->globalY()-m_nMouseClick_Y_Coordinate);
    }
}

//void MainWindow::onAddWidget()
//{
//    QHBoxLayout* Vlayout = qobject_cast<QHBoxLayout*>(ui->frame->layout());

//    QVBoxLayout* Hlayout = new QVBoxLayout(ui->frame);

//    QString buttonText = tr("%1").arg(ui->device_name->toPlainText());
//    // QString buttonText = tr("Button: #%1").arg(Vlayout->count());
//    QPushButton* button = new QPushButton(buttonText, ui->frame);
//    button->setMaximumWidth(316);
//    Hlayout->addWidget(button);

//    QCheckBox* checkbox = new QCheckBox("Check", ui->frame);
//    Hlayout->addWidget(checkbox);

//    QProgressBar* dht_temp = new QProgressBar();
//    dht_temp->setValue(10);
//    Hlayout->addWidget(dht_temp);

//    QProgressBar* dht_humi = new QProgressBar();
//    dht_humi->setValue(50);
//    Hlayout->addWidget(dht_humi);

//    QProgressBar* bmp_temp = new QProgressBar();
//    bmp_temp->setValue(100);
//    Hlayout->addWidget(bmp_temp);

//    QProgressBar* bmp_pres = new QProgressBar();
//    bmp_pres->setFormat("%vhPa");
//    bmp_pres->setMinimum(900);
//    bmp_pres->setMaximum(1200);
//    bmp_pres->setValue(1000);
//    Hlayout->addWidget(bmp_pres);

//    Vlayout->insertLayout(0, Hlayout);

//    mButtonToLayoutMap.insert(button, Hlayout);

//    QObject::connect(button, &QPushButton::clicked, this, &MainWindow::onRemoveWidget);
//}

QSensor dht_humidity()
{
    QSensor dht_humi;

    dht_humi.sensor_label = new QLabel("Humidity");
    dht_humi.sensor_bar = new QProgressBar();
    dht_humi.sensor_bar->setFormat("%v%");
    dht_humi.sensor_bar->setMinimum(0);
    dht_humi.sensor_bar->setMaximum(100);
    dht_humi.sensor_bar->setValue(55);
    dht_humi.sensor_bar->setStyleSheet("QProgressBar::chunk { background-color: #146acc }");

    return dht_humi;
}

QSensor dht_temperature()
{
    QSensor dht_temp;

    dht_temp.sensor_label = new QLabel("Temperature 1:");
    dht_temp.sensor_bar = new QProgressBar();
    dht_temp.sensor_bar->setFormat("%v°C");
    dht_temp.sensor_bar->setMinimum(0);
    dht_temp.sensor_bar->setMaximum(50);
    dht_temp.sensor_bar->setValue(25);
    dht_temp.sensor_bar->setStyleSheet("QProgressBar::chunk { background-color: #b87b00 }");

    return dht_temp;
}

QSensor bmp_temperature()
{
    QSensor bmp_temp;

    bmp_temp.sensor_label = new QLabel("Temperature 2:");
    bmp_temp.sensor_bar = new QProgressBar();
    bmp_temp.sensor_bar->setFormat("%v°C");
    bmp_temp.sensor_bar->setMinimum(0);
    bmp_temp.sensor_bar->setMaximum(50);
    bmp_temp.sensor_bar->setValue(30);
    bmp_temp.sensor_bar->setStyleSheet("QProgressBar::chunk { background-color: #b82e00 }");

    return bmp_temp;
}

QSensor bmp_pressure()
{
    QSensor bmp_pres;

    bmp_pres.sensor_label = new QLabel("Pressure:");
    bmp_pres.sensor_bar = new QProgressBar();
    bmp_pres.sensor_bar->setFormat("%vhPa");
    bmp_pres.sensor_bar->setMinimum(950);
    bmp_pres.sensor_bar->setMaximum(1050);
    bmp_pres.sensor_bar->setValue(1000);
    bmp_pres.sensor_bar->setStyleSheet("QProgressBar::chunk { background-color: #5600b8 }");

    return bmp_pres;
}


void MainWindow::WeatherStation(QFormLayout* layout, QHBoxLayout* Hlayout)
{
    QLabel* device_name = new QLabel(ui->device_name->toPlainText());

    QSensor dht_humi = dht_humidity();
    QSensor dht_temp = dht_temperature();
    QSensor bmp_temp = bmp_temperature();
    QSensor bmp_pres = bmp_pressure();

    QPushButton* remove_device = new QPushButton("Remove device");

    layout->addRow(device_name);
    layout->addRow(dht_humi.sensor_label, dht_humi.sensor_bar);
    layout->addRow(dht_temp.sensor_label, dht_temp.sensor_bar);
    layout->addRow(bmp_temp.sensor_label, bmp_temp.sensor_bar);
    layout->addRow(bmp_pres.sensor_label, bmp_pres.sensor_bar);
    layout->addRow(remove_device);

    ButtonToLayoutMap.insert(remove_device, layout);
    ButtonToHLayoutMap.insert(remove_device, Hlayout);

    QObject::connect(remove_device, &QPushButton::clicked, this, &MainWindow::onRemoveWidget);
    device_counter++;
}

void MainWindow::onAddWidget()
{
    if(device_counter < MAX_DEVICES)
    {
        QHBoxLayout* Hlayout = qobject_cast<QHBoxLayout*>(ui->frame_4->layout());;
        QFormLayout* layout = new QFormLayout();

        if(MAX_DEVICES_PER_ROW > frame_1_counter)
        {
            Hlayout = qobject_cast<QHBoxLayout*>(ui->frame_1->layout());
            frame_1_counter++;
        }
        else if(MAX_DEVICES_PER_ROW > frame_2_counter)
        {
            Hlayout = qobject_cast<QHBoxLayout*>(ui->frame_2->layout());
            frame_2_counter++;
        }
        else if(MAX_DEVICES_PER_ROW > frame_3_counter)
        {
            Hlayout = qobject_cast<QHBoxLayout*>(ui->frame_3->layout());
            frame_3_counter++;
        }
        WeatherStation(layout, Hlayout);

        Hlayout->insertLayout(device_counter,layout);
    }
    else
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Warning");
        msgBox.setText("Maximum amount of devices reached!");
        msgBox.setStyleSheet("QLabel {min-width: 220px; background-color: #3a3a3a; border-width: 0;}");
        msgBox.exec();
    }
}

void MainWindow::onRemoveWidget()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QFormLayout* layout = ButtonToLayoutMap.value(button);
    QHBoxLayout* Hlayout = ButtonToHLayoutMap.value(button);

    while(layout->count() != 0 )
    {
        QLayoutItem* item = layout->takeAt(0);
        delete item->widget();
        delete item;
    }
    device_counter--;
    if(Hlayout->objectName() == "horizontalLayout_1")
    {
        frame_1_counter--;
    }
    else if(Hlayout->objectName() == "horizontalLayout_2")
    {
        frame_2_counter--;
    }
    else if(Hlayout->objectName() == "horizontalLayout_3")
    {
        frame_3_counter--;
    }

}

void MainWindow::on_actionFullscreen_triggered()
{
    if(toggle_fullscreen)
    {
        this->showNormal();
        toggle_fullscreen = false;
    }
    else
    {
        this->setWindowState(Qt::WindowFullScreen);
        toggle_fullscreen = true;
    }
}


void MainWindow::on_actionminimize_triggered()
{
    this->setWindowState(Qt::WindowMinimized);
    toggle_fullscreen = false;
}

void MainWindow::on_actionexit_triggered()
{
    close();
}

