// Copyright (c) 2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// File contains modifications by: The Gulden developers
// All modifications:
// Copyright (c) 2017-2018 The Gulden developers
// Authored by: Malcolm MacLeod (mmacleod@webmail.co.za)
// Distributed under the GULDEN software license, see the accompanying
// file COPYING

#include "syncoverlay.h"
#include "ui_syncoverlay.h"

#include "guiutil.h"

#include "chainparams.h"

#include <QResizeEvent>
#include <QPropertyAnimation>

#include "_Gulden/GuldenGUI.h"
#include "validation/validation.h"

SyncOverlay::SyncOverlay(QWidget *parent)
: QWidget(parent)
, ui(new Ui::SyncOverlay)
, bestHeaderHeight(0)
, bestHeaderDate(QDateTime())
, layerIsVisible(false)
, userClosed(false)
{
    ui->setupUi(this);
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(closeClicked()));
    if (parent) {
        parent->installEventFilter(this);
        raise();
    }

    setVisible(false);


    ui->verticalLayoutMain->setSpacing(0);
    ui->verticalLayoutMain->setContentsMargins( 0, 0, 0, 0 );

    ui->verticalLayoutSub->setSpacing(0);
    ui->verticalLayoutSub->setContentsMargins( 0, 0, 0, 0 );

    ui->verticalLayout->setSpacing(0);
    ui->verticalLayout->setContentsMargins(0,0,0,0);

    ui->infoText->setContentsMargins(0,0,0,0);
    ui->infoText->setIndent(0);

    ui->closeButton->setContentsMargins(0,0,0,0);
    ui->closeButton->setCursor(Qt::PointingHandCursor);

    ui->bgWidget->setStyleSheet("");
    ui->contentWidget->setStyleSheet("");

    ui->bgWidget->setPalette( QApplication::palette( ui->bgWidget ) );
    ui->contentWidget->setPalette( QApplication::palette( ui->contentWidget ) );

    ui->verticalLayout->insertStretch(0, 1);
    ui->verticalLayout->setStretch(1, 0);
    ui->verticalLayout->setStretch(2, 0);

    ui->infoText->setText("");
}

SyncOverlay::~SyncOverlay()
{
    delete ui;
}

bool SyncOverlay::eventFilter(QObject * obj, QEvent * ev) {
    if (obj == parent()) {
        if (ev->type() == QEvent::Resize) {
            QResizeEvent * rev = static_cast<QResizeEvent*>(ev);
            resize(rev->size());
            if (!layerIsVisible)
                setGeometry(0, height(), width(), height());

        }
        else if (ev->type() == QEvent::ChildAdded) {
            raise();
        }
    }
    return QWidget::eventFilter(obj, ev);
}

//! Tracks parent widget changes
bool SyncOverlay::event(QEvent* ev) {
    if (ev->type() == QEvent::ParentAboutToChange) {
        if (parent()) parent()->removeEventFilter(this);
    }
    else if (ev->type() == QEvent::ParentChange) {
        if (parent()) {
            parent()->installEventFilter(this);
            raise();
        }
    }
    return QWidget::event(ev);
}

void SyncOverlay::setKnownBestHeight(int count, const QDateTime& blockDate)
{
    (unused) blockDate;
    static bool doOnceOnly = true;
    if (doOnceOnly)
    {
        int messageChangeThreshold = IsArgSet("-testnet") ? 1000 : 5000;
        if (chainActive.Tip() && chainActive.Tip()->nHeight > messageChangeThreshold)
        {
            ui->infoText->setText(tr("<br/><br/><b>Notice</b><br/><br/>Your wallet is now synchronizing with the Gulden network.<br/>Once your wallet has finished synchronizing, your balance and recent transactions will be visible."));
        }
        else
        {
            ui->infoText->setText(tr("<br/><br/><b>Notice</b><br/><br/>Your wallet is now synchronizing with the Gulden network for the first time.<br/>Once your wallet has finished synchronizing, your balance and recent transactions will be visible."));
        }
        doOnceOnly = false;
    }
}

void SyncOverlay::tipUpdate(int count, const QDateTime& blockDate, double nSyncProgress)
{
    (unused) count;
    (unused) blockDate;
    (unused) nSyncProgress;
}

void SyncOverlay::toggleVisibility()
{
    showHide(layerIsVisible, true);
    if (!layerIsVisible)
        userClosed = true;
}

void SyncOverlay::showHide(bool hide, bool userRequested)
{
    if ( (layerIsVisible && !hide) || (!layerIsVisible && hide) || (!hide && userClosed && !userRequested))
        return;

    if (!isVisible() && !hide)
        setVisible(true);

    setGeometry(0, hide ? 0 : height(), width(), height());

    QPropertyAnimation* animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(3500);
    animation->setStartValue(QPoint(0, hide ? 0 : this->height()));
    animation->setEndValue(QPoint(0, hide ? this->height() : 0));
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    layerIsVisible = !hide;
}

void SyncOverlay::closeClicked()
{
    showHide(true);
    userClosed = true;
}
