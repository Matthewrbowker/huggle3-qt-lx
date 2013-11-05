//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "uaareport.h"
#include "ui_uaareport.h"

using namespace Huggle;

UAAReport::UAAReport(QWidget *parent) : QDialog(parent), ui(new Ui::UAAReport)
{
    ui->setupUi(this);
    this->uaat = NULL;
    this->User = NULL;
    this->contentsOfUAA = "";
    this->qUAApage = NULL;
    this->page = NULL;
    this->uT = NULL;
    this->dr = "";
    this->optionalreason = "";
    this->ta = "";
    this->uaaReportReason = "";
}

UAAReport::~UAAReport()
{
    delete ui;
    delete uaat;
    delete User;
    delete uT;
    delete page;
}

void UAAReport::setUserForUAA(WikiUser *user)
{
    this->User = user;
}

void UAAReport::getPageContents()
{
    this->qUAApage = new ApiQuery();
    qUAApage->SetAction(ActionQuery);
    qUAApage->Parameters = "action=query&prop=revisions&rvprop=content&titles=" + QUrl::toPercentEncoding(Configuration::LocalConfig_UAAPath);
    qUAApage->Target = "Getting content of UAA";
    qUAApage->RegisterConsumer("UAAReport::getPageContents()");
    Core::RunningQueries.append(qUAApage);
    qUAApage->Process();

    connect(this->uT, SIGNAL(timeout()), this, SLOT(onTick()));
    this->uT = new QTimer(this);
    this->uT->start(200);
}

void UAAReport::onTick()
{
    if (!this->qUAApage->Processed())
    {
        return;
    }
    QDomDocument r;
    r.setContent(this->qUAApage->Result->Data);
    QDomNodeList l = r.elementsByTagName("rev");
    if (l.count() == 0)
    {
        this->failed("the query for the page contents returned no data.");
    }
    QDomElement element = l.at(0).toElement();
    if (!element.attributes().contains("rev"))
    {
        this->failed("the page contents weren't available.");
        return;
    }
    this->dr = element.text();
    this->qUAApage->UnregisterConsumer("UAAReport::getPageContents()");
    this->qUAApage = NULL;
    Core::DebugLog("Contents of UAA: " + this->dr);
    if (!this->ui->checkBox->isChecked() && !this->ui->checkBox_2->isChecked() &&
            !this->ui->checkBox_3->isChecked() && !this->ui->checkBox_4->isChecked()
            && this->ui->lineEdit->text().isEmpty())
    {
        QMessageBox *g = new QMessageBox();
        g->setWindowTitle("No reason specified");
        g->setText("You didn't specify a reason as to why the username is a policy violation. "\
                   "Please specify a reason.");
        g->setAttribute(Qt::WA_DeleteOnClose);
        g->exec();

        return;
    }
    QString uaasum = "Reporting " + this->User->Username + " to UAA " + Configuration::EditSuffixOfHuggle;
    this->uT->stop();
    this->insertUsername();
    Core::EditPage(Core::UAAP, dr, uaasum, true);
    Core::Log("Reporting" + this->User->Username + " to UAA" );
    this->ui->pushButton->setEnabled(false);
    this->ui->pushButton->setText("Reported");

}
void UAAReport::insertUsername()
{
    ta = Configuration::LocalConfig_UAATemplate;
    ta.replace("$1", this->User->Username);
    contentsOfUAA = ta + uaaReportReason + optionalreason;
    dr = dr + "\n" + contentsOfUAA;
}

void UAAReport::whatToReport()
{
    optionalreason = this->ui->lineEdit->text();
    if (this->ui->checkBox->isChecked())
    {
        uaaReportReason = "Username is a policy violation because it is disruptive.";
    }
    if (this->ui->checkBox_2->isChecked())
    {
        uaaReportReason = "Username is a policy violation because it is offensive.";
    }
    if (this->ui->checkBox_3->isChecked())
    {
        uaaReportReason = "Username is a policy violation because it is a promotional username.";
    }
    if (this->ui->checkBox_4->isChecked())
    {
        uaaReportReason = "Username is a policy violation because it is a misleading username.";
    }

}

void UAAReport::failed(QString reason)
{
    QMessageBox *_b = new QMessageBox();
    _b->setWindowTitle("Unable to report user to UAA");
    _b->setText("Unable to report the user because " + reason);
    _b->setAttribute(Qt::WA_DeleteOnClose);
    _b->exec();
    return;
}

void UAAReport::on_pushButton_clicked()
{
    this->getPageContents();
}

void UAAReport::on_pushButton_2_clicked()
{
    this->hide();
}