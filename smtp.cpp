#include "smtp.h"

Smtp::Smtp(const QString &user, const QString &pass, const QString &host, int port, int timeout)
{
    socket = new QSslSocket(this);

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorReceived(QAbstractSocket::SocketError)));
    connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(stateChanged(QAbstractSocket::SocketState)));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));

    this->user = user;
    this->pass = pass;
    this->host = host;
    this->port = port;
    this->timeout = timeout;
}

void Smtp::sendMail(const QString &from, const QString &to, const QString &subject, const QString &body)
{
    message = "To: " + to + "\n";
    message.append("From: " + from + "\n");
    message.append("Subject: " + subject + "\n");
    message.append("Mime-Version: 1.0\n");
    message.append("Content-Type: text/plain; charset=\"utf-8\"\n");
    message.append("Content-Transfer-Encoding: 8bit\n\n");
    message.append(body);
    message.replace(QString::fromLatin1("\n"), QString::fromLatin1("\r\n"));
    message.replace(QString::fromLatin1("\r\n.\r\n"), QString::fromLatin1("\r\n..\r\n"));
    this->from = from;
    rcpt = to;
    state = Init;

    // Use ConnectToHostEncrypted for implicit SSL (port 465)
    socket->connectToHostEncrypted(host, port); 
    if (!socket->waitForConnected(timeout)) {
        qDebug() << socket->errorString();
    }
    t = new QTextStream(socket);
}

Smtp::~Smtp()
{
    delete t;
    delete socket;
}

void Smtp::stateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "stateChanged " << socketState;
}

void Smtp::errorReceived(QAbstractSocket::SocketError socketError)
{
    qDebug() << "error " << socketError;
    emit status("Error: " + socket->errorString());
}

void Smtp::disconnected()
{
    qDebug() << "disconneted";
    qDebug() << "error " << socket->errorString();
}

void Smtp::connected()
{
    qDebug() << "Connected ";
}

void Smtp::readyRead()
{
    qDebug() << "readyRead";
    QString responseLine;
    do
    {
        responseLine = socket->readLine();
        response += responseLine;
    }
    while (socket->canReadLine() && responseLine[3] != ' ');

    responseLine.truncate(3);

    qDebug() << "Server response code:" << responseLine;
    qDebug() << "Server response: " << response;

    if (state == Init && responseLine == "220")
    {
        *t << "EHLO localhost" << "\r\n";
        t->flush();
        state = Auth;
    }
    else if (state == Auth && responseLine == "250")
    {
        socket->startClientEncryption();
        if (!socket->waitForEncrypted(timeout)) {
            qDebug() << socket->errorString();
            state = Close;
        }
        *t << "AUTH LOGIN" << "\r\n";
        t->flush();
        state = User;
    }
    else if (state == User && responseLine == "334")
    {
        *t << QByteArray().append(user.toUtf8()).toBase64() << "\r\n";
        t->flush();
        state = Pass;
    }
    else if (state == Pass && responseLine == "334")
    {
        *t << QByteArray().append(pass.toUtf8()).toBase64() << "\r\n";
        t->flush();
        state = Mail;
    }
    else if (state == Mail && responseLine == "235")
    {
        *t << "MAIL FROM:<" << from << ">" << "\r\n";
        t->flush();
        state = Rcpt;
    }
    else if (state == Rcpt && responseLine == "250")
    {
        *t << "RCPT TO:<" << rcpt << ">" << "\r\n"; 
        t->flush();
        state = Data;
    }
    else if (state == Data && responseLine == "250")
    {
        *t << "DATA\r\n";
        t->flush();
        state = Body;
    }
    else if (state == Body && responseLine == "354")
    {
        *t << message << "\r\n.\r\n";
        t->flush();
        state = Quit;
    }
    else if (state == Quit && responseLine == "250")
    {
        *t << "QUIT\r\n";
        t->flush();
        state = Close;
        emit status("Message sent");
        QMessageBox::information(nullptr, "Information", "Email d'alerte envoyé avec succès!");
    }
    else if (state == Close)
    {
        return;
    }
    else
    {
        if (responseLine == "") { return; }
        // Something went wrong
        QMessageBox::warning(nullptr, "Erreur SMTP", "Une erreur inattendue est survenue: " + responseLine + "\n" + response);
        state = Close;
        emit status("Failed to send message: " + response);
    }
    response = "";
}
