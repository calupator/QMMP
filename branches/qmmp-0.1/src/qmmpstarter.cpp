/***************************************************************************
 *   Copyright (C) 2006 by Ilya Kotov                                      *
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

#include <QApplication>
#include <QNetworkInterface>
#include <QUdpSocket>


#include <unistd.h>

#include "mainwindow.h"
#include "version.h"
#include "qmmpstarter.h"

QMMPStarter::QMMPStarter(int argc,char ** argv,QObject* parent) : QObject(parent),mw(NULL)
{	
	QStringList tmp;
	for(int i = 1;i < argc;i++)
		tmp << QString::fromLocal8Bit(argv[i]);

	argString = tmp.join("\n");
	
	if(argString == "--help")
	{
		printUsage();
		exit(0);
	}
	else if(argString == "--version")
	{
		printVersion();
		exit(0);
	}
	
	if(argString.startsWith("--") &&  // command?
		  argString != "--play" && 
		  argString != "--previous" && 
		  argString != "--next" && 
		  argString != "--stop" && 
		  argString != "--pause" &&
		  argString != "--play-pause" 
	  )
	{
		qFatal("QMMP: Unknown command...");
		exit(1);
	}
        
    m_udpSocket = new QUdpSocket(this);
    connect(m_udpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    
    if(m_udpSocket->bind(QHostAddress::LocalHost,LISTEN_PORT_BASE + getuid()))
    {
        qDebug("binded");
        connect(m_udpSocket, SIGNAL(readyRead()),this, SLOT(readCommand()));
        QStringList arg_l = argString.split("\n", QString::SkipEmptyParts);
        mw = new MainWindow(arg_l,0);
    }
    else writeCommand();

}

void QMMPStarter::displayError(QAbstractSocket::SocketError socketError)
{
    
	switch (socketError) 
	{
		case QAbstractSocket::RemoteHostClosedError:
			break;
		case QAbstractSocket::HostNotFoundError:
			qWarning("The host was not found");
			break;
		case QAbstractSocket::ConnectionRefusedError:
			qWarning("The connection was refused by the peer. ");
			break;
		default:
			qWarning("%s:",qPrintable(m_udpSocket->errorString()));
            qWarning("It seems that another version of QMMP is already running ...\n");
	}
}

QMMPStarter::~ QMMPStarter()
{
	if(mw) delete mw;
}

void QMMPStarter::writeCommand()
{
	if(!argString.isEmpty())
	{
        QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        foreach(QNetworkInterface ni, interfaces)
        {
         //   qWarning(qPrintable(ni.name()));
            if(ni.flags() & QNetworkInterface::IsLoopBack && !(ni.flags() & QNetworkInterface::IsUp))
                qWarning("Warning: Loopback interface on this machine is disabled, command will be not executed...");
        }

		char buf[PATH_MAX + 1];
		QString workingDir = QString(getcwd(buf,PATH_MAX)) + "\n";
		
		QByteArray barray;
		barray.append(workingDir);
		barray.append(argString);
        m_udpSocket->writeDatagram ( barray,QHostAddress::LocalHost,LISTEN_PORT_BASE + getuid());
	}
	else
	{
		printUsage();
	}
	
	m_udpSocket->close();
    exit(0);
}

void QMMPStarter::readCommand()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray inputArray;
        inputArray.resize(m_udpSocket->pendingDatagramSize());
        m_udpSocket->readDatagram(inputArray.data(), inputArray.size());

        QStringList slist = QString(inputArray).split("\n",QString::SkipEmptyParts);
        QString cwd = slist.takeAt(0);
        if(mw)
        {
            mw->processCommandArgs(slist,cwd);
        }
    }
}

void QMMPStarter::printUsage()
{
	qWarning(
			"Usage: qmmp [options] [files] \n"
			"Options:\n"
			"--------\n"
			"--help              Display this text and exit.\n"
			"--previous          Skip backwards in playlist\n"
			"--play              Start playing current playlist\n"
			"--pause             Pause current song\n"
			"--play-pause        Pause if playing, play otherwise\n"
			"--stop              Stop current song\n"
			"--next              Skip forward in playlist\n"
			"--version           Print version number and exit.\n\n"
			"Ideas, patches, bugreports send to forkotov02@hotmail.ru\n"
			  );
}

void QMMPStarter::printVersion()
{
	qWarning("QMMP version:  %s",QMMP_STR_VERSION);
}

