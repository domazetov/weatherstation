#include "mainwindow.h"
#include "ui_mainwindow.h"

#define MAX_DEVICES_PER_ROW     3
#define MAX_DEVICES             9
#define RECONNECT_DELAY_TIME    5

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

    m_client = new QMqttClient(this);
    m_client->setHostname("localhost");
    m_client->setPort(1883);
    m_client->connectToHost();

    db.setHostName("localhost");
    db.setUserName("zero");
    db.setPassword("12345678");
    db.setDatabaseName("wstation");

    QObject::connect(ui->add_device, &QPushButton::clicked, this, &MainWindow::onAddWidget);
    connect(m_client, &QMqttClient::messageReceived, this, &MainWindow::onMessageReceived);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(UpdateStatusBar()));
    timer->start(1000);
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

float str2float(const char* payload)
{
    union floatAsBytes{
        float value;
        unsigned char bytes[4];
    }data;

    for(int i = 0; i < 4; i++)
    {
        sscanf(payload+6-2*i, "%2hhx", &data.bytes[i]);
    }

    return data.value;
}

void fill_progressbar(QProgressBar* bar, QString unit, float value)
{
    if((bar->minimum() <= value) && (bar->maximum() >= value))
    {
        bar->setValue(value);
        bar->setFormat(QString::number(value) + unit);
    }
    else
    {
        bar->setValue(bar->minimum());
        bar->setFormat("Error, bad value");
    }
}

void MainWindow::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    float value = 0;
    bool ok=db.open();
    if(!ok)
    {
        pop_message("Critical", "Unable to open database");
    }

    QSqlQuery qry;
    QString device = topic.name();
    QDateTime dateTime = dateTime.currentDateTime();
    device.chop(5); // remove /data from name

    qry.prepare("INSERT INTO test ("
                "room,"
                "device,"
                "dhtt,"
                "dhth,"
                "bmpt,"
                "bmpp,"
                "timestamp)"
                "VALUES (:room, :device, :dhtt, :dhth, :bmpt, :bmpp, :timestamp);");

    Q_FOREACH(QProgressBar* bar, findChildren<QProgressBar*>())
    {
        if(bar->objectName()==(device + "/DHTH"))
        {
            value = str2float(message.data());
            fill_progressbar(bar, "%", value);
            qry.bindValue(":dhth", value);
        }
        else if(bar->objectName()==(device + "/DHTT"))
        {
            value = str2float(message.data()+8);
            fill_progressbar(bar, "°C", value);
            qry.bindValue(":dhtt", value);
        }
        else if(bar->objectName()==(device + "/BMPT"))
        {
            value = str2float(message.data()+16);
            fill_progressbar(bar, "°C", value);
            qry.bindValue(":bmpt", value);
        }
        else if(bar->objectName()==(device + "/BMPP"))
        {
            value = str2float(message.data()+24);
            fill_progressbar(bar, "hPA", value);
            qry.bindValue(":bmpp", value);
        }
    }
    Q_FOREACH(QHBoxLayout* hbox, findChildren<QHBoxLayout*>())
    {
        if((hbox->objectName()==(device + "/HBOX")))
        {
            QCheckBox *check = qobject_cast<QCheckBox*>(hbox->itemAt(0)->widget());
            if(check->isChecked())
            {
                QSlider *min = qobject_cast<QSlider*>(hbox->itemAt(2)->widget());
                QSlider *max = qobject_cast<QSlider*>(hbox->itemAt(4)->widget());
                QString minhex = QString("%1").arg(min->value(), 2, 16, QLatin1Char('0'));
                QString maxhex = QString("%1").arg(max->value(), 2, 16, QLatin1Char('0'));

                m_client->publish(device + "/ack", minhex.toUtf8()+maxhex.toUtf8());
            }
            else
            {
                m_client->publish(device + "/ack", "FFFF");
            }
        }
    }

    qry.bindValue(":device", device);
    qry.bindValue(":room", RoomHash.value(device));
    qry.bindValue(":timestamp", dateTime.toString("yyyy-MM-dd hh:mm:ss"));

    if(!qry.exec())
    {
        pop_message("Critical", "Unable to write data to database");
    }
    db.close();
}

void MainWindow::pop_message(QString title, QString message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    if("Warning" == title)
    {
        msgBox.setIcon(QMessageBox::Warning);
    }
    else if ("Critical" == title)
    {
        msgBox.setIcon(QMessageBox::Critical);
    }
    else
    {
        msgBox.setIcon(QMessageBox::Information);
    }
    msgBox.setStyleSheet("QLabel {background-color: #3a3a3a; border-width: 0;}");
    msgBox.exec();
}

void MainWindow::UpdateStatusBar()
{
    static int reconnected_count = 0;
    if(QMqttClient::Connected == m_client->state())
    {
        status->setText(tr("Client connected! Device count: %1").arg(device_counter));
    }
    else
    {
        status->setText(tr("Client disconnected. Device count: %1").arg(device_counter));
        if(RECONNECT_DELAY_TIME == reconnected_count)
        {
            m_client->connectToHost();
            reconnected_count = 0;
        }
        else
        {
            reconnected_count++;
        }
    }
    ui->statusBar->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    ui->statusBar->addPermanentWidget(status);
}

QSensor dht_humidity()
{
    QSensor dht_humi;

    dht_humi.sensor_label = new QLabel("Humidity");
    dht_humi.sensor_label->setStyleSheet("QLabel {min-width: 120px;}");
    dht_humi.sensor_bar = new QProgressBar();
    dht_humi.sensor_bar->setFormat("Waiting for update");
    dht_humi.sensor_bar->setMinimum(0);
    dht_humi.sensor_bar->setMaximum(100);
    dht_humi.sensor_bar->setValue(0);
    dht_humi.sensor_bar->setStyleSheet("QProgressBar::chunk { background-color: #146acc; }");

    return dht_humi;
}

QSensor dht_temperature()
{
    QSensor dht_temp;

    dht_temp.sensor_label = new QLabel("Temperature 1:");
    dht_temp.sensor_label->setStyleSheet("QLabel {min-width: 120px;}");
    dht_temp.sensor_bar = new QProgressBar();
    dht_temp.sensor_bar->setFormat("Waiting for update");
    dht_temp.sensor_bar->setMinimum(0);
    dht_temp.sensor_bar->setMaximum(50);
    dht_temp.sensor_bar->setValue(0);
    dht_temp.sensor_bar->setStyleSheet("QProgressBar::chunk { background-color: #b87b00 }");

    return dht_temp;
}

QSensor bmp_temperature()
{
    QSensor bmp_temp;

    bmp_temp.sensor_label = new QLabel("Temperature 2:");
    bmp_temp.sensor_label->setStyleSheet("QLabel {min-width: 120px;}");
    bmp_temp.sensor_bar = new QProgressBar();
    bmp_temp.sensor_bar->setFormat("Waiting for update");
    bmp_temp.sensor_bar->setMinimum(0);
    bmp_temp.sensor_bar->setMaximum(50);
    bmp_temp.sensor_bar->setValue(0);
    bmp_temp.sensor_bar->setStyleSheet("QProgressBar::chunk { background-color: #b82e00 }");

    return bmp_temp;
}

QSensor bmp_pressure()
{
    QSensor bmp_pres;

    bmp_pres.sensor_label = new QLabel("Pressure:");
    bmp_pres.sensor_label->setStyleSheet("QLabel {min-width: 120px;}");
    bmp_pres.sensor_bar = new QProgressBar();
    bmp_pres.sensor_bar->setFormat("Waiting for update");
    bmp_pres.sensor_bar->setMinimum(950);
    bmp_pres.sensor_bar->setMaximum(1050);
    bmp_pres.sensor_bar->setValue(950);
    bmp_pres.sensor_bar->setStyleSheet("QProgressBar::chunk { background-color: #5600b8 }");

    return bmp_pres;
}

void MainWindow::ac_control(QFormLayout* layout)
{
    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->setObjectName(ui->device_id->toPlainText() + "/HBOX");
    QCheckBox* control_ac = new QCheckBox();
    control_ac->setText("AC Control: ");
    control_ac->setLayoutDirection(Qt::RightToLeft);
    control_ac->setObjectName("control_ac");
    QSlider* ac_min = new QSlider(Qt::Horizontal);
    ac_min->setMinimum(0);
    ac_min->setMaximum(50);
    QSlider* ac_max = new QSlider(Qt::Horizontal);
    ac_max->setMinimum(0);
    ac_max->setMaximum(50);
    QLabel* ac_min_value = new QLabel("Minimum: 0°C");
    ac_min_value->setMinimumWidth(80);

    QObject::connect(ac_min, &QSlider::valueChanged, ac_max, [=] () {
        ac_min_value->setText("Minimum: " + QString::number(ac_min->value()) + "°C");
        if(ac_max->value() < ac_min->value())
        {
            ac_max->setValue(ac_min->value());
        }
    });

    QLabel* ac_max_value = new QLabel("Maximum: 0°C");
    ac_max_value->setMinimumWidth(80);

    QObject::connect(ac_max, &QSlider::valueChanged, ac_min, [=] () {
        if(ac_max->value() < ac_min->value())
        {
            ac_max->setValue(ac_min->value());
        }
        ac_max_value->setText("Maximum: " + QString::number(ac_max->value()) + "°C");
    });

    hbox->addWidget(control_ac);
    hbox->addWidget(ac_min_value);
    hbox->addWidget(ac_min);
    hbox->addWidget(ac_max_value);
    hbox->addWidget(ac_max);

    layout->addRow(hbox);
}


void MainWindow::WeatherStation(QFormLayout* layout, QHBoxLayout* Hlayout)
{
    auto subscription = m_client->subscribe(ui->device_id->toPlainText() + "/data");
    if (!subscription)
    {
        pop_message("Error", "Could not subscribe. Is there a valid connection?");
        return;
    }

    QPushButton* device_name;

    if(ui->device_name->toPlainText().isEmpty())
    {
        device_name = new QPushButton(ui->device_id->toPlainText());
    }
    else
    {
        device_name = new QPushButton(ui->device_name->toPlainText());
    }

    QSensor dht_humi = dht_humidity();
    QSensor dht_temp = dht_temperature();
    QSensor bmp_temp = bmp_temperature();
    QSensor bmp_pres = bmp_pressure();

    dht_humi.sensor_bar->setObjectName(ui->device_id->toPlainText() + "/DHTH");
    dht_temp.sensor_bar->setObjectName(ui->device_id->toPlainText() + "/DHTT");
    bmp_temp.sensor_bar->setObjectName(ui->device_id->toPlainText() + "/BMPT");
    bmp_pres.sensor_bar->setObjectName(ui->device_id->toPlainText() + "/BMPP");

    layout->addRow(device_name);
    layout->addRow(dht_humi.sensor_label, dht_humi.sensor_bar);
    layout->addRow(dht_temp.sensor_label, dht_temp.sensor_bar);
    layout->addRow(bmp_temp.sensor_label, bmp_temp.sensor_bar);
    layout->addRow(bmp_pres.sensor_label, bmp_pres.sensor_bar);

    ac_control(layout);

    RoomHash.insert(ui->device_id->toPlainText(), device_name->text());
    LayoutHash.insert(device_name, layout);
    HLayoutHash.insert(device_name, Hlayout);
    TopicHash.insert(device_name, ui->device_id->toPlainText());

    QObject::connect(device_name, &QPushButton::clicked, this, &MainWindow::onRemoveWidget);
    device_counter++;
}

void MainWindow::onAddWidget()
{
    if(ui->device_id->toPlainText().isEmpty())
    {
        pop_message("Warning", "Device ID is required");
        return;
    }

    foreach(QPushButton* i, TopicHash.keys())
    {
        if(TopicHash[i] == ui->device_id->toPlainText())
        {
            pop_message("Warning", "Device already added!");
            return;
        }
    }

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
        pop_message("Warning", "Device count reached maximum!");
    }
}

void deleteWidget(QLayout* layout)
{
    while(layout->count() != 0 )
    {
        QLayoutItem* item = layout->takeAt(0);
        if(QWidget* widget = item->widget())
        {
            widget->deleteLater();
        }
        if(QLayout* childLayout = item->layout())
        {
            deleteWidget(childLayout); //Looks dangerous, I know...
        }
        delete item;
    }
}

void MainWindow::onRemoveWidget()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    QFormLayout* layout = LayoutHash.value(button);
    QHBoxLayout* Hlayout = HLayoutHash.value(button);
    QString topic = TopicHash.value(button);

    m_client->unsubscribe(topic + "/#");
    TopicHash.remove(button);

    deleteWidget(layout);

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


void MainWindow::on_actionMinimize_triggered()
{
    this->setWindowState(Qt::WindowMinimized);
    toggle_fullscreen = false;
}

void MainWindow::on_actionExit_triggered()
{
    close();
}
