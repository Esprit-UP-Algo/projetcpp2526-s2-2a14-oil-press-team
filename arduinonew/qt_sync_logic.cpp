// --- QT SIDE: Database Polling Logic for Arduino Synchronization ---
// Put this inside your MainWindow constructor or a timer setup.

// 1. Initialize Serial
serial = new QSerialPort(this);
serial->setPortName("COM9"); // Or use auto-detection
serial->setBaudRate(QSerialPort::Baud115200);
serial->open(QIODevice::ReadWrite);

// 2. Setup Polling Timer (Every 2 seconds)
QTimer *dbPollTimer = new QTimer(this);
QObject::connect(dbPollTimer, &QTimer::timeout, [this]() {
    // Check Machine #1 Status in Database
    QSqlQuery q("SELECT ETAT_MACHINE FROM MACHINE WHERE ID_MACHINE = 1");
    if (q.next()) {
        QString state = q.value(0).toString().trimmed();
        
        if (serial && serial->isOpen()) {
            if (state == "En danger") {
                serial->write("DANGER\n");
                serial->flush();
                qDebug() << "[SERIAL] Database is in DANGER state. Sending DANGER command.";
            } else {
                serial->write("SAFE\n");
                serial->flush();
                // Optionally log only on state change to avoid spam
            }
        }
    }
});
dbPollTimer->start(2000);

// 3. Handle Incoming Alerts (Arduino -> Qt -> Database)
QObject::connect(serial, &QSerialPort::readyRead, [this]() {
    while (serial->canReadLine()) {
        QString line = serial->readLine().trimmed();
        if (line.startsWith("ALERT_GAS:")) {
            QString machineId = line.mid(10);
            qDebug() << "[SERIAL] GAS DETECTED on Machine ID:" << machineId;
            
            // Update Database to trigger intervention
            QSqlQuery update;
            update.prepare("UPDATE MACHINE SET ETAT_MACHINE = 'En danger' WHERE ID_MACHINE = :id");
            update.bindValue(":id", machineId.toInt());
            if (update.exec()) {
                qDebug() << "[DB] Machine state updated to 'En danger'.";
            }
        }
    }
});
