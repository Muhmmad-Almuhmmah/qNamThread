#ifndef QNAMTHREAD_H
#define QNAMTHREAD_H
// qnetworkaccessmanager threading :)
#include <QObject>
#include <QThread>
#include <QApplication>
#include <stdio.h>
#include <string>
#include <QString>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QSharedPointer>
#include <QHttpPart>
#include <QFile>
#include <QElapsedTimer>

typedef QSharedPointer<QHttpMultiPart> SharedMultiPart;
typedef QSharedPointer<QNetworkAccessManager> SharedManager;

//http Request

#define POST_Key    "POST"
#define GET_Key     "GET"
#define OPTIONS_Key "OPTIONS"
#define VIEW_Key    "VIEW"
#define PUT_Key     "PUT"
#define DELETE_Key  "DELETE"

#define \
    EventLoop(networkManager){\
    QEventLoop loop;\
    connect(networkManager, SIGNAL(finished()), &loop, SLOT(quit()));\
    loop.exec();\
    }

class JsKeys{
public:
    JsKeys(QString sKey,QString sValue){
        Key=sKey;
        Value=sValue;
        //qDebug() <<"JS_KEY"<<Key<<Value;
    }
    QString Key;
    QString Value;
};

typedef QList<JsKeys> ListKeys;


#ifdef QNAM_DEBUG
#define QNAM_MSG(x) qDebug().noquote() <<__FUNCTION__<<":"<<x
#else
#define QNAM_MSG(x)
#endif

class qNamThread:public QObject
{
    Q_OBJECT
    enum{
        QNAM_DOWNLOADER=1,
        QNAM_UPLOADER,
        QNAM_GET,
        QNAM_POST,
        QNAM_DELETE,
        QNAM_PUT,
        QNAM_VIEW
    };
    QString url,output,Error,result;
    int method=0,retry=3;
    ListKeys Options,Headers;
    SharedManager networkManager;
    SharedMultiPart multiPart;
public:
    explicit qNamThread();
    void setOptions(const ListKeys &value);
    void setHeaders(const ListKeys &value);
    bool DoHttpRequest(QString Url, const QString &RequestType);
    bool UploadFile(QString Url, const QString &file);
    bool DownloadFile(const QString &Url,const QString &Output);
    bool WaitforFinish();
    void DoJob();
    bool Downloader(const QString &Url);
    bool UploadRequestMulti(const QString &RequestType);
    bool Uploader(const QString &File);
    void downloadFinished(QNetworkReply *data);
    static QString timeConversion(int msecs);
    QString getError() const;
    QString getResult() const;
signals:
    void Update(const int &value);
#ifdef CUSTOM_METHOD
    void UpdateWait(const QString &Message,const int &value);
#endif
public slots:
    void emitCancel();
    void LoadReply(QNetworkReply *reply);
    void downloadProgress(qint64 recieved, qint64 total);
};

#endif // QNAMTHREAD_H
