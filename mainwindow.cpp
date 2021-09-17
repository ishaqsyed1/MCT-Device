#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    mctClock = new MCTClock();
    clockTimer = new QTimer(this);
    connect(clockTimer, SIGNAL(timeout()), this, SLOT(on_clockTimer_activated()));
    clockTimer->start(1000);

    pulseCounter = 0;
    therapyCounter = 20*60;
    pulseTimer = new QTimer(this);
    connect(pulseTimer, SIGNAL(timeout()), this, SLOT(on_pulseTimer_activated()));

    battery = new Battery();
    batteryTimer = new QTimer(this);
    batteryPrompt = "Battery: ";
    connect(batteryTimer, SIGNAL(timeout()), this, SLOT(on_batteryTimer_activated()));
    batteryTimer->start(1000);

    ui->setupUi(this);
    setFixedSize(349, 663);

    ui->therapyWidget->hide();
    ui->medWidget->hide();
    ui->list_2->hide();
    ui->list_3->hide();
    ui->list_4->hide();
    ui->list_5->hide();
    ui->list_6->hide();
    ui->list_7->hide();

    counter = -1;
    currentWidget = ui->list;

    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(on_freq_adjusted()));
    curFreq = 200;

    // bluetooth related setup
    connect(disc, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));
    disc->start();
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
  }

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closePotentialTherapy(){
    pulseTimer->stop();
    battery->remDrain((float)curFreq * 0.002);
    ui->horizontalSlider->setEnabled(true);
}

Battery* MainWindow::getBattery(){return battery;}

void MainWindow::on_batteryTimer_activated(){
    if(battery->drain() == 0) QCoreApplication::quit();
    ui->batteryIndicator->setText(QString::fromStdString(batteryPrompt) + QString::number(battery->batteryStatus()));
}

void MainWindow::on_pulseTimer_activated(){
    if(pulseCounter > 0){
        pulseCounter --;
        currentCountdown->display(pulseCounter);
    }else if(pulseCounter == 0){
        battery->addDrain(curFreq*0.002);
        pulseCounter = -1;
    }else if(therapyCounter > 0){
        pulseCounter = -1;
        currentCountdown->display(therapyCounter);
        therapyCounter --;
        ui->label->setText("Therapy is now active");
        ui->horizontalSlider->setEnabled(false);
    }else{
        pulseCounter = 0;
        battery->remDrain(curFreq*0.002);
        therapyCounter = 0;
        pulseTimer->stop();
    }
}

void MainWindow::on_clockTimer_activated(){
    ui->clockIndicator->setText(mctClock->getTime());
}

void MainWindow::updateScreen(QWidget *w){
    history.push(currentWidget);
    currentWidget->hide();
    currentWidget = w;
    counter = -1;
    currentWidget->show();
}

void MainWindow::updateScreen(QWidget *w, int programFlag, int cdTime, int theraTime){
    updateScreen(w);
    std::string prompt;
    switch(programFlag){
        case 0: prompt = "Place electrode at upper abdomen\n and 7th cervical vertebra area. You\n have 30 seconds before therapy begins."; break;
        case 1: prompt = "Place electrode at painful area.\n You have 30 seconds before therapy begins."; break;
        case 2: prompt = "Place electrode on intensly painful area.\n You have 30 seconds before therapy begins."; break;
        case 3: prompt = "Now in MED"; break;
        case 4: prompt = "Now in SCREENING"; break;
    }
    ui->label->clear();
    ui->label->setText(QString::fromStdString(prompt));
    currentCountdown = ui->lcdNumber_2;
    pulseCounter = cdTime;
    therapyCounter = theraTime;
    currentCountdown->display(pulseCounter);
    pulseTimer->start(1000);
}

void MainWindow::on_pushButton_clicked(){

    std::string widgetType = currentWidget->metaObject()->className();
    if(widgetType.compare("QListWidget") == 0){
        QListWidgetItem *item = ((QListWidget*)currentWidget)->currentItem();
        if(ui->label_3->text().compare("SETTINGS") != 0) ui->label_3->setText(item->text()); // prevents label_3 changing to selected option if in SETTINGS
        if(item->text().compare("PROGRAMS") == 0)       updateScreen(ui->list_2);
        else if(item->text().compare("SETTINGS") == 0)  updateScreen(ui->list_7);
        else if(item->text().compare("Allergy") == 0)   updateScreen(ui->therapyWidget, 0, 30, 5*60);
        else if(item->text().compare("Pain") == 0)      updateScreen(ui->therapyWidget, 1, 30, 15*60);
        else if(item->text().compare("Int. Pain") == 0) updateScreen(ui->therapyWidget, 2, 30, 15*60);
        else if(item->text().compare("MED") == 0)       updateScreen(ui->therapyWidget, -1, 15, 5*60);
        else if(item->text().compare("SCREENING") == 0) updateScreen(ui->therapyWidget, -1, 10, 3*60);
        else if(item->text().compare("Economy") == 0){
            if(!battery->getEcoState()) batteryPrompt = "ECO " + batteryPrompt;
            else batteryPrompt = "Battery: ";
            battery->setEcoState(!battery->getEcoState());
        }
    }
}

void MainWindow::on_pushButton_3_clicked(){
    if (!(counter > ((QListWidget*)(currentWidget))->count() - 2)) counter ++;
    ((QListWidget*)(currentWidget))->setCurrentItem(((QListWidget*)(currentWidget))->item(counter));
}

void MainWindow::on_pushButton_2_clicked(){
    if (counter > 0) counter --;
    ((QListWidget*)(currentWidget))->setCurrentItem(((QListWidget*)(currentWidget))->item(counter));
}

//back button
void MainWindow::on_pushButton_7_clicked(){

    closePotentialTherapy();

    if(history.empty()) return;

    if(currentWidget == ui->therapyWidget){
        pulseTimer->stop();
        pulseCounter = 0;
        therapyCounter = 20*60;
    }
    if(history.top() == ui->list_2){
        ui->label_3->setText("PROGRAMS");
    }

    if(!history.empty()){
        currentWidget->hide();
        currentWidget = history.top();
        history.pop();
        counter = -1;
        currentWidget->show();
    }
    else if (ui->label_3->text().compare("MED") == 0){
        pulseTimer->stop();
        pulseCounter = 0;
        ui->medWidget->hide();
        ui->list->show();
    }else if (ui->label_3->text().compare("SCREENING") == 0){
        ui->medWidget->hide();
        ui->list->show();
    }

    if(history.empty()){
        ui->label_3->setText("MAIN MENU");
    }
}
//menu button
void MainWindow::on_pushButton_8_clicked()
{
    closePotentialTherapy();

    if(!history.empty()){
        currentWidget->hide();
        currentWidget = ui->list;
        history.pop();
        counter = -1;
        currentWidget->show();
    } else if (ui->label_3->text().compare("MED") == 0){
        ui->medWidget->hide();
        ui->list->show();
    }else if (ui->label_3->text().compare("SCREENING") == 0){
        ui->medWidget->hide();
        ui->list->show();
    }
}

void MainWindow::on_freq_adjusted(){
    curFreq = ui->horizontalSlider->value() * 5 + 200;
    ui->freqLbl->setText(QString::number(curFreq));
}

// bluetooth function definitions

void MainWindow::on_Find_Device_clicked(){
    ui->listWidget->show();
    ui->listWidget->clear();
    disc->stop();
    disc->start();
}

void MainWindow::on_Bluetooth_On_clicked() {socket->write("ON");}
void MainWindow::on_Bluetoot_Off_clicked() {socket->write("OFF");}

void MainWindow::deviceDiscovered(const QBluetoothDeviceInfo &device){
    ui->listWidget->addItem(device.address().toString());
    ui->listWidget->setCurrentRow(1);
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item){
    string = item->text();
    ui->listWidget->addItem(string);
    static const QString serviceUuid(QStringLiteral("00001101-0000-1000-8000-00805F9B34FB"));
    socket->connectToService(QBluetoothAddress(string), QBluetoothUuid(serviceUuid), QIODevice::ReadWrite);
}

void MainWindow::on_list_7_itemClicked(QListWidgetItem *item){
    if (item->text().compare("Economy") == 0){
        //item->setText("economyon");
            item->setText("Economy On");
            batteryPrompt = "ECO " + batteryPrompt;
    }
    else if (item->text().compare("Economy On") == 0){
        //item->setText("economyon");
            item->setText("Economy");
    }
}
