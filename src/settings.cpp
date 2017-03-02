#include "settings.h"
#include "ui_settings.h"

settings::settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::settings)
{
    ui->setupUi(this);
}

settings::~settings()
{
    delete ui;
}

void settings::setMessage(const QString &message)
{
    ui->plainTextEdit->setPlainText(message);
}

void settings::setUserName(const QString &userName)
{
    ui->lineEditLogin->setText(userName);
}

void settings::setPassword(const QString &password)
{
    ui->lineEditPassword->setText(password);
}

QString settings::getMessage() const
{
    return ui->plainTextEdit->toPlainText();
}

QString settings::getUserName() const
{
    return ui->lineEditLogin->text();
}

QString settings::getPassword() const
{
    return ui->lineEditPassword->text();
}
