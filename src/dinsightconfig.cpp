/***************************************************************************
**
**  Implementation of the DInsightConfig class
**
**  Creation date:  2017/06/12
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2020 Piql AS.
**  
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation; either version 3 of the License, or
**  any later version.
**  
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**  
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
***************************************************************************/

//  PROJECT INCLUDES
//
#include    "dinsightconfig.h"

//  QT INCLUDES
//
#include    <QSettings>
#include    <QCoreApplication>
#include    <QFile>
#include    <QDateTime>
#include    <QtSystemDetection>


/****************************************************************************/
/*! \class DInsightConfig dinsightconfig.h
 *  \ingroup Insight
 *  \brief Application config file
 * 
 *  Handles config file settings.
 *  
 */

//===================================
//  P U B L I C   I N T E R F A C E
//===================================

DInsightConfig::DInsightConfig( const QString& fileName )
    : m_FileName( fileName ),
      m_Settings( m_FileName, QSettings::IniFormat )
{
#ifdef _WIN32
//    m_Settings.setIniCodec( "UTF-8" );
#endif // _WIN32
}


DInsightConfig::DInsightConfig()
  : m_FileName( DefaultFileName() ),
#if defined(Q_OS_WIN)
    m_Settings( DefaultFileName(), QSettings::IniFormat )
#elif defined(Q_OS_MACOS)
    m_Settings( DefaultFileName(), QSettings::IniFormat )
#else
    m_Settings( QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(), QCoreApplication::applicationName() )
#endif
{
#ifdef _WIN32
//    m_Settings.setIniCodec( "UTF-8" );
#endif // _WIN32
}


QString  DInsightConfig::get( const QString& key, const QString& def /*= ""*/ )
{
    return m_Settings.value( key, def ).toString();
}


int DInsightConfig::getInt( const QString& key, int def /*= -1*/ )
{
    return m_Settings.value( key, def ).toInt();
}


bool DInsightConfig::getBool( const QString& key, bool def /*= false*/ )
{
    return m_Settings.value( key, def ).toBool();
}


DRegExps DInsightConfig::getRegExps( const QString& key, const QString& def /*= ""*/ )
{
    return StringToRegExps( get( key, def ) );
}

DLeafMatchers DInsightConfig::getLeafMatchers( const QString& key, const QString& def /*= ""*/ )
{
    return StringToLeafMatchers( get( key, def ) );
}


QString DInsightConfig::getLocalizedKey( const QString& key )
{
    return key + "_" + get( "LANGUAGE", "EN" );
}

DInsightConfig& DInsightConfig::AppConfig()
{
    static DInsightConfig conf;
    return conf;
}

//----------------------------------------------------------------------------
/*! 
 *  Get config file value.
 */

QString DInsightConfig::Get( const QString& key, const QString& def /*= ""*/ )
{
  return AppConfig().get( key, def );
}


//----------------------------------------------------------------------------
/*! 
 *  Get integer config file value.
 */

int DInsightConfig::GetInt( const QString& key, int def /*= -1*/ )
{
    return AppConfig().getInt( key, def );
}


//----------------------------------------------------------------------------
/*! 
 *  Get bool config file value.
 */

bool DInsightConfig::GetBool( const QString& key, bool def /*= false*/ )
{
    return AppConfig().getBool( key, def );
}


//----------------------------------------------------------------------------
/*! 
 *  Get regexp list.
 */

DRegExps DInsightConfig::GetRegExps( const QString& key, const QString& def /*= ""*/ )
{
    return AppConfig().getRegExps( key, def );
}


//----------------------------------------------------------------------------
/*!
 *  Get leaf matchers list.
 */

DLeafMatchers DInsightConfig::GetLeafMatchers( const QString& key, const QString& def /*= ""*/ )
{
    return AppConfig().getLeafMatchers( key, def );
}


//----------------------------------------------------------------------------
/*! 
 *  Get localized regexp list.
 */

QString DInsightConfig::GetLocalizedKey( const QString& key )
{
    return AppConfig().getLocalizedKey( key );
}


DRegExps DInsightConfig::StringToRegExps( const QString& str )
{
    DRegExps regExps;

    QStringList regExpsString = str.split( "@" );
    for ( const QString& r: regExpsString )
    {
        if ( r.length() )
        {
            QRegularExpression regExp( r );
            regExp.optimize();
            regExps.push_back( regExp );
        }
    }

    return regExps;    
}


//----------------------------------------------------------------------------
/*!
 *
 */

DLeafMatchers DInsightConfig::StringToLeafMatchers( const QString& str )
{
    DLeafMatchers matchers;

    QStringList strings = str.split( "@" );
    for ( const QString& r: strings )
    {
        if ( r.length() )
        {
            DLeafMatcher matcher;
            if ( DLeafMatcher::CreateFromString( matcher, r) )
            {
                matchers.push_back( matcher );
            }
        }
    }

    return matchers;
}


//----------------------------------------------------------------------------
/*! 
 *  Set config file value.
 */

void    DInsightConfig::Set( const QString& key, const QString& value )
{
    DInsightConfig conf( DefaultFileName() );

    conf.m_Settings.setValue( key, value );
}


//----------------------------------------------------------------------------
/*! 
 *  Return application log.
 */

QDebug&  DInsightConfig::Log()
{
    static QFile logFile( Get( "LOGFILE", QCoreApplication::applicationName() + ".log" ) );
    static QDebug logStream( &logFile );

    if ( !logFile.isOpen() )
    {
        logFile.open( QIODevice::WriteOnly | QIODevice::Text );
    }

    return logStream;
}

//----------------------------------------------------------------------------
/*!
 *  Return current date as string, suitable for creating filenames with dates.
 */

QString DInsightConfig::FileNameDatePart()
{
    return QDateTime::currentDateTime().toString( "yyyy-MM-dd" );
}

//----------------------------------------------------------------------------
/*! 
 *  Get def config file name.
 */

QString DInsightConfig::DefaultFileName()
{
    static QString def = QCoreApplication::applicationName() + QString( ".conf" );

    return def;
}
