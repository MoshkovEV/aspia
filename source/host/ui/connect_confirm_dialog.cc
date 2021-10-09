//
// Aspia Project
// Copyright (C) 2021 Dmitry Chapyshev <dmitry@aspia.ru>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//

#include "host/ui/connect_confirm_dialog.h"

#include "common/session_type.h"

#include <QAbstractButton>
#include <QTimer>

namespace host {

ConnectConfirmDialog::ConnectConfirmDialog(const proto::internal::ConnectConfirmationRequest& request,
                                           QWidget* parent)
    : QDialog(parent),
      timer_(new QTimer(this)),
      auto_accept_(request.timeout() > 0)
{
    ui.setupUi(this);
    setWindowFlag(Qt::WindowStaysOnTopHint);

    connect(timer_, &QTimer::timeout, this, &ConnectConfirmDialog::onTimeout);
    connect(ui.button_box, &QDialogButtonBox::clicked,
            this, &ConnectConfirmDialog::onButtonBoxClicked);

    if (auto_accept_)
    {
        time_seconds_ = static_cast<int>(request.timeout() / 1000);
    }
    else
    {
        time_seconds_ = 60;
    }

    QString user_name = QLatin1String("<b>") + QString::fromStdString(request.user_name()) +
        QLatin1String("</b>");
    QString computer_name = QLatin1String("<b>") + QString::fromStdString(request.computer_name()) +
        QLatin1String("</b>");
    QString session_name = QLatin1String("<b>") +
        common::sessionTypeToLocalizedString(request.session_type()) + QLatin1String("</b>");

    message_ = tr("User %1 (%2) is trying to connect to your computer with %3 session type.")
        .arg(user_name, computer_name, session_name);
    question_ = tr("Allow the connection?");

    updateMessage();

    timer_->start(std::chrono::seconds(1));
}

ConnectConfirmDialog::~ConnectConfirmDialog() = default;

void ConnectConfirmDialog::onButtonBoxClicked(QAbstractButton* button)
{
    QDialogButtonBox::StandardButton standard_button = ui.button_box->standardButton(button);
    if (standard_button == QDialogButtonBox::Yes)
    {
        accept();
    }
    else
    {
        reject();
    }

    close();
}

void ConnectConfirmDialog::onTimeout()
{
    time_seconds_ -= 1;

    if (time_seconds_ <= 0)
    {
        if (auto_accept_)
            accept();
        else
            reject();

        close();
    }
    else
    {
        updateMessage();
    }
}

void ConnectConfirmDialog::updateMessage()
{
    QString timeout_string;

    if (auto_accept_)
    {
        timeout_string =
            tr("The connection will be automatically accepted after %n seconds.", "", time_seconds_);
    }
    else
    {
        timeout_string =
            tr("The connection will be automatically rejected after %n seconds.", "", time_seconds_);
    }

    ui.label_msg->setText(
        message_ + QLatin1String("<br/>") + timeout_string + QLatin1String("<br/><b>") + question_ +
        QLatin1String("</b>"));
}

} // namespace host