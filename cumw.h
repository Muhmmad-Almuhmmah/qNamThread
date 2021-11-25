#ifndef CUMW_H
#define CUMW_H

#include <QMainWindow>
#include <QNAMThread.h>
QT_BEGIN_NAMESPACE
namespace Ui { class cumw; }
QT_END_NAMESPACE
#include <QMessageBox>
class cumw : public QMainWindow
{
    Q_OBJECT
public:
    cumw(QWidget *parent = nullptr);
    ~cumw();

private slots:
    void on_pushButton_clicked();
private:
    Ui::cumw *ui;
};
#endif // CUMW_H
