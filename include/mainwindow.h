#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include "chatmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onPeerListUpdated();
    void onMessageReceived(const QString& senderName, const QString& text, const QDateTime& timestamp);
    void on_sendButton_clicked();
    void on_saveUsernameButton_clicked();
    void on_visibilityCheckBox_stateChanged(int state);
    void onPeerSelected();

private:
    void loadChatHistory();
    void appendToChatLog(const QString& sender, const QString& message, const QDateTime& timestamp, bool saveToFile = true);
    void setupConnections();

    Ui::MainWindow *ui;
    ChatManager *m_chatManager;
    QString m_selectedPeerId;
    QString m_historyFilePath;
};

#endif // MAINWINDOW_H
