#include "QNAMThread.h"
#include <QNetworkReply>

qNamThread::qNamThread(){
    networkManager=SharedManager(new QNetworkAccessManager);
    multiPart = SharedMultiPart(new QHttpMultiPart(QHttpMultiPart::FormDataType));
    QObject::connect(networkManager.get(), &QNetworkAccessManager::finished,this,&qNamThread::LoadReply);
}

QString qNamThread::getError() const
{
    return Error;
}

QString qNamThread::getResult() const
{
    return result;
}

void qNamThread::setOptions(const ListKeys &value)
{
    Options = value;
}

void qNamThread::emitCancel()
{

}

void qNamThread::LoadReply(QNetworkReply *reply)
{
    result=reply->readAll();
    //    QNAM_MSG(result);
    //    QNAM_MSG("result"<<result);
    if(reply->error()){
        QNAM_MSG(reply->error()<<"Error"<<reply->errorString());
    }

}

void qNamThread::setHeaders(const ListKeys &value)
{
    Headers = value;
}

bool qNamThread::DoHttpRequest(QString Url, const QString &RequestType)
{
    url=Url;
    if(RequestType==POST_Key)
        method=QNAM_POST;
    else if(RequestType==GET_Key)
        method=QNAM_GET;
    else if(RequestType==DELETE_Key)
        method=QNAM_DELETE;
    else if(RequestType==PUT_Key)
        method=QNAM_PUT;
    else if(RequestType==VIEW_Key)
        method=QNAM_VIEW;
    return WaitforFinish();
}

bool qNamThread::DownloadFile(const QString &Url, const QString &Output)
{
    method=QNAM_DOWNLOADER;
    url=Url;
    output=Output;
    return WaitforFinish();
}

bool qNamThread::UploadFile(QString Url, const QString &file)
{
    method=QNAM_UPLOADER;
    url=Url;
    output=file;
    return WaitforFinish();
}

bool qNamThread::WaitforFinish()
{
    DoJob();
    //    while(isFinished()==false){
    //        QApplication::processEvents();
    //        //        if(inDownloading){
    //        //            emit Update((double(totaldownloaded)/totalSize)*100);
    //        //        }
    //    }
    return Error.isEmpty();
}

void qNamThread::DoJob()
{
    QNAM_MSG("qNamThread");
    retry--;
    Error.clear();
    //    custom request with custom header and without
    {
        if(method == QNAM_DOWNLOADER or method==QNAM_UPLOADER){
            switch (method) {
            case QNAM_DOWNLOADER:
                Downloader(url);
                break;
            case QNAM_UPLOADER:
                Uploader(output);
                break;
            }
        }else{
            QString type;
            if(method==QNAM_GET)
                type=GET_Key;
            else if(method==QNAM_DELETE)
                type=DELETE_Key;
            else if(method==QNAM_POST)
                type=POST_Key;
            else if(method==QNAM_PUT)
                type=PUT_Key;
            else if(method==QNAM_VIEW)
                type=VIEW_Key;
            QNAM_MSG("type"<<type);
            UploadRequestMulti(type);
        }
    }
    if(result.isEmpty() and retry>=0){
        QNAM_MSG(retry<<"Retry"<<result);
        networkManager->clearConnectionCache();
        networkManager->clearAccessCache();
        QThread::msleep(500);
        DoJob();
    }
}

bool qNamThread::Downloader(const QString &Url)
{
    QObject::disconnect(networkManager.get(), &QNetworkAccessManager::finished,this,&qNamThread::LoadReply);
    QObject::connect(networkManager.get(), &QNetworkAccessManager::finished,this,&qNamThread::downloadFinished);
    QUrl ur=QUrl(Url);
    QNetworkRequest request(ur);
    request.setRawHeader("User-Agent", "");
    for( int i=0;i<Headers.count();i++){
        JsKeys hkey=Headers.at(i);
        request.setRawHeader(hkey.Key.toLatin1(),hkey.Value.toLatin1());
        QNAM_MSG(hkey.Key.toLatin1()<<hkey.Value.toLatin1());
    }
    QNetworkReply *reply=networkManager->sendCustomRequest(request,GET_Key);
    QObject::connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(ProgressProgress(qint64,qint64)));
    EventLoop(reply);
    QThread::msleep(50);
    return true;
}

bool qNamThread::UploadRequestMulti(const QString &RequestType)
{
    QNAM_MSG(url<<RequestType<<Headers.count()<<Options.count());
    QUrl ur=QUrl(url);
    QNetworkRequest request(ur);
    request.setRawHeader("User-Agent", "");
    for( int i=0;i<Headers.count();i++){
        JsKeys hkey=Headers.at(i);
        request.setRawHeader(hkey.Key.toLatin1(),hkey.Value.toLatin1());
        QNAM_MSG(hkey.Key.toLatin1()<<hkey.Value.toLatin1());
    }
    QThread::msleep(30);
    for(int i=0;i<Options.count();i++){
        JsKeys ckey=Options.at(i);
        QHttpPart dataPart;
        dataPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));
        dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=%1;").arg(ckey.Key)));
        dataPart.setBody(ckey.Value.toLatin1());
        multiPart->append(dataPart);
        QNAM_MSG(ckey.Key.toLatin1()<<ckey.Value.toLatin1());
    }
    EventLoop(networkManager->sendCustomRequest(request,RequestType.toLatin1(), multiPart.get()));
    QThread::msleep(50);
    return true;
}

bool qNamThread::Uploader(const QString &File)
{
    QNAM_MSG("Uploader"<<url);
    QUrl ur=QUrl(url);
    QNetworkRequest request(ur);
    request.setRawHeader("User-Agent", "HEEE");
    for( int i=0;i<Headers.count();i++){
        JsKeys hkey=Headers.at(i);
        request.setRawHeader(hkey.Key.toLatin1(),hkey.Value.toLatin1());
        QNAM_MSG(hkey.Key.toLatin1()<<hkey.Value.toLatin1());
    }
    QHttpPart file_Part;
    file_Part.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\"file\""));/* version.tkt is the name on my Disk of the file that I want to upload */

    QFile *file = new QFile(File);
    file->open(QIODevice::ReadOnly);
    file_Part.setBodyDevice(file);
    file->setParent(multiPart.get()); // we cannot delete the file now, so delete it with the multiPart

    multiPart->append(file_Part);

    QNetworkReply *reply = networkManager->post(request, multiPart.get());
    QObject::connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(ProgressProgress(qint64,qint64)));
    multiPart->setParent(reply); // delete the multiPart with the reply
    EventLoop(reply);
    QThread::msleep(50);
    return true;
}

void qNamThread::ProgressProgress(qint64 recieved, qint64 total) {
    QNAM_MSG(recieved << total);
    emit Update(PercentageValue(recieved,total));
    if(recieved==total){
        result="Success";
    }
}

void qNamThread::downloadFinished(QNetworkReply *data) {
    QNAM_MSG("downloadFinished");
    QFile localFile(output);
    if (!localFile.open(QIODevice::WriteOnly)){
        Error=localFile.errorString();
        return;
    }
    const QByteArray sdata = data->readAll();
    if(sdata.isEmpty()){
        Error="Fail load data";
    }
    localFile.write(sdata);
    //    qDebug() << "sdata"<<sdata.length();
    localFile.close();
}

QString qNamThread::timeConversion(int msecs)
{
    QString formattedTime;

    int hours = msecs/(1000*60*60);
    int minutes = (msecs-(hours*1000*60*60))/(1000*60);
    int seconds = (msecs-(minutes*1000*60)-(hours*1000*60*60))/1000;
    int milliseconds = msecs-(seconds*1000)-(minutes*1000*60)-(hours*1000*60*60);

    formattedTime.append(QString("%1").arg(hours, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(minutes, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(seconds, 2, 10, QLatin1Char('0')) + ":" +
                         QString( "%1" ).arg(milliseconds, 3, 10, QLatin1Char('0')));

    return formattedTime;
}

void qNamThread::sleep( int millisecondsToWait )
{
    QTime dieTime = QTime::currentTime().addMSecs( millisecondsToWait );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

bool qNamThread::PureHttpRequest(const QString &Url, const QString &RequestType, int MaxRetry)
{
    QNAM_MSG(Url<<RequestType<<MaxRetry);
    if(RequestType.isEmpty()){
        qDebug() <<"Error Type Empty!";
        return 0;
    }
    QUrl ur=QUrl(Url);
    QNetworkRequest request(ur);
    QNetworkReply *reply=networkManager->sendCustomRequest(request,RequestType.toLatin1());
    EventLoop(reply);
    QThread::msleep(50);
    if(reply->error()==QNetworkReply::HostNotFoundError or reply->error()==QNetworkReply::InternalServerError){
        for(int i=0;i<MaxRetry;i++){
            QNAM_MSG("try"<<i);
            reply=networkManager->sendCustomRequest(request,RequestType.toLatin1());
            EventLoop(reply);
            sleep(1000);
            if(reply->error()!=QNetworkReply::HostNotFoundError)
            {
                break;
            }
        }
    }
    QNAM_MSG(reply->error()<<result);
    return !reply->error();
}

bool qNamThread::QuickHttpRequest(const QString &Url, const QString &RequestType, QString &responce, int MaxRetry)
{
    qNamThread qnam;
    qnam.PureHttpRequest(Url,RequestType,MaxRetry);
    responce=qnam.getResult();
    if(responce.isEmpty())
        responce=qnam.getError();
    return !responce.isEmpty();
}
