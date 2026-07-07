/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.11.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *profileGroupBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *usernameLabel;
    QLineEdit *usernameEdit;
    QPushButton *saveUsernameButton;
    QSpacerItem *horizontalSpacer;
    QCheckBox *visibilityCheckBox;
    QSplitter *splitter;
    QWidget *leftContainer;
    QVBoxLayout *verticalLayout_3;
    QLabel *onlineLabel;
    QListWidget *peersListWidget;
    QWidget *rightContainer;
    QVBoxLayout *verticalLayout;
    QTextEdit *chatDisplay;
    QHBoxLayout *horizontalLayout;
    QLineEdit *messageEdit;
    QPushButton *sendButton;
    QLabel *statusLabel;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        verticalLayout_2 = new QVBoxLayout(centralwidget);
        verticalLayout_2->setObjectName("verticalLayout_2");
        profileGroupBox = new QGroupBox(centralwidget);
        profileGroupBox->setObjectName("profileGroupBox");
        horizontalLayout_2 = new QHBoxLayout(profileGroupBox);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        usernameLabel = new QLabel(profileGroupBox);
        usernameLabel->setObjectName("usernameLabel");

        horizontalLayout_2->addWidget(usernameLabel);

        usernameEdit = new QLineEdit(profileGroupBox);
        usernameEdit->setObjectName("usernameEdit");

        horizontalLayout_2->addWidget(usernameEdit);

        saveUsernameButton = new QPushButton(profileGroupBox);
        saveUsernameButton->setObjectName("saveUsernameButton");

        horizontalLayout_2->addWidget(saveUsernameButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        visibilityCheckBox = new QCheckBox(profileGroupBox);
        visibilityCheckBox->setObjectName("visibilityCheckBox");
        visibilityCheckBox->setChecked(true);

        horizontalLayout_2->addWidget(visibilityCheckBox);


        verticalLayout_2->addWidget(profileGroupBox);

        splitter = new QSplitter(centralwidget);
        splitter->setObjectName("splitter");
        splitter->setOrientation(Qt::Horizontal);
        leftContainer = new QWidget(splitter);
        leftContainer->setObjectName("leftContainer");
        verticalLayout_3 = new QVBoxLayout(leftContainer);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        onlineLabel = new QLabel(leftContainer);
        onlineLabel->setObjectName("onlineLabel");

        verticalLayout_3->addWidget(onlineLabel);

        peersListWidget = new QListWidget(leftContainer);
        peersListWidget->setObjectName("peersListWidget");

        verticalLayout_3->addWidget(peersListWidget);

        splitter->addWidget(leftContainer);
        rightContainer = new QWidget(splitter);
        rightContainer->setObjectName("rightContainer");
        verticalLayout = new QVBoxLayout(rightContainer);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        chatDisplay = new QTextEdit(rightContainer);
        chatDisplay->setObjectName("chatDisplay");
        chatDisplay->setReadOnly(true);

        verticalLayout->addWidget(chatDisplay);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        messageEdit = new QLineEdit(rightContainer);
        messageEdit->setObjectName("messageEdit");

        horizontalLayout->addWidget(messageEdit);

        sendButton = new QPushButton(rightContainer);
        sendButton->setObjectName("sendButton");

        horizontalLayout->addWidget(sendButton);


        verticalLayout->addLayout(horizontalLayout);

        splitter->addWidget(rightContainer);

        verticalLayout_2->addWidget(splitter);

        statusLabel = new QLabel(centralwidget);
        statusLabel->setObjectName("statusLabel");

        verticalLayout_2->addWidget(statusLabel);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "LAN WiFi P2P Chat", nullptr));
        profileGroupBox->setTitle(QCoreApplication::translate("MainWindow", "My Profile Settings", nullptr));
        usernameLabel->setText(QCoreApplication::translate("MainWindow", "Your Username:", nullptr));
        saveUsernameButton->setText(QCoreApplication::translate("MainWindow", "Update Username", nullptr));
        visibilityCheckBox->setText(QCoreApplication::translate("MainWindow", "Visible to Nearby Users", nullptr));
        onlineLabel->setText(QCoreApplication::translate("MainWindow", "Online Peers (Select to chat):", nullptr));
        messageEdit->setPlaceholderText(QCoreApplication::translate("MainWindow", "Select a peer from the list and type your message here...", nullptr));
        sendButton->setText(QCoreApplication::translate("MainWindow", "Send Message", nullptr));
        statusLabel->setText(QCoreApplication::translate("MainWindow", "Status: Offline", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
