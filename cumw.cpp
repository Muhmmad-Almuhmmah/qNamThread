#include "cumw.h"
#include "ui_cumw.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    cumw w;
    w.show();
    return a.exec();
}

cumw::cumw(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::cumw)
{
    ui->setupUi(this);
}

cumw::~cumw()
{
    delete ui;
}

void cumw::on_pushButton_clicked()
{
    QElapsedTimer timer;
    timer.start();
    qNamThread clt;
    connect(&clt,SIGNAL(Update(int)),ui->progressBar,SLOT(setValue(int)));
    connect(&clt,SIGNAL(UpdateWait(QString,int)),this,SLOT(WaitMessage(QString,int)));
    connect(ui->pushButton_2,SIGNAL(pressed()),&clt,SLOT(emitCancel()));
    bool state=clt.DownloadFile("https://mirror2.internetdownloadmanager.com/idman638build22.exe?b=1&filename=idman638build22.exe","IDM_TRAIL.EXE");
    qDebug() <<"state"<<state<<clt.timeConversion(timer.elapsed());
}
