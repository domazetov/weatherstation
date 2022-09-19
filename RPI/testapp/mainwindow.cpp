#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "string.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_client = new QMqttClient(this);
    m_client->setHostname("localhost");
    m_client->setPort(1883);
    m_client->connectToHost();
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    char data_buffer[50];

    float dhth = ui->doubleSpinBox_4->value();
    float dhtt = ui->doubleSpinBox_3->value();
    float bmpt = ui->doubleSpinBox->value();
    float bmpp = ui->doubleSpinBox_2->value();

    snprintf(data_buffer, 33, "%08X%08X%08X%08X",	*(uint32_t*)&dhth, *(uint32_t*)&dhtt, *(uint32_t*)&bmpt, *(uint32_t*)&bmpp);
    ui->label_5->setText(data_buffer);
    m_client->publish(ui->lineEdit->text()+"/data", data_buffer);
}


void MainWindow::on_doubleSpinBox_4_valueChanged(double arg1)
{
    float test = arg1;
    char value[10];
    snprintf(value, 9, "%08X", *(uint32_t*)&test);
    ui->label_6->setText(value);
}


void MainWindow::on_doubleSpinBox_3_valueChanged(double arg1)
{

    float test = arg1;
    char value[10];
    snprintf(value, 9, "%08X", *(uint32_t*)&test);
    ui->label_7->setText(value);
}


void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
    float test = arg1;
    char value[10];
    snprintf(value, 9, "%08X", *(uint32_t*)&test);
    ui->label_8->setText(value);
}


void MainWindow::on_doubleSpinBox_2_valueChanged(double arg1)
{
    float test = arg1;
    char value[10];
    snprintf(value, 9, "%08X", *(uint32_t*)&test);
    ui->label_9->setText(value);
}

