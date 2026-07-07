#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTextStream>
#include <QScrollBar>
#include <QFile>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_chatManager(nullptr)
    , m_historyFilePath("chat_history.txt")
{
    ui->setupUi(this);

    m_chatManager = new ChatManager(this);

    ui->usernameEdit->setText(m_chatManager->username());
    ui->visibilityCheckBox->setChecked(m_chatManager->isVisible());

    setupConnections();
    m_chatManager->start();

    setWindowTitle(QString("LAN WiFi P2P Chat - %1 (Port: %2)")
                   .arg(m_chatManager->username())
                   .arg(m_chatManager->tcpPort()));
    
    ui->statusLabel->setText(QString("Discovery port: 45454 | TCP port: %1")
                             .arg(m_chatManager->tcpPort()));

    loadChatHistory();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    connect(m_chatManager, &ChatManager::peerListUpdated, this, &MainWindow::onPeerListUpdated);
    connect(m_chatManager, &ChatManager::messageReceived, this, &MainWindow::onMessageReceived);
    connect(ui->peersListWidget, &QListWidget::itemSelectionChanged, this, &MainWindow::onPeerSelected);

    // enter key sends the message
    connect(ui->messageEdit, &QLineEdit::returnPressed, this, &MainWindow::on_sendButton_clicked);
}

void MainWindow::loadChatHistory()
{
    QFile file(m_historyFilePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        ui->chatDisplay->setPlainText(in.readAll());
        file.close();

        // FIXME: scroll doesnt always go to the very bottom, not sure why
        QScrollBar *sb = ui->chatDisplay->verticalScrollBar();
        if (sb) {
            sb->setValue(sb->maximum());
        }
    }
}

void MainWindow::appendToChatLog(const QString &sender, const QString &message, const QDateTime &timestamp, bool saveToFile)
{
    QString formattedTime = timestamp.toString("yyyy-MM-dd hh:mm:ss");
    QString logLine = QString("[%1] %2: %3").arg(formattedTime).arg(sender).arg(message);

    ui->chatDisplay->append(logLine);

    if (saveToFile) {
        QFile file(m_historyFilePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << logLine << "\n";
            file.close();
        }
    }
}

void MainWindow::onPeerListUpdated()
{
    // remember who was selected so we can re-select them after refreshing
    QString previouslySelectedId = m_selectedPeerId;

    ui->peersListWidget->clear();
    
    QList<PeerInfo> activePeers = m_chatManager->onlinePeers();
    QListWidgetItem* itemToSelect = nullptr;

    for (const PeerInfo &peer : activePeers) {
        auto *item = new QListWidgetItem(peer.displayName(), ui->peersListWidget);
        item->setData(Qt::UserRole, peer.id);

        if (peer.id == previouslySelectedId) {
            itemToSelect = item;
        }
    }

    if (itemToSelect) {
        ui->peersListWidget->setCurrentItem(itemToSelect);
    } else if (!previouslySelectedId.isEmpty()) {
        m_selectedPeerId.clear();
        ui->messageEdit->setPlaceholderText("Select a peer from the list and type your message here...");
        ui->statusLabel->setText("Selected peer went offline.");
    }
}

void MainWindow::onPeerSelected()
{
    QListWidgetItem* currentItem = ui->peersListWidget->currentItem();
    if (currentItem) {
        m_selectedPeerId = currentItem->data(Qt::UserRole).toString();
        PeerInfo peer = m_chatManager->peerInfo(m_selectedPeerId);
        ui->messageEdit->setPlaceholderText(QString("Chatting with %1...").arg(peer.username));
    } else {
        m_selectedPeerId.clear();
        ui->messageEdit->setPlaceholderText("Select a peer from the list and type your message here...");
    }
}

void MainWindow::on_sendButton_clicked()
{
    if (m_selectedPeerId.isEmpty()) {
        ui->statusLabel->setText("Error - select a peer first!");
        return;
    }

    QString text = ui->messageEdit->text().trimmed();
    if (text.isEmpty()) return;

    m_chatManager->sendMessage(m_selectedPeerId, text);
    appendToChatLog("Me", text, QDateTime::currentDateTime(), true);

    ui->messageEdit->clear();
    ui->messageEdit->setFocus();
}

void MainWindow::onMessageReceived(const QString &senderName, const QString &text, const QDateTime &timestamp)
{
    appendToChatLog(senderName, text, timestamp, true);
    
    ui->statusLabel->setText(QString("Message from %1 at %2")
                             .arg(senderName)
                             .arg(timestamp.toString("hh:mm:ss")));
}

void MainWindow::on_saveUsernameButton_clicked()
{
    QString newUsername = ui->usernameEdit->text().trimmed();
    if (newUsername.isEmpty()) {
        QMessageBox::warning(this, "Invalid Username", "Username cannot be empty.");
        return;
    }

    m_chatManager->setUsername(newUsername);
    
    setWindowTitle(QString("LAN WiFi P2P Chat - %1 (Port: %2)")
                   .arg(m_chatManager->username())
                   .arg(m_chatManager->tcpPort()));

    ui->statusLabel->setText(QString("Username changed to %1").arg(newUsername));
}

void MainWindow::on_visibilityCheckBox_stateChanged(int state)
{
    bool isVisible = (state == Qt::Checked);
    m_chatManager->setVisibility(isVisible);

    if (isVisible) {
        ui->statusLabel->setText("You are now visible to nearby users");
    } else {
        ui->statusLabel->setText("You are now hidden");
    }
}
