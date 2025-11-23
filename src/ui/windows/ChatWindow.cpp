/**
 * @file ChatWindow.cpp
 * @brief Chat window implementation
 * @author FlyKylin Development Team
 * @date 2024-11-19
 */

#include "ChatWindow.h"
#include "../../core/config/UserProfile.h"
#include <QCloseEvent>
#include <QScrollBar>
#include <QDateTime>
#include <QDebug>
#include <QFileDialog>
#include <QPixmap>

namespace flykylin {
namespace ui {

ChatWindow::ChatWindow(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_peerNameLabel(nullptr)
    , m_scrollArea(nullptr)
    , m_messageContainer(nullptr)
    , m_messageLayout(nullptr)
    , m_inputEdit(nullptr)
    , m_sendButton(nullptr)
    , m_imageButton(nullptr)
    , m_fileButton(nullptr)
    , m_viewModel(nullptr)
{
    setupUI();
    
    // Create ViewModel
    m_viewModel = new ui::ChatViewModel(this);
    
    connectSignals();
    
    qInfo() << "[ChatWindow] Created";
}

ChatWindow::~ChatWindow() {
    qInfo() << "[ChatWindow] Destroyed";
}

void ChatWindow::setupUI() {
    // Main layout
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    // Peer name label (header)
    m_peerNameLabel = new QLabel("Chat", this);
    m_peerNameLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   font-weight: bold;"
        "   padding: 5px;"
        "   background-color: #f0f0f0;"
        "   border-radius: 3px;"
        "}"
    );
    m_mainLayout->addWidget(m_peerNameLabel);
    
    // Message scroll area
    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // Message container
    m_messageContainer = new QWidget();
    m_messageLayout = new QVBoxLayout(m_messageContainer);
    m_messageLayout->setContentsMargins(5, 5, 5, 5);
    m_messageLayout->setSpacing(10);
    m_messageLayout->addStretch(); // Push messages to top
    
    m_scrollArea->setWidget(m_messageContainer);
    m_mainLayout->addWidget(m_scrollArea, 1); // Stretch factor 1
    
    // Input area (horizontal layout)
    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(5);
    
    // Text input
    m_inputEdit = new QTextEdit(this);
    m_inputEdit->setPlaceholderText("Type a message...");
    m_inputEdit->setMaximumHeight(80);
    m_inputEdit->setMinimumHeight(40);
    inputLayout->addWidget(m_inputEdit, 1);
    
    m_imageButton = new QPushButton("Img", this);
    m_imageButton->setFixedWidth(60);
    inputLayout->addWidget(m_imageButton);

    m_fileButton = new QPushButton("File", this);
    m_fileButton->setFixedWidth(60);
    inputLayout->addWidget(m_fileButton);

    m_sendButton = new QPushButton("Send", this);
    m_sendButton->setFixedWidth(80);
    m_sendButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #0078d4;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 3px;"
        "   padding: 8px;"
        "   font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #106ebe;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #005a9e;"
        "}"
    );
    inputLayout->addWidget(m_sendButton);
    
    m_mainLayout->addLayout(inputLayout);
    
    // Window settings
    setWindowTitle("Chat Window");
    resize(500, 600);
}

void ChatWindow::connectSignals() {
    // Send button clicked
    connect(m_sendButton, &QPushButton::clicked,
            this, &ChatWindow::onSendButtonClicked);
    connect(m_imageButton, &QPushButton::clicked,
            this, &ChatWindow::onSendImageButtonClicked);
    connect(m_fileButton, &QPushButton::clicked,
            this, &ChatWindow::onSendFileButtonClicked);
    
    // ViewModel signals
    connect(m_viewModel, &ui::ChatViewModel::messagesUpdated,
            this, &ChatWindow::onMessagesUpdated);
    connect(m_viewModel, &ui::ChatViewModel::messageReceived,
            this, &ChatWindow::onMessageReceived);
    connect(m_viewModel, &ui::ChatViewModel::messageSent,
            this, &ChatWindow::onMessageSent);
    connect(m_viewModel, &ui::ChatViewModel::messageFailed,
            this, &ChatWindow::onMessageFailed);
}

void ChatWindow::setPeer(const QString& peerId, const QString& peerName) {
    m_currentPeerId = peerId;
    m_currentPeerName = peerName;
    
    // Update header
    m_peerNameLabel->setText(QString("Chat with %1").arg(peerName));
    
    // Set peer in ViewModel
    m_viewModel->setCurrentPeer(peerId, peerName);
    
    // Load message history
    refreshMessageList();
    
    qInfo() << "[ChatWindow] Set peer to" << peerId << peerName;
}

QString ChatWindow::currentPeerId() const {
    return m_currentPeerId;
}

void ChatWindow::clearHistory() {
    m_viewModel->clearHistory();
    refreshMessageList();
}

void ChatWindow::closeEvent(QCloseEvent* event) {
    qInfo() << "[ChatWindow] Closing";
    emit closed();
    event->accept();
}

void ChatWindow::onSendButtonClicked() {
    QString text = m_inputEdit->toPlainText().trimmed();
    
    if (text.isEmpty()) {
        qWarning() << "[ChatWindow] Cannot send empty message";
        return;
    }
    
    if (m_currentPeerId.isEmpty()) {
        qWarning() << "[ChatWindow] No peer selected";
        return;
    }
    
    // Send via ViewModel
    m_viewModel->sendMessage(text);
    
    // Clear input
    m_inputEdit->clear();
    
    qDebug() << "[ChatWindow] Sent message:" << text.left(20) << "...";
}

void ChatWindow::onSendImageButtonClicked() {
    if (m_currentPeerId.isEmpty()) {
        qWarning() << "[ChatWindow] No peer selected for image";
        return;
    }

    QString filter = tr("Images (*.png *.jpg *.jpeg *.gif *.bmp);;All Files (*.*)");
    QString filePath = QFileDialog::getOpenFileName(this, tr("Select image"), QString(), filter);
    if (filePath.isEmpty()) {
        return;
    }

    m_viewModel->sendImage(filePath);
}

void ChatWindow::onSendFileButtonClicked() {
    if (m_currentPeerId.isEmpty()) {
        qWarning() << "[ChatWindow] No peer selected for file";
        return;
    }

    QString filePath = QFileDialog::getOpenFileName(this, tr("Select file"));
    if (filePath.isEmpty()) {
        return;
    }

    m_viewModel->sendFile(filePath);
}

void ChatWindow::onMessagesUpdated() {
    qDebug() << "[ChatWindow] Messages updated, refreshing";
    refreshMessageList();
}

void ChatWindow::onMessageReceived(const flykylin::core::Message& message) {
    qInfo() << "[ChatWindow] Message received from" << message.fromUserId();
    // Message already added by onMessagesUpdated
    scrollToBottom();
}

void ChatWindow::onMessageSent(const flykylin::core::Message& message) {
    qInfo() << "[ChatWindow] Message sent to" << message.toUserId();
    // Message already added by onMessagesUpdated
    scrollToBottom();
}

void ChatWindow::onMessageFailed(const flykylin::core::Message& message, QString error) {
    qWarning() << "[ChatWindow] Message failed:" << error;
    // TODO: Show error indicator on message bubble
    // For now, the message is still visible but we log the error
}

void ChatWindow::refreshMessageList() {
    // Clear existing messages (but keep the final stretch if it exists)
    QLayoutItem* item;
    while ((item = m_messageLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }
    
    // Ensure we have a stretch at the end for proper message insertion
    m_messageLayout->addStretch();
    
    // Get messages from ViewModel
    QList<core::Message> messages = m_viewModel->getMessages();
    
    qDebug() << "[ChatWindow] Refreshing" << messages.size() << "messages";
    
    // Add message bubbles (they will be inserted before the stretch)
    for (const auto& message : messages) {
        addMessageBubble(message);
    }
    
    // Scroll to bottom
    scrollToBottom();
}

void ChatWindow::scrollToBottom() {
    QScrollBar* scrollBar = m_scrollArea->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void ChatWindow::addMessageBubble(const flykylin::core::Message& message) {
    // Determine if this is a sent or received message based on local user ID
    const QString localUserId = core::UserProfile::instance().userId();
    bool isSent = (message.fromUserId() == localUserId);
    
    // Create container widget for this message
    QWidget* messageWidget = new QWidget(m_messageContainer);
    QHBoxLayout* horizontalLayout = new QHBoxLayout(messageWidget);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setSpacing(5);
    
    // Add left spacer for sent messages
    if (isSent) {
        horizontalLayout->addStretch();
    }
    
    // Create vertical layout for bubble + timestamp
    QWidget* bubbleContainer = new QWidget(messageWidget);
    QVBoxLayout* verticalLayout = new QVBoxLayout(bubbleContainer);
    verticalLayout->setContentsMargins(0, 0, 0, 0);
    verticalLayout->setSpacing(2);
    
    QLabel* bubbleLabel = new QLabel(bubbleContainer);
    bubbleLabel->setMaximumWidth(350);

    if (message.kind() == flykylin::core::MessageKind::Image) {
        QPixmap pix(message.attachmentLocalPath());
        if (!pix.isNull()) {
            if (pix.width() > 260) {
                pix = pix.scaledToWidth(260, Qt::SmoothTransformation);
            }
            bubbleLabel->setPixmap(pix);
        } else {
            bubbleLabel->setText(message.attachmentName());
            bubbleLabel->setWordWrap(true);
            bubbleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        }
    } else if (message.kind() == flykylin::core::MessageKind::File) {
        bubbleLabel->setText(message.attachmentName());
        bubbleLabel->setWordWrap(true);
        bubbleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        bubbleLabel->setStyleSheet(
            "QLabel {"
            "   background-color: #f5f5f5;"
            "   color: black;"
            "   border-radius: 6px;"
            "   padding: 6px 10px;"
            "}"
        );
    } else {
        bubbleLabel->setText(message.content());
        bubbleLabel->setWordWrap(true);
        bubbleLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        if (isSent) {
            bubbleLabel->setStyleSheet(
                "QLabel {"
                "   background-color: #0078d4;"
                "   color: white;"
                "   border-radius: 10px;"
                "   padding: 8px 12px;"
                "}"
            );
        } else {
            bubbleLabel->setStyleSheet(
                "QLabel {"
                "   background-color: #e5e5e5;"
                "   color: black;"
                "   border-radius: 10px;"
                "   padding: 8px 12px;"
                "}"
            );
        }
    }
    
    // Add timestamp (small label below bubble)
    QLabel* timestampLabel = new QLabel(
        message.timestamp().toString("hh:mm"),
        bubbleContainer
    );
    timestampLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 10px;"
        "   color: gray;"
        "}"
    );
    
    // Set timestamp alignment to match bubble
    if (isSent) {
        timestampLabel->setAlignment(Qt::AlignRight);
    } else {
        timestampLabel->setAlignment(Qt::AlignLeft);
    }
    
    // Add bubble and timestamp to vertical layout
    verticalLayout->addWidget(bubbleLabel);
    verticalLayout->addWidget(timestampLabel);
    
    // Add bubble container to horizontal layout
    horizontalLayout->addWidget(bubbleContainer);
    
    // Add right spacer for received messages
    if (!isSent) {
        horizontalLayout->addStretch();
    }
    
    // Insert before the final stretch
    int insertIndex = m_messageLayout->count() - 1;
    m_messageLayout->insertWidget(insertIndex, messageWidget);
}

} // namespace ui
} // namespace flykylin
