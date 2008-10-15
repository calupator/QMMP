/***************************************************************************
 *   Copyright (C) 2006-2008 by Ilya Kotov                                 *
 *   forkotov02@hotmail.ru                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <QtGui>
#include <QFileDialog>
#include <QDir>
#include <QAction>
#include <QMenu>

#include <math.h>

#include <qmmp/soundcore.h>
#include <qmmp/visual.h>
#include <qmmpui/generalhandler.h>
#include <qmmpui/general.h>
#include <qmmpui/playlistparser.h>
#include <qmmpui/playlistformat.h>
#include <qmmpui/commandlinemanager.h>
#include <qmmpui/filedialog.h>

#include "textscroller.h"
#include "mainwindow.h"
#include "fileloader.h"
#include "skin.h"
#include "playlist.h"
#include "playlistmodel.h"
#include "configdialog.h"
#include "dock.h"
#include "eqwidget.h"
#include "mainvisual.h"
#include "jumptotrackdialog.h"
#include "aboutdialog.h"
#include "addurldialog.h"
#include "listwidget.h"
#include "visualmenu.h"
#include "builtincommandlineoption.h"

#define KEY_OFFSET 10

MainWindow::MainWindow(const QStringList& args, BuiltinCommandLineOption* option_manager, QWidget *parent)
        : QMainWindow(parent)
{
    m_vis = 0;
    seeking = FALSE;
    m_update = FALSE;
    m_paused = FALSE;
    m_elapsed = 0;
    m_option_manager = option_manager;
    m_core = new SoundCore(this);

    setWindowIcon(QIcon(":/32x32/qmmp.png"));

    m_skin = new Skin(this);
    Dock *dock = new Dock(this);
    dock->setMainWidget(this);

    setWindowFlags(Qt::FramelessWindowHint);
    setFixedSize (275,116);

    display = new MainDisplay(this);
    setCentralWidget(display);
    display->show();
    display->setFocus ();

    m_playlistName = tr("Default");

    new PlaylistParser(this);
    m_playlist = new PlayList(this);

    connect (m_playlist,SIGNAL(next()),SLOT(next()));
    connect (m_playlist,SIGNAL(prev()),SLOT(previous()));
    connect (m_playlist,SIGNAL(play()),SLOT(play()));
    connect (m_playlist,SIGNAL(pause()),SLOT(pause()));
    connect (m_playlist,SIGNAL(stop()),SLOT(stop()));
    connect (m_playlist,SIGNAL(eject()),SLOT(addFile()));

    connect (m_playlist,SIGNAL(newPlaylist()),SLOT(newPlaylist()));
    connect (m_playlist,SIGNAL(loadPlaylist()),SLOT(loadPlaylist()));
    connect (m_playlist,SIGNAL(savePlaylist()),SLOT(savePlaylist()));

    m_playListModel = new PlayListModel(this);

    connect(display,SIGNAL(shuffleToggled(bool)),m_playListModel,SLOT(prepareForShufflePlaying(bool)));
    connect(display,SIGNAL(repeatableToggled(bool)),m_playListModel,SLOT(prepareForRepeatablePlaying(bool)));

    dock->addWidget(m_playlist);

    m_equalizer = new EqWidget(this);
    dock->addWidget(m_equalizer);
    connect(m_equalizer, SIGNAL(valueChanged()), SLOT(updateEQ()));

    m_playlist->setModel(m_playListModel);

    m_jumpDialog = new JumpToTrackDialog(this);
    m_jumpDialog->setModel(m_playListModel);
    connect(m_jumpDialog,SIGNAL(playRequest()),this,SLOT(play()));
    m_jumpDialog->hide();

    createActions();

    m_titlebar = new TitleBar(this);
    m_titlebar->move(0,0);
    m_titlebar->show();
    m_titlebar->setActive(TRUE);

    readSettings();
    dock->updateDock();

    display->setEQ(m_equalizer);
    display->setPL(m_playlist);

    m_vis = MainVisual::getPointer();

    Visual::initialize(this, m_visMenu, SLOT(updateActions()));
    Visual::add(m_vis);

    connect(m_core, SIGNAL(finished()), SLOT(next()));
    connect(m_core, SIGNAL(stateChanged(Qmmp::State)), SLOT(showState(Qmmp::State)));
    connect(m_core, SIGNAL(elapsedChanged(qint64)),m_playlist, SLOT(setTime(qint64)));
    connect(m_core, SIGNAL(elapsedChanged(qint64)),m_titlebar, SLOT(setTime(qint64)));
    connect(m_core, SIGNAL(metaDataChanged()),SLOT(showMetaData()));
    connect(m_core, SIGNAL(bufferingProgress(int)), TextScroller::getPointer(),
            SLOT(setProgress(int)));

    updateEQ();

    m_generalHandler = new GeneralHandler(this);
    connect(m_generalHandler, SIGNAL(playCalled()), SLOT(play()));
    connect(m_generalHandler, SIGNAL(nextCalled()), SLOT(next()));
    connect(m_generalHandler, SIGNAL(previousCalled()), SLOT(previous()));
    connect(m_generalHandler, SIGNAL(stopCalled()), SLOT(stop()));
    connect(m_generalHandler, SIGNAL(pauseCalled()), SLOT(pause()));
    connect(m_generalHandler, SIGNAL(seekCalled(int)), SLOT(seek(int)));
    connect(m_generalHandler, SIGNAL(toggleVisibilityCalled()), SLOT(toggleVisibility()));
    connect(m_generalHandler, SIGNAL(exitCalled()), SLOT(close()));
    connect(m_generalHandler, SIGNAL(volumeChanged(int, int)),
            m_core, SLOT(setVolume(int, int)));

    m_playListModel->readSettings();
    char buf[PATH_MAX + 1];
    QString cwd = QString::fromLocal8Bit(getcwd(buf,PATH_MAX));
    processCommandArgs(args,cwd);
    if (m_startHidden && m_generalHandler->visibilityControl())
        toggleVisibility();
}


MainWindow::~MainWindow()
{
    stop();
}

void MainWindow::play()
{
    disconnect(m_playListModel, SIGNAL(firstAdded()), this, SLOT(play()));
    m_playListModel->doCurrentVisibleRequest();

    if (m_core->state() == Qmmp::Paused)
    {
        pause();
        return;
    }
    //stop();
    if (m_playListModel->count() == 0)
        return;

    m_equalizer->loadPreset(m_playListModel->currentItem()->fileName());
    //m_playListModel->currentItem()->updateTags();
    m_playlist->listWidget()->updateList();
    QString s = m_playListModel->currentItem()->path();
    if (s.isEmpty())
        return;
    if (m_core->play(s))
        m_generalHandler->setTime(0);
    else
    {
        //find out the reason why playback failed
        switch ((int) m_core->state())
        {
        case Qmmp::FatalError:
        {
            stop();
            return; //unrecovable error in output, so abort playing
        }
        case Qmmp::NormalError:
        {
            //error in decoder, so we should try to play next song
            qApp->processEvents();
            if (!m_playListModel->isEmptyQueue())
            {
                m_playListModel->setCurrentToQueued();
            }
            else if (!m_playListModel->next())
            {
                stop();
                display->hideTimeDisplay();
                return;
            }
            m_playlist->update();
            play();
            break;
        }
        }
    }
}

void MainWindow::replay()
{
    stop();
    play();
}

void MainWindow::seek(int pos)
{
    if (!seeking)
        m_core->seek(pos);
}


void MainWindow::forward()
{
    seek(m_core->elapsed() + KEY_OFFSET);
}

void MainWindow::backward()
{
    seek(qMax(qint64(0), m_core->elapsed() - KEY_OFFSET));
}

void MainWindow::setVolume(int volume, int balance)
{
    m_core->setVolume(volume-qMax(balance,0)*volume/100,
                      volume+qMin(balance,0)*volume/100);
}

void MainWindow::pause(void)
{
    m_core->pause();
}

void MainWindow::stop()
{
    //display->setTime(0);
    m_core->stop();
}
void MainWindow::next()
{
    if (!m_playListModel->isEmptyQueue())
    {
        m_playListModel->setCurrentToQueued();
    }
    else if (!m_playListModel->next())
    {
        stop();
        display->hideTimeDisplay();
        return;
    }
    m_playlist->update();
    if (m_core->state() != Qmmp::Stopped)
    {
        stop();
        m_elapsed = 0;
        play();
    }
    else
        display->hideTimeDisplay();
}
void MainWindow::previous()
{
    if (!m_playListModel->previous())
    {
        display->hideTimeDisplay();
        return;
    }

    m_playlist->update();
    if (m_core->state() != Qmmp::Stopped)
    {
        stop();
        play();
    }
    else
        display->hideTimeDisplay();
}

void MainWindow::updateEQ()
{
    int b[10];
    for (int i=0; i<10; ++i)
        b[i] = m_equalizer->gain(i);
    m_core->setEQ(b, m_equalizer->preamp());
    m_core->setEQEnabled(m_equalizer->isEQEnabled());
}

void MainWindow::showState(Qmmp::State state)
{
    switch ((int) state)
    {
    case Qmmp::Playing:
    {
        m_generalHandler->setState(General::Playing);
        /*if (m_playListModel->currentItem())
        {
            SongInfo info = *m_playListModel->currentItem();
            if (info.isEmpty())
                info.setValue(SongInfo::TITLE, m_playlist->currentItem()->text());
            m_generalHandler->setSongInfo(info);
        }*/
        if (m_playlist->listWidget())
            m_playlist->listWidget()->updateList(); //removes progress message from TextScroller
        break;
    }
    case Qmmp::Paused:
    {
        m_generalHandler->setState(General::Paused);
        break;
    }
    case Qmmp::Stopped:
    {
        m_generalHandler->setState(General::Stopped);
        m_playlist->setTime(-1);
        m_titlebar->setTime(-1);
        break;
    }
    }
}
void MainWindow::showMetaData()
{
    qDebug("===== metadata ======");
    qDebug("ARTIST = %s", qPrintable(m_core->metaData(Qmmp::TITLE)));
    qDebug("TITLE = %s", qPrintable(m_core->metaData(Qmmp::ARTIST)));
    qDebug("ALBUM = %s", qPrintable(m_core->metaData(Qmmp::ALBUM)));
    qDebug("COMMENT = %s", qPrintable(m_core->metaData(Qmmp::COMMENT)));
    qDebug("GENRE = %s", qPrintable(m_core->metaData(Qmmp::GENRE)));
    qDebug("YEAR = %s", qPrintable(m_core->metaData(Qmmp::YEAR)));
    qDebug("TRACK = %s", qPrintable(m_core->metaData(Qmmp::TRACK)));
    qDebug("== end of metadata ==");

    if (m_playlist->currentItem())
    {
        SongInfo info;
        info.setValue(SongInfo::TITLE, m_core->metaData(Qmmp::TITLE));
        info.setValue(SongInfo::ARTIST, m_core->metaData(Qmmp::ARTIST));
        info.setValue(SongInfo::ALBUM, m_core->metaData(Qmmp::ALBUM));
        info.setValue(SongInfo::COMMENT, m_core->metaData(Qmmp::COMMENT));
        info.setValue(SongInfo::GENRE, m_core->metaData(Qmmp::GENRE));
        info.setValue(SongInfo::YEAR, m_core->metaData(Qmmp::YEAR).toUInt());
        info.setValue(SongInfo::TRACK, m_core->metaData(Qmmp::TRACK).toUInt());
        //info.setValue(SongInfo::LENGTH, st.tag()->length());
        info.setValue(SongInfo::STREAM, !QFile::exists(m_playlist->currentItem()->path()));
        info.setValue(SongInfo::PATH, m_playlist->currentItem()->path());
        m_generalHandler->setSongInfo(info);
        m_playlist->currentItem()->updateMetaData(m_core->metaData());
        m_playlist->listWidget()->updateList();
    }
}

void MainWindow::changeTitle(const QString &title)
{
    if (m_playlist->currentItem())
        m_playlist->currentItem()->setText(title);
    m_playlist->listWidget()->updateList();
    SongInfo info;
    info.setValue(SongInfo::TITLE, title);
    info.setValue(SongInfo::STREAM, TRUE);
    m_generalHandler->setSongInfo(info);
}

void MainWindow::closeEvent ( QCloseEvent *)
{
    writeSettings();
    m_playlist->close();
    m_equalizer->close();
    QApplication::quit ();
}

void MainWindow::addDir()
{
    FileDialog::popup(this, FileDialog::AddDirs, &m_lastDir,
                      m_playListModel, SLOT(addFileList(const QStringList&)),
                      tr("Choose a directory"));
}

void MainWindow::addFile()
{
    QStringList filters;
    filters << tr("All Supported Bitstreams")+" (" + Decoder::nameFilters().join (" ") +")";
    filters << Decoder::filters();
    FileDialog::popup(this, FileDialog::AddDirsFiles, &m_lastDir,
                      m_playListModel, SLOT(addFileList(const QStringList&)),
                      tr("Select one or more files to open"), filters.join(";;"));
}

void MainWindow::clear()
{
    m_playListModel->clear();
}

void MainWindow::startSeek()
{
    seeking = TRUE;
}

void MainWindow::endSeek()
{
    seeking = FALSE;
}

void MainWindow::changeEvent (QEvent * event)
{
    if (event->type() == QEvent::ActivationChange)
    {
        m_titlebar->setActive(isActiveWindow());
    }
}

void MainWindow::readSettings()
{
    QSettings settings(QDir::homePath()+"/.qmmp/qmmprc", QSettings::IniFormat);
    if (!m_update)
    {
        settings.beginGroup("MainWindow");
        //geometry
        move(settings.value("pos", QPoint(100, 100)).toPoint());
        //last directory
        m_lastDir = settings.value("last_dir","/").toString();
        m_startHidden = settings.value("start_hidden", FALSE).toBool();
        settings.endGroup();
        show();

        //visibility
        m_playlist->setVisible(settings.value("Playlist/visible",TRUE).toBool());
        m_equalizer->setVisible(settings.value("Equalizer/visible",TRUE).toBool());
        bool val = settings.value("Playlist/repeatable",FALSE).toBool();

        // Repeat/Shuffle
        m_playListModel->prepareForRepeatablePlaying(val);
        display->setIsRepeatable(val);
        val = settings.value("Playlist/shuffle",FALSE).toBool();
        display->setIsShuffle(val);
        m_playListModel->prepareForShufflePlaying(val);

        // Playlist name
        m_playlistName = settings.value("Playlist/playlist_name","Default").toString();

        m_update = TRUE;
    }
    m_hideOnClose = settings.value("MainWindow/hide_on_close", FALSE).toBool();
}

void MainWindow::writeSettings()
{
    QSettings settings(QDir::homePath()+"/.qmmp/qmmprc", QSettings::IniFormat);
    settings.beginGroup("MainWindow");
    //geometry
    settings.setValue("pos", this->pos());
    //last directory
    settings.setValue("last_dir",m_lastDir);
    settings.endGroup();

    // Repeat/Shuffle
    settings.beginGroup("Playlist");
    settings.setValue("repeatable",display->isRepeatable());
    settings.setValue("shuffle",display->isShuffle());

    // Playlist name
    settings.setValue("playlist_name",m_playlistName);
    settings.endGroup();
}

void MainWindow::showSettings()
{
    m_confDialog = new ConfigDialog(this);
    if (m_confDialog->exec() == QDialog::Accepted)
    {
        readSettings();
        m_playlist->readSettings();
        TextScroller::getPointer()->readSettings();
        //m_core->updateConfig();
        m_visMenu->updateActions();
    }
    delete m_confDialog;
}

void MainWindow::toggleVisibility()
{
    if (isHidden())
    {
        show();
        m_playlist->setVisible(display->isPlaylistVisible());
        m_equalizer->setVisible(display->isEqualizerVisible());
        if (isMinimized())
        {
            if (isMaximized())
                showMaximized();
            else
                showNormal();
        }
        raise();
        activateWindow();
    }
    else
    {
        hide();
        if (m_playlist->isVisible())
            m_playlist->hide();
        if (m_equalizer->isVisible())
            m_equalizer->hide();
    }
}

void MainWindow::createActions()
{
    m_mainMenu = new QMenu(this);
    m_mainMenu->addAction(tr("&Play"),this, SLOT(play()), tr("X"));
    m_mainMenu->addAction(tr("&Pause"),this, SLOT(pause()), tr("C"));
    m_mainMenu->addAction(tr("&Stop"),this, SLOT(stop()), tr("V"));
    m_mainMenu->addAction(tr("&Previous"),this, SLOT(previous()), tr("Z"));
    m_mainMenu->addAction(tr("&Next"),this, SLOT(next()), tr("B"));
    m_mainMenu->addAction(tr("&Play/Pause"),this, SLOT(playPause()), tr("Space"));
    m_mainMenu->addSeparator();
    m_mainMenu->addAction(tr("&Jump To File"),this, SLOT(jumpToFile()), tr("J"));
    m_mainMenu->addSeparator();
    m_visMenu = new VisualMenu(this);
    m_mainMenu->addMenu(m_visMenu);

    m_mainMenu->addSeparator();
    m_mainMenu->addAction(tr("&Settings"), this, SLOT(showSettings()), tr("Ctrl+P"));
    m_mainMenu->addSeparator();
    m_mainMenu->addAction(tr("&About"), this, SLOT(about()));
    m_mainMenu->addAction(tr("&About Qt"), qApp, SLOT(aboutQt()));
    Dock::getPointer()->addActions(m_mainMenu->actions());
    m_mainMenu->addSeparator();
    m_mainMenu->addAction(tr("&Exit"),this, SLOT(close ()), tr("Ctrl+Q"));

    QAction* forward = new QAction(this);
    forward->setShortcut(QKeySequence(Qt::Key_Right));
    connect(forward,SIGNAL(triggered(bool)),this,SLOT(forward()));
    QAction* backward = new QAction(this);
    backward->setShortcut(QKeySequence(Qt::Key_Left));
    connect(backward,SIGNAL(triggered(bool)),this,SLOT(backward()));

    Dock::getPointer()->addActions( QList<QAction*>() << forward << backward );
    Dock::getPointer()->addActions(m_mainMenu->actions());
}


void MainWindow::about()
{
    AboutDialog dlg(this);
    dlg.exec();
}

QMenu* MainWindow::menu()
{
    return m_mainMenu;
}

void MainWindow::newPlaylist()
{
    m_playListModel->clear();
    m_playlistName = tr("Default");
}

void MainWindow::loadPlaylist()
{
    QStringList l;
    QList<PlaylistFormat*> p_list = PlaylistParser::instance()->formats();
    if (!p_list.isEmpty())
    {
        foreach(PlaylistFormat* fmt,p_list)
        l << fmt->getExtensions();

        QString mask = tr("Playlist Files")+" (" + l.join(" *.").prepend("*.") + ")";
        //TODO use nonmodal dialog and multiplier playlists
        QString f_name = FileDialog::getOpenFileName(this,tr("Open Playlist"),m_lastDir,mask);
        if (!f_name.isEmpty())
        {
            m_playListModel->clear();
            m_playListModel->loadPlaylist(f_name);
            m_playlistName = QFileInfo(f_name).baseName();
            m_lastDir = QFileInfo(f_name).absoluteDir().path();
        }
    }
    else
    {
        qWarning("Error: There is no registered playlist parsers");
    }
}

void MainWindow::savePlaylist()
{
    QStringList l;
    QList<PlaylistFormat*> p_list = PlaylistParser::instance()->formats();
    if (!p_list.isEmpty())
    {
        foreach(PlaylistFormat* fmt,p_list)
        l << fmt->getExtensions();

        QString mask = tr("Playlist Files")+" (" + l.join(" *.").prepend("*.") + ")";
        QString f_name = FileDialog::getSaveFileName(this, tr("Save Playlist"),m_lastDir + "/" +
                         m_playlistName + "." + l[0],mask);

        if (!f_name.isEmpty())
        {
            m_playListModel->savePlaylist(f_name);
            m_playlistName = QFileInfo(f_name).baseName();
            m_lastDir = QFileInfo(f_name).absoluteDir().path();
        }
    }
    else
        qWarning("Error: There is no registered playlist parsers");
}

void MainWindow::setFileList(const QStringList & l)
{
    connect(m_playListModel, SIGNAL(firstAdded()), SLOT(play()));
    if (!m_playListModel->setFileList(l))
    {
        disconnect(m_playListModel, SIGNAL(firstAdded()), this, SLOT(play()));
        addFile();
    }
}

void MainWindow::playPause()
{
    if (m_core->state() == Qmmp::Playing)
        pause();
    else
        play();
}

bool MainWindow::processCommandArgs(const QStringList &slist,const QString& cwd)
{
    if (slist.count() > 0)
    {
        QString str = slist[0];
        if (str.startsWith("--")) // is it a command?
        {
            if (CommandLineManager::hasOption(str))
                m_generalHandler->executeCommand(str);
            else if (m_option_manager->identify(str))
                m_option_manager->executeCommand(str,this);
            else
                return FALSE;
        }
        else// maybe it is a list of files or dirs
        {
            QStringList full_path_list;
            foreach(QString s,slist)
            {
                qWarning(qPrintable(cwd + "/" + s));
                if (s.left(1) == "/")   //is it absolute path?
                    full_path_list << s;
                else
                    full_path_list << cwd + "/" + s;
            }
            setFileList(full_path_list);
        }
    }
    return TRUE;
}

void MainWindow::jumpToFile()
{
    if (m_jumpDialog->isHidden())
    {
        m_jumpDialog->show();
        m_jumpDialog->refresh();
    }
}

void MainWindow::handleCloseRequest()
{
    if (m_hideOnClose && m_generalHandler->visibilityControl())
        toggleVisibility();
    else
        QApplication::closeAllWindows();
}

void MainWindow::addUrl( )
{
    AddUrlDialog::popup(this,m_playListModel);
}

SoundCore * MainWindow::soundCore() const
{
    return m_core;
}

MainDisplay * MainWindow::mainDisplay() const
{
    return display;
}

void MainWindow::keyPressEvent(QKeyEvent *ke)
{
    QKeyEvent event = QKeyEvent(ke->type(), ke->key(),
                                ke->modifiers(), ke->text(),ke->isAutoRepeat(), ke->count());
    QApplication::sendEvent(m_playlist,&event);
}
