#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QFormLayout>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void onAddWidget();
    void onRemoveWidget();
    void WeatherStation(QFormLayout* layout, QHBoxLayout* Hlayout);

private slots:

    void on_actionFullscreen_triggered();
    void on_actionminimize_triggered();
    void on_actionexit_triggered();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::MainWindow *ui;
    QHash<QPushButton*, QFormLayout*> ButtonToLayoutMap;
    QHash<QPushButton*, QHBoxLayout*> ButtonToHLayoutMap;

};
#endif // MAINWINDOW_H
