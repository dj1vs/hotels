#include "bookwidget.h"

#include <QLabel>
#include <QDebug>
#include <QCompleter>
#include <QMessageBox>

BookWidget::BookWidget(SQLWorker *w, int _guestId) {
    worker = w;
    guestId = _guestId;

    setupUi();
    setupWorker();

    emit getFreeRooms(QDate::currentDate(), QDate::currentDate());
    emit getGuest(guestId);
}

void BookWidget::setupUi() {


    freeRoomsModel = new QStandardItemModel;
    freeRoomsTable = new QTableView;

    freeRoomsModel->setColumnCount(header.size());
    for (int i = 0; i < header.size(); ++i) {
        freeRoomsModel->setHeaderData(i, Qt::Horizontal, header[i]);
    }
    freeRoomsTable->setModel(freeRoomsModel);

    guestName = new QLineEdit;
    guestName->setReadOnly(true);

    roomBox = new QComboBox;
    calendar = new CalendarWidget;

    bookButton = new QPushButton("Забронировать");
    exitButton = new QPushButton("Обратно");

    guestLayout = new QHBoxLayout;
    guestLayout->addWidget(new QLabel("Имя гостя:"));
    guestLayout->addWidget(guestName);

    roomLayout = new QHBoxLayout;
    roomLayout->addWidget(new QLabel("Номер:"));
    roomLayout->addWidget(roomBox);

    calendarLayout = new QHBoxLayout;
    calendarLayout->addWidget(new QLabel("Даты бронирования:"));
    calendarLayout->addWidget(calendar);

    layout = new QVBoxLayout;
    layout->addWidget(new QLabel("Бронирование номеров"));
    layout->addLayout(guestLayout);
    layout->addWidget(new QLabel("Свободные номера:"));
    layout->addWidget(freeRoomsTable);
    layout->addLayout(roomLayout);
    layout->addLayout(calendarLayout);
    layout->addWidget(bookButton);
    layout->addWidget(exitButton);

    setLayout(layout);

    connect(exitButton, &QPushButton::clicked, this, [this] {
        emit exit();
    });

}

void BookWidget::setupWorker() {
    connect(this, &BookWidget::getFreeRooms, worker, &SQLWorker::getFreeRooms);
    connect(worker, &SQLWorker::getFreeRoomsReady, this, &BookWidget::processFreeRooms);

    connect(this, &BookWidget::getGuests, worker, &SQLWorker::getGuests);
    connect(worker, &SQLWorker::getGuestsReady, this, &BookWidget::processGuests);

    connect(this, &BookWidget::getGuest, worker, &SQLWorker::getGuest);
    connect(worker, &SQLWorker::getGuestReady, this, &BookWidget::processGuest);

    connect(bookButton, &QPushButton::clicked, this, &BookWidget::makeBooking);
    connect(this, &BookWidget::book, worker, &SQLWorker::book);

    connect(calendar, &CalendarWidget::rangeChanged, this, [this] {emit getFreeRooms(calendar->getFromDate(), calendar->getToDate());});
}

void BookWidget::makeBooking() {

    const QPair<QDate, QDate> dateRange = calendar->getRange();
    const int roomNumber = roomBox->currentText().toInt();

    emit book(guestId, roomNumber, dateRange.first, dateRange.second);

    QMessageBox::information(this, "Бронирование", "Забронировано!");

    emit getFreeRooms(calendar->getFromDate(), calendar->getToDate());
}

void BookWidget::processFreeRooms(QVector <QMap <QString, QVariant>> rooms) {
    roomBox->clear();
    freeRoomsModel->removeRows(0, freeRoomsModel->rowCount());
    freeRoomsModel->setRowCount(rooms.size());
    for (int i = 0; i < rooms.size(); ++i) {
        const auto room = rooms[i];

        const auto hotelName = room["hotel_name"];
        const auto kind = room["kind"];
        const auto number = room["number"];
        const auto price = room["price"];

        freeRoomsModel->setData(freeRoomsModel->index(i, 0), hotelName);
        freeRoomsModel->setData(freeRoomsModel->index(i, 1), kind);
        freeRoomsModel->setData(freeRoomsModel->index(i, 2), number);
        freeRoomsModel->setData(freeRoomsModel->index(i, 3), price);

        roomBox->addItem(number.toString());
    }
}

void BookWidget::processGuests(QStringList guests) {
    guestName->setCompleter(new QCompleter(guests));
}

void BookWidget::processGuest(QString name) {
    guestName->setText(name);
}
