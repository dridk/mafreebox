#ifndef MAFREEBOX_H
#define MAFREEBOX_H

#include <QObject>
#include <QtNetwork>
#include "filesystemapi.h"
#include "downloadapi.h"
#include "callapi.h"
#include "contactapi.h"
#include "connectionapi.h"

class FileSystemApi;
class DownloadApi;
class CallApi;
class ContactApi;
class ConnectionApi;

struct ApiInfo;
struct ApiInfo
{
    QString deviceName;
    QString version;
    QString baseUrl;
    QString deviceType;

};
class FbxAPI : public QNetworkAccessManager
{
    Q_OBJECT
public:

    enum AuthStatus {
        UnknownStatus = 0,
        PendingStatus = 1 ,
        TimeOutStatus = 2 ,
        GrantedStatus = 3,
        DeniedStatus  = 4
    };

    explicit FbxAPI(QObject *parent = 0);
    ~FbxAPI();

    bool logged(){return mLogged;}
    //set
    void setHostName(const QString& host, int port = 80) ;
    void setApplicationToken(const QString& token);
    void setApplicationName(const QString& name);
    void setApplicationVersion(const QString& version);
    void setApplicationId(const QString& id);
    void setBaseUrl(const QString& base);

    //get
    const QString& hostName() const;
    int port() const;
    const QString& applicationToken() const;
    const QString& applicationId() const;
    const QString& applicationName() const;
    const QString& applicationVersion() const;
    const QString& sessionToken() const;
    const QString& baseUrl() const;
    const QString& challenge() const;
    const QString& deviceName() const;
    const QString& deviceType() const;
    const QString& errorString() const;
    const QString& errorCode() const;
    const QStringList& permissions() const;

    QNetworkRequest myCreateRequest(const QString& uri) const;
    bool parseResult(const QJsonDocument& doc);
    void sendError(const QString& message, const QString& code);

    //request
    void requestApiInfo();
    void requestAuthorize();
    void requestAuthorizeStatus(int trackId);
    void requestLogin();
    void requestSession();

    void logout();

    //get Module
    FileSystemApi * fileSystem() {return mFileSystem;}
    DownloadApi * download() {return mDownload;}
    CallApi * call() {return mCall;}
    ContactApi * contact() {return mContact;}
    ConnectionApi * connection(){return mConnection;}


signals:
    void error(const QString& message, const QString& errorCode);
    void authorizeReceived(const QString& applicationToken, int trackId);
    void authorizeStatusChanged(const FbxAPI::AuthStatus& status);

    void apiInfoChanged();
    void challengeChanged();
    void sessionReceived();
    void loginSuccess();
    void logoutSuccess();

private slots:
    void requestApiInfoFinished();
    void requestAuthorizeFinished();
    void requestAuthorizeStatusFinished();
    void requestLoginFinished();
    void requestSessionFinished();
    void errorReceived(QNetworkReply::NetworkError errCode);

protected:
    static QString hmacSha1(QByteArray key, QByteArray baseString);

private:
    ApiInfo mApiInfo;
    QString mApplicationToken;
    QString mSessionToken;
    QString mApplicationName;
    QString mApplicationId;
    QString mApplicationVersion;
    QString mChallenge;
    QString mHostName;
    QString mErrorString;
    QString mErrorCode;
    QStringList mPermissions;
    bool mLogged;
    int mPort;
    int mRequestLoginAttempt;

    //Modules
   FileSystemApi * mFileSystem;
   DownloadApi * mDownload;
   CallApi * mCall;
   ContactApi * mContact;
   ConnectionApi * mConnection;

    
};







#endif // MAFREEBOX_H
