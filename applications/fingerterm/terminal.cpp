/*
    Copyright 2011-2012 Heikki Holstila <heikki.holstila@gmail.com>

    This file is part of FingerTerm.

    FingerTerm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    FingerTerm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FingerTerm.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QGuiApplication>
#include <QClipboard>
#include <QDebug>

#include "terminal.h"
#include "ptyiface.h"
#include "textrender.h"
#include "util.h"

static bool charIsHexDigit(QChar ch)
{
    if (ch.isDigit()) // 0-9
        return true;
    else if (ch.toLatin1() >= 65 && ch.toLatin1() <= 70) // A-F
        return true;
    else if (ch.toLatin1() >= 97 && ch.toLatin1() <= 102) // a-f
        return true;

    return false;
}

Terminal::Terminal(QObject *parent) :
    QObject(parent), iPtyIFace(0), iWindow(0), iUtil(0),
    iTermSize(0,0), iEmitCursorChangeSignal(true),
    iShowCursor(true), iUseAltScreenBuffer(false), iAppCursorKeys(false)
{
    zeroChar.c = ' ';
    zeroChar.bgColor = defaultBgColor;
    zeroChar.fgColor = defaultFgColor;
    zeroChar.attrib = 0;

    escape = -1;

    iTermAttribs.currentFgColor = defaultFgColor;
    iTermAttribs.currentBgColor = defaultBgColor;
    iTermAttribs.currentAttrib = 0;
    iTermAttribs.cursorPos = QPoint(0,0);
    iMarginBottom = 0;
    iMarginTop = 0;

    resetBackBufferScrollPos();

    iTermAttribs_saved = iTermAttribs;
    iTermAttribs_saved_alt = iTermAttribs;

    resetTerminal();
}

void Terminal::setPtyIFace(PtyIFace *pty)
{
    iPtyIFace = pty;
    if(!pty) {
        qDebug() << "warning: null pty iface";
    }
}

void Terminal::setCursorPos(QPoint pos)
{
    if( iTermAttribs.cursorPos != pos ) {
        int tlimit = 1;
        int blimit = iTermSize.height();
        if(iTermAttribs.originMode) {
            tlimit = iMarginTop;
            blimit = iMarginBottom;
        }

        if(pos.x() < 1)
            pos.setX(1);
        if(pos.x() > iTermSize.width()+1)
            pos.setX(iTermSize.width()+1);
        if(pos.y() < tlimit)
            pos.setY(tlimit);
        if(pos.y() > blimit)
            pos.setY(blimit);

        iTermAttribs.cursorPos=pos;
        if(iEmitCursorChangeSignal)
            emit cursorPosChanged(pos);
    }
}

QPoint Terminal::cursorPos()
{
    return iTermAttribs.cursorPos;
}

bool Terminal::showCursor()
{
    if(iBackBufferScrollPos != 0)
        return false;

    return iShowCursor;
}

QList<QList<TermChar> >& Terminal::buffer()
{
    if(iUseAltScreenBuffer)
        return iAltBuffer;

    return iBuffer;
}

void Terminal::setTermSize(QSize size)
{
    if( iTermSize != size ) {
        iMarginTop = 1;
        iMarginBottom = size.height();
        iTermSize=size;

        resetTabs();

        emit termSizeChanged(size.height(), size.width());
    }
}

void Terminal::putString(QString str, bool unEscape)
{
    if (unEscape) {
        str.replace("\\r", "\r");
        str.replace("\\n", "\n");
        str.replace("\\e", QChar(ch_ESC));
        str.replace("\\b", "\b");
        str.replace("\\t", "\t");

        //hex
        while(str.indexOf("\\x") != -1) {
            int i = str.indexOf("\\x")+2;
            QString num;
            while(num.length() < 2 && str.length()>i && charIsHexDigit(str.at(i))) {
                num.append(str.at(i));
                i++;
            }
            str.remove(i-2-num.length(), num.length()+2);
            bool ok;
            str.insert(i-2-num.length(), QChar(num.toInt(&ok,16)));
        }
        //octal
        while(str.indexOf("\\0") != -1) {
            int i = str.indexOf("\\0")+2;
            QString num;
            while(num.length() < 3 && str.length()>i &&
                  (str.at(i).toLatin1() >= 48 && str.at(i).toLatin1() <= 55)) { //accept only 0-7
                num.append(str.at(i));
                i++;
            }
            str.remove(i-2-num.length(), num.length()+2);
            bool ok;
            str.insert(i-2-num.length(), QChar(num.toInt(&ok,8)));
        }
    }

    if(iPtyIFace)
        iPtyIFace->writeTerm(str);
}

void Terminal::keyPress(int key, int modifiers, const QString& text)
{
    QString toWrite;

    resetBackBufferScrollPos();

    if (key > 0xFFFF) {
        int modcode = (modifiers & Qt::ShiftModifier ? 1 : 0) | 
                      (modifiers & Qt::AltModifier ? 2 : 0) |
                      (modifiers & Qt::ControlModifier ? 4 : 0);

        if (modcode == 0) {
            QString fmt;
            char cursorModif='[';
            if(iAppCursorKeys)
                cursorModif = 'O';

            if( key==Qt::Key_Up ) fmt = QString("%2%1A").arg(cursorModif);
            if( key==Qt::Key_Down ) fmt = QString("%2%1B").arg(cursorModif);
            if( key==Qt::Key_Right ) fmt = QString("%2%1C").arg(cursorModif);
            if( key==Qt::Key_Left ) fmt = QString("%2%1D").arg(cursorModif);
            if( key==Qt::Key_PageUp ) fmt = "%1[5~";
            if( key==Qt::Key_PageDown ) fmt = "%1[6~";
            if( key==Qt::Key_Home ) fmt = "%1OH";
            if( key==Qt::Key_End ) fmt = "%1OF";
            if( key==Qt::Key_Insert ) fmt = "%1[2~";
            if( key==Qt::Key_Delete ) fmt = "%1[3~";

            if( key==Qt::Key_F1 ) fmt = "%1OP";
            if( key==Qt::Key_F2 ) fmt = "%1OQ";
            if( key==Qt::Key_F3 ) fmt = "%1OR";
            if( key==Qt::Key_F4 ) fmt = "%1OS";
            if( key==Qt::Key_F5 ) fmt = "%1[15~";
            if( key==Qt::Key_F6 ) fmt = "%1[17~";
            if( key==Qt::Key_F7 ) fmt = "%1[18~";
            if( key==Qt::Key_F8 ) fmt = "%1[19~";
            if( key==Qt::Key_F9 ) fmt = "%1[20~";
            if( key==Qt::Key_F10 ) fmt = "%1[21~";
            if( key==Qt::Key_F11 ) fmt = "%1[23~";
            if( key==Qt::Key_F12 ) fmt = "%1[24~";
            
            if (!fmt.isEmpty())
                toWrite += fmt.arg(ch_ESC);

        } else {
            QString fmt;
            char modChar = '1' + modcode;

            if( key==Qt::Key_Up ) fmt = "%1[1;%2A";
            if( key==Qt::Key_Down ) fmt = "%1[1;%2B";
            if( key==Qt::Key_Right ) fmt = "%1[1;%2C";
            if( key==Qt::Key_Left ) fmt = "%1[1;%2D";
            if( key==Qt::Key_PageUp ) fmt = "%1[5;%2~";
            if( key==Qt::Key_PageDown ) fmt = "%1[6;%2~";
            if( key==Qt::Key_Home ) fmt = "%1[1;%2H";
            if( key==Qt::Key_End ) fmt = "%1[1;%2F";
            if( key==Qt::Key_Insert ) fmt = "%1[2;%2~";
            if( key==Qt::Key_Delete ) fmt = "%1[3;%2~";

            if( key==Qt::Key_F1 ) fmt = "%1[1;%2P";
            if( key==Qt::Key_F2 ) fmt = "%1[1;%2Q";
            if( key==Qt::Key_F3 ) fmt = "%1[1;%2R";
            if( key==Qt::Key_F4 ) fmt = "%1[1;%2S";
            if( key==Qt::Key_F5 ) fmt = "%1[15;%2~";
            if( key==Qt::Key_F6 ) fmt = "%1[17;%2~";
            if( key==Qt::Key_F7 ) fmt = "%1[18;%2~";
            if( key==Qt::Key_F8 ) fmt = "%1[19;%2~";
            if( key==Qt::Key_F9 ) fmt = "%1[20;%2~";
            if( key==Qt::Key_F10 ) fmt = "%1[21;%2~";
            if( key==Qt::Key_F11 ) fmt = "%1[23;%2~";
            if( key==Qt::Key_F12 ) fmt = "%1[24;%2~";

            if (!fmt.isEmpty())
                toWrite += fmt.arg(ch_ESC).arg(modChar);

        }

        if( key==Qt::Key_Enter || key==Qt::Key_Return ) {
            if ( (modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) ==
                (Qt::ShiftModifier | Qt::ControlModifier) )
                toWrite += QChar(0x9E);
            else if (modifiers & Qt::ControlModifier)
                toWrite += QChar(0x1E); // ^^
            else if (modifiers & Qt::ShiftModifier)
                toWrite += "\n";
            else if(iNewLineMode)
                toWrite += "\r\n";
            else
                toWrite += "\r";
        }
        if( key==Qt::Key_Backspace ) {
            if ( (modifiers & (Qt::ShiftModifier | Qt::ControlModifier)) ==
                (Qt::ShiftModifier | Qt::ControlModifier) )
                toWrite += QChar(0x9F);
            else if (modifiers & Qt::ControlModifier)
                toWrite += QChar(0x1F); // ^_
            else
                toWrite += "\x7F";
        }
        if( key==Qt::Key_Tab || key==Qt::Key_Backtab ) {
            if ( key == Qt::Key_Backtab ) modifiers |= Qt::ShiftModifier;
            if (modifiers & Qt::ControlModifier) {
                char modChar = '5' + (modifiers & Qt::ShiftModifier ? 1 : 0);
                toWrite += QString("%1[1;%2I").arg(ch_ESC).arg(modChar);
            } else if (modifiers & Qt::ShiftModifier) {
                toWrite += QString("%1[Z").arg(ch_ESC);
            } else {
                toWrite += "\t";
            }
        }

        if( key==Qt::Key_Escape ) {
            if (modifiers & Qt::ShiftModifier)
                toWrite += QChar(0x9B);
            else
                toWrite += QString(1,ch_ESC);
        }

        if (!toWrite.isEmpty()) {
            if(iPtyIFace)
                iPtyIFace->writeTerm(toWrite);
        } else {
            qDebug() << "unknown special key: " << key;
        }
        return;
    }

    QChar c(key);

    if (c.isLetter()) {
        c = ((modifiers & Qt::ShiftModifier) != 0) ? c.toUpper() : c.toLower();
    }

    if((modifiers & Qt::AltModifier) != 0) {
        toWrite.append(ch_ESC);
    }

    if ((modifiers & Qt::ControlModifier) != 0) {
        char asciiVal = c.toUpper().toLatin1();

        if (asciiVal >= 0x41 && asciiVal <= 0x5f) {
            // Turn uppercase characters into their control code equivalent
            toWrite.append(asciiVal - 0x40);
        } else {
            qWarning() << "Ctrl+" << c << " does not translate into a control code";
        }
    } else {
        if (text.isEmpty()) {
            toWrite.append(c);
        } else {
            toWrite.append(text);
        }
    }

    if (iPtyIFace) {
        iPtyIFace->writeTerm(toWrite);
    }
    return;
}

void Terminal::insertInBuffer(const QString& chars)
{
    if(iTermSize.isNull()) {
        qDebug() << "null size terminal";
        return;
    }

    iEmitCursorChangeSignal = false;

    for(int i=0; i<chars.size(); i++) {
        QChar ch = chars.at(i);
        if(ch.toLatin1()=='\n' || ch.toLatin1()==11 || ch.toLatin1()==12) {  // line feed, vertical tab or form feed
            if(cursorPos().y()==iMarginBottom) {
                scrollFwd(1);
                if(iNewLineMode)
                    setCursorPos(QPoint(1,cursorPos().y()));
            }
            else if(cursorPos().x() <= columns()) // ignore newline after <termwidth> cols (terminfo: xenl)
            {
                if(iNewLineMode)
                    setCursorPos(QPoint(1,cursorPos().y()+1));
                else
                    setCursorPos(QPoint(cursorPos().x(), cursorPos().y()+1));
            }
        }
        else if(ch.toLatin1()=='\r') {  // carriage return
            setCursorPos(QPoint(1,cursorPos().y()));
        }
        else if(ch.toLatin1()=='\b' || ch.toLatin1()==127) {  //backspace & del (only move cursor, don't erase)
            setCursorPos(QPoint(cursorPos().x()-1,cursorPos().y()));
        }
        else if(ch.toLatin1()=='\a') {  // BEL
            if(escape==']') {  // BEL also ends OSC sequence
                escape=-1;
                oscSequence(oscSeq);
                oscSeq.clear();
            } else {
                iUtil->bellAlert();
            }
        }
        else if(ch.toLatin1()=='\t') {  //tab
            if(cursorPos().y() <= iTabStops.size()) {
                for(int i=0; i<iTabStops[cursorPos().y()-1].count(); i++) {
                    if(iTabStops[cursorPos().y()-1][i] > cursorPos().x()) {
                        setCursorPos(QPoint( iTabStops[cursorPos().y()-1][i], cursorPos().y() ));
                        break;
                    }
                }
            }
        }
        else if(ch.toLatin1()==14 || ch.toLatin1()==15) {  //SI and SO, related to character set... ignore
        }
        else {
            if( escape>=0 ) {
                if( escape==0 && (ch.toLatin1()=='[') ) {
                    escape='['; //ansi sequence
                    escSeq += ch;
                }
                else if( escape==0 && (ch.toLatin1()==']') ) {
                    escape=']'; //osc sequence
                    oscSeq += ch;
                }
                else if( escape==0 && multiCharEscapes.contains(ch.toLatin1())) {
                    escape = ch.toLatin1();
                    escSeq += ch;
                }
                else if( escape==0 && ch.toLatin1()=='\\' ) {  // ESC\ also ends OSC sequence
                    escape=-1;
                    oscSequence(oscSeq);
                    oscSeq.clear();
                }
                else if (ch.toLatin1()==ch_ESC) {
                    escape = 0;
                }
                else if( escape=='[' || multiCharEscapes.contains(escape) ) {
                    escSeq += ch;
                }
                else if( escape==']' ) {
                    oscSeq += ch;
                }
                else if( multiCharEscapes.contains(escape) ) {
                    escSeq += ch;
                }
                else {
                    escControlChar(QByteArray(1,ch.toLatin1()));
                    escape=-1;
                }

                if( escape=='[' && ch.toLatin1() >= 64 && ch.toLatin1() <= 126 && ch.toLatin1() != '[' ) {
                    ansiSequence(escSeq);
                    escape=-1;
                    escSeq.clear();
                }
                if( multiCharEscapes.contains(escape) && escSeq.length()>=2 ) {
                    escControlChar(escSeq);
                    escape=-1;
                    escSeq.clear();
                }
            } else {
                if (ch.isPrint())
                    insertAtCursor(ch, !iReplaceMode);
                else if (ch.toLatin1()==ch_ESC)
                    escape=0;
                else if (ch.toLatin1() != 0)
                    qDebug() << "unprintable char" << int(ch.toLatin1());
            }
        }
    }

    iEmitCursorChangeSignal = true;
    emit displayBufferChanged();
}

void Terminal::insertAtCursor(QChar c, bool overwriteMode, bool advanceCursor)
{
    if(cursorPos().x() > iTermSize.width() && advanceCursor) {
        if(iTermAttribs.wrapAroundMode) {
            if(cursorPos().y()>=iMarginBottom) {
                scrollFwd(1);
                setCursorPos(QPoint(1, cursorPos().y()));
            } else {
                setCursorPos(QPoint(1, cursorPos().y()+1));
            }
        } else {
            setCursorPos(QPoint(iTermSize.width(), cursorPos().y()));
        }
    }

    while(currentLine().size() < cursorPos().x() )
        currentLine().append(zeroChar);

    if(!overwriteMode)
        currentLine().insert(cursorPos().x()-1,zeroChar);

    currentLine()[cursorPos().x()-1].c = c;
    currentLine()[cursorPos().x()-1].fgColor = iTermAttribs.currentFgColor;
    currentLine()[cursorPos().x()-1].bgColor = iTermAttribs.currentBgColor;
    currentLine()[cursorPos().x()-1].attrib = iTermAttribs.currentAttrib;

    if (advanceCursor) {
        setCursorPos(QPoint(cursorPos().x()+1,cursorPos().y()));
    }
}

void Terminal::deleteAt(QPoint pos)
{
    clearAt(pos);
    buffer()[pos.y()-1].removeAt(pos.x()-1);
}

void Terminal::clearAt(QPoint pos)
{
    if(pos.y() <= 0 || pos.y()-1 > buffer().size() ||
            pos.x() <= 0 || pos.x()-1 > buffer()[pos.y()-1].size())
    {
        qDebug() << "warning: trying to clear char out of bounds";
        return;
    }

    // just in case...
    while(buffer().size() < pos.y())
        buffer().append(QList<TermChar>());
    while(buffer()[pos.y()-1].size() < pos.x() )
        buffer()[pos.y()-1].append(zeroChar);

    buffer()[pos.y()-1][pos.x()-1] = zeroChar;
}

void Terminal::eraseLineAtCursor(int from, int to)
{
    if(from==-1 && to==-1) {
        currentLine().clear();
        return;
    }
    if(from < 1)
        from=1;
    from--;

    if(to < 1 || to > currentLine().size())
        to=currentLine().size();
    to--;

    if(from>to)
        return;

    for(int i=from; i<=to; i++) {
        currentLine()[i] = zeroChar;
    }
}

void Terminal::clearAll(bool wholeBuffer)
{
    clearSelection();
    if(wholeBuffer) {
        backBuffer().clear();
        resetBackBufferScrollPos();
    }
    buffer().clear();
    setCursorPos(QPoint(1,1));
}


void Terminal::ansiSequence(const QString& seq)
{
    if(seq.length() <= 1 || seq.at(0)!='[')
        return;

    QChar cmdChar = seq.at(seq.length()-1);
    QString extra;
    QList<int> params;

    int x=1;
    while(x<seq.length()-1 && !QChar(seq.at(x)).isNumber())
        x++;

    QList<QString> tmp = seq.mid(x,seq.length()-x-1).split(';');
    foreach(QString b, tmp) {
        bool ok=false;
        int t = b.toInt(&ok);
        if(ok) {
            params.append(t);
        }
    }
    if(x>1)
        extra = seq.mid(1,x-1);

    bool unhandled = false;

    switch(cmdChar.toLatin1())
    {
    case 'A': //cursor up
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        setCursorPos(QPoint( cursorPos().x(), qMax(iMarginTop, cursorPos().y()-params.at(0)) ));
        break;
    case 'B': //cursor down
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        setCursorPos(QPoint( cursorPos().x(), qMin(iMarginBottom, cursorPos().y()+params.at(0)) ));
        break;
    case 'C': //cursor fwd
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        setCursorPos(QPoint( qMin(iTermSize.width(),cursorPos().x()+params.at(0)), cursorPos().y() ));
        break;
    case 'D': //cursor back
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        setCursorPos(QPoint( qMax(1,cursorPos().x()-params.at(0)), cursorPos().y() ));
        break;
    case 'E': //cursor next line
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        setCursorPos(QPoint( 1, qMin(iMarginBottom, cursorPos().y()+params.at(0)) ));
        break;
    case 'F': //cursor prev line
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        setCursorPos(QPoint( 1, qMax(iMarginTop, cursorPos().y()-params.at(0)) ));
        break;
    case 'G': //go to column
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        setCursorPos(QPoint( params.at(0), cursorPos().y() ));
        break;
    case 'H': //cursor pos
    case 'f': //cursor pos
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        while(params.count()<2)
            params.append(1);
        if (iTermAttribs.originMode)
            setCursorPos(QPoint( params.at(1), params.at(0)+iMarginTop-1 ));
        else
            setCursorPos(QPoint( params.at(1), params.at(0) ));
        break;
    case 'J': //erase data
        if(!extra.isEmpty() && extra!="?") {
            unhandled=true;
            break;
        }
        if(params.count()>=1 && params.at(0)==1) {
            eraseLineAtCursor(1,cursorPos().x());
            for(int i=0; i<cursorPos().y()-1; i++) {
                buffer()[i].clear();
            }
        } else if(params.count()>=1 && params.at(0)==2) {
            clearAll();
        } else {
            eraseLineAtCursor(cursorPos().x());
            for(int i=cursorPos().y(); i<buffer().size(); i++)
                buffer()[i].clear();
        }
        break;
    case 'K': //erase in line
        if(!extra.isEmpty() && extra!="?") {
            unhandled=true;
            break;
        }
        if(params.count()>=1 && params.at(0)==1) {
            eraseLineAtCursor(1,cursorPos().x());
        }
        else if(params.count()>=1 && params.at(0)==2) {
            currentLine().clear();
        } else {
            eraseLineAtCursor(cursorPos().x());
        }
        break;

    case 'L':  // insert lines
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(cursorPos().y() < iMarginTop || cursorPos().y() > iMarginBottom)
            break;
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        if(params.at(0) > iMarginBottom-cursorPos().y())
            scrollBack(iMarginBottom-cursorPos().y(), cursorPos().y());
        else
            scrollBack(params.at(0), cursorPos().y());
        setCursorPos(QPoint(1,cursorPos().y()));
        break;
    case 'M':  // delete lines
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(cursorPos().y() < iMarginTop || cursorPos().y() > iMarginBottom)
            break;
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        if(params.at(0) > iMarginBottom-cursorPos().y())
            scrollFwd(iMarginBottom-cursorPos().y(), cursorPos().y());
        else
            scrollFwd(params.at(0), cursorPos().y());
        setCursorPos(QPoint(1,cursorPos().y()));
        break;

    case 'P': // delete characters
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        for(int i=0; i<params.at(0); i++)
            deleteAt(cursorPos());
        break;
    case '@': // insert blank characters
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0] = 1;
        for(int i=1; i<=params.at(0); i++)
            insertAtCursor(zeroChar.c, false, false);
        break;

    case 'S':  // scroll up n lines
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        scrollFwd(params.at(0));
        break;
    case 'T':  // scroll down n lines
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        scrollBack(params.at(0));
        break;

    case 'c': // vt100 identification
        if(params.count()==0)
            params.append(0);
        if(params.count()==1 && params.at(0)==0) {
            QString toWrite = QString("%1[?1;2c").arg(ch_ESC).toLatin1();
            if(iPtyIFace)
                iPtyIFace->writeTerm(toWrite);
        } else unhandled=true;
        break;

    case 'd': //go to row
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count()<1)
            params.append(1);
        if(params.at(0)==0)
            params[0]=1;
        setCursorPos(QPoint( cursorPos().x(), params.at(0) ));
        break;

    case 'g': //tab stop manipulation
        if(params.count()==0)
            params.append(0);
        if(params.at(0)==0 && extra=="") {  //clear tab at current position
            if(cursorPos().y() <= iTabStops.size()) {
                int idx = iTabStops[cursorPos().y()-1].indexOf(cursorPos().x());
                if(idx != -1)
                    iTabStops[cursorPos().y()-1].removeAt(idx);
            }
        }
        else if(params.at(0)==3 && extra=="") {  //clear all tabs
            iTabStops.clear();
        }
        break;

    case 'n':
        if(params.count()>=1 && params.at(0)==6 && extra=="") {  // write cursor pos
            QString toWrite = QString("%1[%2;%3R").arg(ch_ESC).arg(cursorPos().y()).arg(cursorPos().x()).toLatin1();
            if(iPtyIFace)
                iPtyIFace->writeTerm(toWrite);
        } else unhandled=true;
        break;

    case 'p':
        if(extra=="!") {  // reset terminal
            resetTerminal();
        } else unhandled=true;
        break;

    case 's': //save cursor
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        iTermAttribs_saved = iTermAttribs;
        break;
    case 'u': //restore cursor
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        iTermAttribs = iTermAttribs_saved;
        break;

    case 'm': //graphics mode
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count() > 0) {
            // xterm 256-colour support
            if(params.count() > 1 && (params[0] == 38 || params[0] == 48)) {
                if(params.count() > 2 && params[1] == 5 &&
                   params[2] >= 0 && params[2] <= 255) {
                    if(params[0] == 38)
                        iTermAttribs.currentFgColor = params[2];
                    else
                        iTermAttribs.currentBgColor = params[2];
                }
                // TODO: 2;r;g;b for 24-bit colour support (Konsole etc)
                break;
            }

            if(params.contains(0)) {
                iTermAttribs.currentFgColor = defaultFgColor;
                iTermAttribs.currentBgColor = defaultBgColor;
                iTermAttribs.currentAttrib = attribNone;
            }
            if(params.contains(1))
                iTermAttribs.currentAttrib |= attribBold;
            if(params.contains(4))
                iTermAttribs.currentAttrib |= attribUnderline;
            if(params.contains(7))
                iTermAttribs.currentAttrib |= attribNegative;

            if(params.contains(22))
                iTermAttribs.currentAttrib &= ~attribBold;
            if(params.contains(24))
                iTermAttribs.currentAttrib &= ~attribUnderline;
            if(params.contains(27))
                iTermAttribs.currentAttrib &= ~attribNegative;

            foreach(int p, params) {
                if(p >= 30 && p<= 37) {
                    iTermAttribs.currentFgColor = p-30;
                }
                if(p >= 40 && p<= 47) {
                    iTermAttribs.currentBgColor = p-40;
                }
            }

            // high-intensity regular-weight extension (nonstandard)
            foreach(int p, params) {
                if(p >= 90 && p<= 97) {
                    iTermAttribs.currentFgColor = p-90+8;
                }
                if(p >= 100 && p<= 107) {
                    iTermAttribs.currentBgColor = p-100+8;
                }
            }

            if(params.contains(39))
                iTermAttribs.currentFgColor = defaultFgColor;
            if(params.contains(49))
                iTermAttribs.currentBgColor = defaultBgColor;
        } else {
            iTermAttribs.currentFgColor = defaultFgColor;
            iTermAttribs.currentBgColor = defaultBgColor;
            iTermAttribs.currentAttrib = attribNone;
        }
        break;

    case 'h':
        if(params.count()>=1 && params.contains(1) && extra=="?") { // application cursor keys
            iAppCursorKeys = true;
        }
        else if(params.count()>=1 && params.contains(3) && extra=="?") { //column mode
            // not supported, just clear screen, move cursor home & reset scrolling region
            clearAll();
            resetTabs();
            iMarginTop = 1;
            iMarginBottom = iTermSize.height();
        }
        else if(params.count()>=1 && params.contains(6) && extra=="?") { //origin mode enable
            iTermAttribs.originMode = true;
        }
        else if(params.count()>=1 && params.contains(7) && extra=="?") { //wraparound mode enable
            iTermAttribs.wrapAroundMode = true;
        }
        else if(params.count()>=1 && params.contains(12) && extra=="?") { // start blinking cursor
            // just ignore, we don't blink
        }
        else if(params.count()>=1 && params.contains(25) && extra=="?") { // show cursor
            iShowCursor = true;
        }
        else if(params.count()>=1 && params.contains(1049) && extra=="?") { //use alt screen buffer & save cursor
            iTermAttribs_saved_alt = iTermAttribs;
            iUseAltScreenBuffer = true;
            iMarginTop = 1;
            iMarginBottom = iTermSize.height();
            resetBackBufferScrollPos();

            clearAll();
            resetTabs();
            emit displayBufferChanged();
        }
        else if(params.count()>=1 && params.contains(4) && extra=="") {
            iReplaceMode = true;
        }
        else if(params.count()>=1 && params.contains(20) && extra=="") {
            iNewLineMode = true;
        }
        else unhandled=true;
        break;

    case 'l':
        if(params.count()>=1 && params.contains(1) && extra=="?") { // normal cursor keys
            iAppCursorKeys = false;
        }
        else if(params.count()>=1 && params.contains(3) && extra=="?") { //column mode
            // not supported, just clear screen, move cursor home & reset scrolling region
            clearAll();
            resetTabs();
            iMarginTop = 1;
            iMarginBottom = iTermSize.height();
        }
        else if(params.count()>=1 && params.contains(6) && extra=="?") { //origin mode disable
            iTermAttribs.originMode = false;
        }
        else if(params.count()>=1 && params.contains(7) && extra=="?") { //wraparound mode disable
            iTermAttribs.wrapAroundMode = false;
        }
        else if(params.count()>=1 && params.contains(12) && extra=="?") { // stop blinking cursor
            // no need to do anything, we don't blink
        }
        else if(params.count()>=1 && params.contains(25) && extra=="?") { // hide cursor
            iShowCursor = false;
        }
        else if(params.count()>=1 && params.contains(1049) && extra=="?") { //return from alt screen buffer & restore cursor
            iUseAltScreenBuffer = false;
            iTermAttribs = iTermAttribs_saved_alt;
            iMarginBottom = iTermSize.height();
            iMarginTop = 1;
            resetBackBufferScrollPos();
            resetTabs();
            emit displayBufferChanged();
        }

        else if(params.count()>=1 && params.contains(4) && extra=="") {
            iReplaceMode = false;
        }
        else if(params.count()>=1 && params.contains(20) && extra=="") {
            iNewLineMode = false;
        }
        else unhandled=true;
        break;

    case 'r':  // scrolling region
        if(!extra.isEmpty()) {
            unhandled=true;
            break;
        }
        if(params.count() < 2) {
            while(params.count() < 2)
                params.append(1);
            params[0] = 1;
            params[1] = iTermSize.height();
        }
        if(params.at(0) < 1)
            params[0] = 1;
        if(params.at(1) > iTermSize.height())
            params[1] = iTermSize.height();
        iMarginTop = params.at(0);
        iMarginBottom = params.at(1);
        if(iMarginTop >= iMarginBottom) {
            //invalid scroll region
            if(iMarginTop == iTermSize.height()) {
                iMarginTop = iMarginBottom - 1;
            } else {
                iMarginBottom = iMarginTop + 1;
            }
        }
        setCursorPos(QPoint( 1, iMarginTop ));
        break;

    default:
        unhandled=true;
        break;
    }

    if (unhandled)
        qDebug() << "unhandled ansi sequence " << cmdChar << params << extra;
}

void Terminal::oscSequence(const QString& seq)
{
    if(seq.length() <= 1 || seq.at(0)!=']')
        return;

    // set window title
    if( seq.length() >= 3 && seq.at(0)==']' &&
        (seq.at(1)=='0' || seq.at(1)=='2') &&
        seq.at(2)==';' )
    {
        if(iWindow) {
            iUtil->setWindowTitle(seq.mid(3));
        }
        return;
    }

    qDebug() << "unhandled OSC" << seq;
}

void Terminal::escControlChar(const QString& seq)
{
    QChar ch;

    if(seq.length()==1) {
        ch = seq.at(0);
    } else if (seq.length()>1 ){ // control sequences longer than 1 characters
        if( seq.at(0) == '(' || seq.at(0)==')' ) // character set, ignore this for now...
            return;
        if( seq.at(0) == '#' && seq.at(1)=='8' ) { // test mode, fill screen with 'E'
            clearAll(true);
            for (int i = 0; i < rows(); i++) {
                QList<TermChar> line;
                for(int j = 0; j < columns(); j++) {
                    TermChar c = zeroChar;
                    c.c = 'E';
                    line.append(c);
                }
                buffer().append(line);
            }
            return;
        }
    }

    if(ch.toLatin1()=='7') { //save cursor
        iTermAttribs_saved = iTermAttribs;
    }
    else if(ch.toLatin1()=='8') { //restore cursor
        iTermAttribs = iTermAttribs_saved;
    }
    else if(ch.toLatin1()=='>' || ch.toLatin1()=='=') { //app keypad/normal keypad - ignore these for now...
    }

    else if(ch.toLatin1()=='H') {  // set a tab stop at cursor position
        while(iTabStops.size() < cursorPos().y())
            iTabStops.append(QList<int>());

        iTabStops[cursorPos().y()-1].append(cursorPos().x());
        qSort(iTabStops[cursorPos().y()-1]);
    }
    else if(ch.toLatin1()=='D') {  // cursor down/scroll down one line
        scrollFwd(1, cursorPos().y());
    }
    else if(ch.toLatin1()=='M') {  // cursor up/scroll up one line
        scrollBack(1, cursorPos().y());
    }

    else if(ch.toLatin1()=='E') {  // new line
        if(cursorPos().y()==iMarginBottom) {
            scrollFwd(1);
            setCursorPos(QPoint(1,cursorPos().y()));
        } else {
            setCursorPos(QPoint(1,cursorPos().y()+1));
        }
    }
    else if(ch.toLatin1()=='c') {  // full reset
        resetTerminal();
    }
    else if(ch.toLatin1()=='g') {  // visual bell
        iUtil->bellAlert();
    }
    else {
        qDebug() << "unhandled escape code ESC" << seq;
    }
}

QList<TermChar>& Terminal::currentLine()
{
    while(buffer().size() <= cursorPos().y()-1)
        buffer().append(QList<TermChar>());

    if( cursorPos().y() >= 1 &&
            cursorPos().y() <= buffer().size() )
    {
        return buffer()[cursorPos().y()-1];
    }

    // we shouldn't get here
    return buffer()[buffer().size()-1];
}

const QStringList Terminal::printableLinesFromCursor(int lines)
{
    QStringList ret;

    int start = cursorPos().y() - lines;
    int end = cursorPos().y() + lines;

    for(int l=start-1; l<end; l++) {
        ret.append("");
        if(l >= 0 && l < buffer().size()) {
            for(int i=0; i<buffer()[l].size(); i++) {
                if(buffer()[l][i].c.isPrint())
                    ret[ret.size()-1].append(buffer()[l][i].c);
            }
        }
    }

    return ret;
}

void Terminal::trimBackBuffer()
{
    while(backBuffer().size() > maxScrollBackLines) {
        backBuffer().removeFirst();
    }
}

void Terminal::scrollBack(int lines, int insertAt)
{
    if(lines <= 0)
        return;

    adjustSelectionPosition(lines);

    bool useBackbuffer = true;
    if(insertAt==-1) {
        insertAt = iMarginTop;
        useBackbuffer = false;
    }
    insertAt--;

    while(lines>0) {
        if(!iUseAltScreenBuffer) {
            if(iBackBuffer.size()>0 && useBackbuffer)
                buffer().insert(insertAt, iBackBuffer.takeLast());
            else
                buffer().insert(insertAt, QList<TermChar>());
        } else {
            buffer().insert(insertAt, QList<TermChar>());
        }

        int rm = iMarginBottom;
        if(rm >= buffer().size())
            rm = buffer().size()-1;

        buffer().removeAt(rm);

        lines--;
    }
}

void Terminal::scrollFwd(int lines, int removeAt)
{
    if(lines <= 0)
        return;

    adjustSelectionPosition(-lines);

    if(removeAt==-1) {
        removeAt = iMarginTop;
    }
    removeAt--;

    while(buffer().size() < iMarginBottom)
        buffer().append(QList<TermChar>());

    while(lines>0) {
        buffer().insert(iMarginBottom, QList<TermChar>());

        if(!iUseAltScreenBuffer)
            iBackBuffer.append( buffer().takeAt(removeAt) );
        else
            buffer().removeAt(removeAt);

        lines--;
    }
    trimBackBuffer();
}

void Terminal::resetTerminal()
{
    iBuffer.clear();
    iAltBuffer.clear();
    iBackBuffer.clear();

    iTermAttribs.currentFgColor = defaultFgColor;
    iTermAttribs.currentBgColor = defaultBgColor;
    iTermAttribs.currentAttrib = 0;
    iTermAttribs.cursorPos = QPoint(1,1);
    iTermAttribs.wrapAroundMode = true;
    iTermAttribs.originMode = false;

    iTermAttribs_saved = iTermAttribs;
    iTermAttribs_saved_alt = iTermAttribs;

    iMarginBottom = iTermSize.height();
    iMarginTop = 1;

    iShowCursor = true;
    iUseAltScreenBuffer = false;
    iAppCursorKeys = false;
    iReplaceMode = false;
    iNewLineMode = false;

    resetBackBufferScrollPos();

    resetTabs();
    clearSelection();
}

void Terminal::resetTabs()
{
    iTabStops.clear();
    for(int i=0; i<iTermSize.height(); i++) {
        int tab=1;
        iTabStops.append(QList<int>());
        while(tab <= iTermSize.width()) {
            iTabStops.last().append(tab);
            tab += 8;
        }
    }
}

void Terminal::pasteFromClipboard()
{
    QClipboard *cb = QGuiApplication::clipboard();

    if(iPtyIFace && !cb->text().isEmpty()) {
        resetBackBufferScrollPos();
        iPtyIFace->writeTerm(cb->text());
    }
}

const QStringList Terminal::grabURLsFromBuffer()
{
    QStringList ret;
    QByteArray buf;

    //backbuffer
    if ((iUtil->settingsValue("general/grabUrlsFromBackbuffer", false).toBool()
         && !iUseAltScreenBuffer)
        || backBufferScrollPos() > 0)  //a lazy workaround: just grab everything when the buffer is being scrolled (TODO: make a proper fix)
    {
        for (int i=0; i<iBackBuffer.size(); i++) {
            for (int j=0; j<iBackBuffer[i].size(); j++) {
                if (iBackBuffer[i][j].c.isPrint())
                    buf.append(iBackBuffer[i][j].c);
                else if (iBackBuffer[i][j].c == 0)
                    buf.append(' ');
            }
            if (iBackBuffer[i].size() < iTermSize.width())
                buf.append(' ');
        }
    }

    //main buffer
    for (int i=0; i<buffer().size(); i++) {
        for (int j=0; j<buffer()[i].size(); j++) {
            if (buffer()[i][j].c.isPrint())
                buf.append(buffer()[i][j].c);
            else if (buffer()[i][j].c == 0)
                buf.append(' ');
        }
        if (buffer()[i].size() < iTermSize.width())
            buf.append(' ');
    }

    QStringList lookFor;
    lookFor.append("http://");
    lookFor.append("https://");

    foreach(QString prot, lookFor) {
        int ind=0;
        while( ind != -1 ) {
            ind = buf.indexOf(prot, ind);
            if(ind!=-1) {
                int ind2 = buf.indexOf(" ",ind);
                int l=-1;
                if(ind2!=-1)
                    l = ind2-ind;
                ret << buf.mid(ind,l); // the URL
                ind += prot.length();
            }
        }
    }
    ret.removeDuplicates();
    return ret;
}

QString Terminal::getUserMenuXml()
{
    if(!iUtil)
        return QString();

    QString ret;
    QFile f( iUtil->configPath()+"/menu.xml" );
    if(f.open(QIODevice::ReadOnly|QIODevice::Text)) {
        ret = f.readAll();
        f.close();
    }

    return ret;
}

void Terminal::scrollBackBufferFwd(int lines)
{
    if(iUseAltScreenBuffer || lines<=0)
        return;

    clearSelection();

    iBackBufferScrollPos -= lines;
    if(iBackBufferScrollPos < 0)
        iBackBufferScrollPos = 0;

    emit scrollBackBufferAdjusted(false);
}

void Terminal::scrollBackBufferBack(int lines)
{
    if (iUseAltScreenBuffer || lines<=0)
        return;

    clearSelection();

    iBackBufferScrollPos += lines;
    if (iBackBufferScrollPos > iBackBuffer.size())
        iBackBufferScrollPos = iBackBuffer.size();

    emit scrollBackBufferAdjusted(false);
}

void Terminal::resetBackBufferScrollPos()
{
    if(iBackBufferScrollPos==0 && iSelection.isNull())
        return;

    iBackBufferScrollPos = 0;
    clearSelection();

    emit scrollBackBufferAdjusted(true);
}

void Terminal::copySelectionToClipboard()
{
    if (selection().isNull())
        return;

    QClipboard *cb = QGuiApplication::clipboard();
    cb->clear();

    QString text;
    QString line;

    // backbuffer
    if (iBackBufferScrollPos > 0 && !iUseAltScreenBuffer) {
        int lineFrom = iBackBuffer.size() - iBackBufferScrollPos + selection().top() - 1;
        int lineTo = iBackBuffer.size() - iBackBufferScrollPos + selection().bottom() - 1;

        for (int i=lineFrom; i<=lineTo; i++) {
            if (i >= 0 && i < iBackBuffer.size()) {
                line.clear();
                int start = 0;
                int end = iBackBuffer[i].size()-1;
                if (i==lineFrom) {
                    start = selection().left()-1;
                }
                if (i==lineTo) {
                    end = selection().right()-1;
                }
                for (int j=start; j<=end; j++) {
                    if (j >= 0 && j < iBackBuffer[i].size() && iBackBuffer[i][j].c.isPrint())
                        line += iBackBuffer[i][j].c;
                }
                text += line.trimmed() + "\n";
            }
        }
    }

    // main buffer
    int lineFrom = selection().top()-1-iBackBufferScrollPos;
    int lineTo = selection().bottom()-1-iBackBufferScrollPos;
    for (int i=lineFrom; i<=lineTo; i++) {
        if (i >= 0 && i < buffer().size()) {
            line.clear();
            int start = 0;
            int end = buffer()[i].size()-1;
            if (i==lineFrom) {
                start = selection().left()-1;
            }
            if (i==lineTo) {
                end = selection().right()-1;
            }
            for (int j=start; j<=end; j++) {
                if (j >= 0 && j < buffer()[i].size() && buffer()[i][j].c.isPrint())
                    line += buffer()[i][j].c;
            }
            text += line.trimmed() + "\n";
        }
    }

    //qDebug() << text.trimmed();

    cb->setText(text.trimmed());
}

void Terminal::adjustSelectionPosition(int lines)
{
    // adjust selection position when terminal contents move

    if (iSelection.isNull() || lines==0)
        return;

    int tx = iSelection.left();
    int ty = iSelection.top() + lines;
    int bx = iSelection.right();
    int by = iSelection.bottom() + lines;

    if (ty<1) {
        ty = 1;
        tx = 1;
    }
    if (by>iTermSize.height()) {
        by = iTermSize.height();
        bx = iTermSize.width();
    }
    if (by<1 || ty>iTermSize.height()) {
        clearSelection();
        return;
    }

    iSelection = QRect(QPoint(tx,ty), QPoint(bx,by));

    emit selectionChanged();
}

void Terminal::setSelection(QPoint start, QPoint end, bool selectionOngoing)
{
    if (start.y() > end.y())
        qSwap(start, end);
    if (start.y() == end.y() && start.x() > end.x())
        qSwap(start, end);

    if (start.x() < 1)
        start.rx() = 1;
    if (start.y() < 1)
        start.ry() = 1;
    if (end.x() > iTermSize.width())
        end.rx() = iTermSize.width();
    if (end.y() > iTermSize.height())
        end.ry() = iTermSize.height();

    iSelection = QRect(start, end);

    emit selectionChanged();

    if (!selectionOngoing) {
        emit selectionFinished();
    }
}

void Terminal::clearSelection()
{
    if (iSelection.isNull())
        return;

    iSelection = QRect();

    emit selectionFinished();
    emit selectionChanged();
}

int Terminal::rows()
{
    return iTermSize.height();
}

int Terminal::columns()
{
    return iTermSize.width();
}

QRect Terminal::selection()
{
    return iSelection;
}
