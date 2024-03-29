#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QPushButton>
#include <QFormLayout>
#include <QMqttClient>
#include <QLabel>
#include <QTimer>
#include <QMouseEvent>
#include <QtWidgets>
#include <QSqlDatabase>
#include <QtSql>
#include <QFile>
#include <QSlider>
#include <QFrame>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

typedef struct
{
    QLabel* sensor_label;
    QProgressBar* sensor_bar;
}QSensor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void onAddWidget();
    void onRemoveWidget();
    void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);
    void HandleNewMessage(QString device, const QByteArray &message, QSqlQuery &qry);
    void WeatherStation(QFormLayout* layout, QHBoxLayout* Hlayout);
    void ac_control(QFormLayout* layout);
    void enable_deepsleep(QFormLayout* layout);
    void AddToLayout(QSensor* dhth, QSensor* dhtt, QSensor* bmpt, QSensor* bmpp, QFormLayout* layout, QHBoxLayout* Hlayout);
    QLabel* status = new QLabel;
    QSqlDatabase db=QSqlDatabase::addDatabase("QMYSQL");


private slots:

    void on_actionFullscreen_triggered();
    void on_actionMinimize_triggered();
    void on_actionExit_triggered();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void UpdateStatusBar();
    void pop_message(QString title, QString message);

private:
    Ui::MainWindow *ui;
    QHash<QPushButton*, QFormLayout*> LayoutHash;
    QHash<QPushButton*, QHBoxLayout*> HLayoutHash;
    QHash<QPushButton*, QString> TopicHash;
    QHash<QString, QString> RoomHash;
    QMqttClient *m_client;
};
#endif // MAINWINDOW_H
