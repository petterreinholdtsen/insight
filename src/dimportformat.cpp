/*****************************************************************************
**  
**  Implementation of the DImportFormat class
**
**  Creation date:  2019/03/19
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
*****************************************************************************/

//  PROJECT INCLUDES
//
#include "dimportformat.h"
#include "dinsightconfig.h"

//  QT INCLUDES
//
#include <QFileInfo>
#include <QString>
#include <QDir>
#include <QVector>
#include <QProcess>
 
DImportFormat::DImportFormat()
    : m_BaseFormat( nullptr ),
      m_Config( nullptr )
{
}

const QString & DImportFormat::name() const
{
    return m_Name;
}

const DRegExps & DImportFormat::patterns() const
{
    return m_Patterns;
}

const QString & DImportFormat::parser() const
{
    return m_Parser;
}

const DRegExps & DImportFormat::treeViewLabelRegExp() const
{
    return m_TreeViewLabelRegExp;
}

const DRegExps & DImportFormat::treeViewNodeRegExp() const
{
    return m_TreeViewNodeRegExp;
}

const DRegExps & DImportFormat::infoViewLabelRegExp() const
{
    return m_InfoViewLabelRegExp;
}

const DLeafMatchers & DImportFormat::documentTypeRegExp() const
{
    return m_DocumentTypeRegExp;
}

const DJournalMatchers & DImportFormat::journalMatchers() const
{
    return m_JournalMatchers;
}

const DRegExps & DImportFormat::folderTypeRegExp() const
{
    return m_FolderTypeRegExp;
}

const DRegExps & DImportFormat::deleteTypeRegExp() const
{
    return m_DeleteTypeRegExp;
}
    
const DLeafMatchers & DImportFormat::importTypeRegExp() const
{
    return m_ImportTypeRegExp;
}
    
const DLeafMatchers & DImportFormat::autoImportRegExp() const
{
    return m_AutoImportRegExp;
}

const DRegExps & DImportFormat::autoSelectRegExp() const
{
    return m_AutoSelectRegExp;
}

const DRegExps & DImportFormat::autoCollapseRegExp() const
{
    return m_AutoCollapseRegExp;
}

const DRegExps & DImportFormat::checksumTypeRegExp() const
{
    return m_ChecksumTypeRegExp;
}
    
const DRegExps & DImportFormat::checksumSourceTypeRegExp() const
{
    return m_ChecksumSourceTypeRegExp;
}

QString DImportFormat::extractTool( const QString& fileName, const QString& destination ) const
{
    if (m_ExtractTool.length() == 0 || m_ExtractTool.length() % 2 != 0)
    {
        return QString();
    }

    QString extractTool = m_ExtractTool[0+1].pattern(); // First tool is default
    for ( int i = 0; i < m_ExtractTool.length(); i += 2 )
    {
        if ( m_ExtractTool[i].match(fileName).hasMatch() )
        {
            extractTool = m_ExtractTool[i+1].pattern();
        }
    }

    QString tool = extractTool;
    tool.replace("%FILENAME%", fileName);
    tool.replace("%DESTINATION%", destination);

    return tool;
}

QString DImportFormat::fileIdTool( const QString& fileName ) const
{
    QString tool = m_FileIdTool;
    tool.replace("%FILENAME%", fileName);
    return tool;
}

QString DImportFormat::convertToPdfTool( const QString& filesFile, const QString& out, const QString& temp ) const
{
    QString tool = QDir::toNativeSeparators( m_PdfCreatorTool );
    tool.replace( "%FILESFILE%", QDir::toNativeSeparators( filesFile ) );
    tool.replace( "%OUT%", QDir::toNativeSeparators( out ) );
    tool.replace( "%TEMP%", QDir::toNativeSeparators( temp ) );
    return tool;
}

DRegExps DImportFormat::getRegExps( const QString& key )
{
    DRegExps regExps = m_Config->getRegExps( key );
    if ( regExps.size() == 0 )
    {
        if ( m_BaseFormat )
        {
            return m_BaseFormat->getRegExps( key );
        }
    }
    return regExps;
}

DLeafMatchers DImportFormat::getLeafMatchers( const QString& key )
{
    DLeafMatchers matchers = m_Config->getLeafMatchers( key );
    if ( matchers.size() == 0 )
    {
        if ( m_BaseFormat )
        {
            return m_BaseFormat->getLeafMatchers( key );
        }
    }
    return matchers;
}

bool DImportFormat::Load( DImportFormat& format, const QString& fileName )
{
    QFileInfo info( fileName );
    if ( !info.exists() )
    {
        DInsightConfig::Log() << "Non existing file: " << fileName << Qt::endl;
        return false;
    }

    delete format.m_Config;
    format.m_Config = new DInsightConfig( fileName );

    // Base format
    QString baseFileName = format.m_Config->get( "IMPORT_FORMAT_BASE", "" );
    if ( baseFileName.length() )
    {
        if (!QDir::isAbsolutePath(baseFileName))
        {
            baseFileName = info.dir().filePath(baseFileName);
        }
        format.m_BaseFormat = new DImportFormat(); 
        if ( !DImportFormat::Load( *format.m_BaseFormat, baseFileName ) )
        {
            DInsightConfig::Log() << "Failed to load base format: " << baseFileName << Qt::endl;
            return false;
        }
    }
    
    // Format name
    format.m_Name = format.m_Config->get( "IMPORT_FORMAT_NAME", "" );
    format.m_Patterns = format.m_Config->getRegExps( "IMPORT_FORMAT_PATTERNS", "" );
    format.m_Parser = format.m_Config->get( "IMPORT_FORMAT_PARSER", "xml" );

    // Exctract tool - used to expand some format types (ZIP, 7Z, tgz, etc) before parsing
    format.m_ExtractTool = format.m_Config->getRegExps( "EXTRACT_TOOL", "" );

    // File id tool - used to identify file formats, prefered before extension matching
    format.m_FileIdTool = format.m_Config->get( "IMPORT_FORMAT_ID_TOOL", "" );

    // Pdf creator tool - creates PDF for journal pages
    format.m_PdfCreatorTool = format.m_Config->get( "JOURNAL_PDF_CREATOR_TOOL", "" );
    
    // Tree node modifiers, makes node names more "presentable"
    format.m_TreeViewNodeRegExp = format.getRegExps( "TREEVIEW_NODE_REGEXP" );

    // Tree labels replaces %nodename% with node value. Special name is %node%, replaces with node itself.
    format.m_TreeViewLabelRegExp = format.getRegExps( format.m_Config->getLocalizedKey( "TREEVIEW_LABEL_REGEXP" ) );
    if ( format.m_TreeViewLabelRegExp.size() && format.m_TreeViewLabelRegExp.size() % 2 != 0 )
    {
        DInsightConfig::Log() << "Invalid TREEVIEW_LABEL_REGEXP. It must have regexp/label pairs." << Qt::endl;
        format.m_TreeViewLabelRegExp.clear();
    }

    // Info node modifiers, makes node names more "presentable"
    format.m_InfoViewLabelRegExp = format.getRegExps( "INFOVIEW_LABEL_REGEXP" );

    // Detect document info node type, presented with a 'View' button in info view.
    format.m_DocumentTypeRegExp = format.getLeafMatchers( "INFOVIEW_DOCUMENT_TYPE_REGEXP" );

    // Detect journal nodes, journal nodes have options for handling associated documents
    if ( !DJournalMatcher::CreateFromString( format.m_JournalMatchers, format.m_Config->get( "INFO_VIEW_JOURNAL_TYPE_REGEXP" ) ) )
    {
        DInsightConfig::Log() << "Invalid INFO_VIEW_JOURNAL_TYPE_REGEXP. It must have regexp/wildcard/wildcard triplets." << Qt::endl;
    }

    // Detect folder info node type, presented with a 'View' button in info view
    format.m_FolderTypeRegExp = format.getRegExps( format.m_Config->getLocalizedKey( "INFOVIEW_FOLDER_TYPE_REGEXP" ) );

    // Detect delete info node type, presented with a 'Delete' button in info view
    format.m_DeleteTypeRegExp = format.getRegExps( "INFOVIEW_DELETE_TYPE_REGEXP" );

    // Detect import info node type, presented with a 'Import' button in info view
    format.m_ImportTypeRegExp = format.getLeafMatchers( format.m_Config->getLocalizedKey( "INFOVIEW_IMPORT_TYPE_REGEXP" ) );

    // Auto load nodes
    format.m_AutoImportRegExp = format.getLeafMatchers( format.m_Config->getLocalizedKey( "INFOVIEW_AUTO_IMPORT_REGEXP" ) );

    // Auto select nodes
    format.m_AutoSelectRegExp = format.getRegExps( "TREEVIEW_AUTO_SELECT_REGEXP" );

    // Auto collapse nodes
    format.m_AutoCollapseRegExp = format.getRegExps( "TREEVIEW_AUTO_COLLAPSE_REGEXP" );

    // Detect checksum node, presented with a 'Validate' button in info view
    format.m_ChecksumTypeRegExp = format.getRegExps( format.m_Config->getLocalizedKey( "INFOVIEW_CHECKSUFORMAT_TYPE_REGEXP" ) );
    format.m_ChecksumSourceTypeRegExp = format.getRegExps( format.m_Config->getLocalizedKey( "INFOVIEW_CHECKSUFORMAT_SOURCE_TYPE_REGEXP" ) );

    return true;
}


bool DImportFormats::Load( DImportFormats& formats, const QString& dirName )
{
    // Enumerate all files in dir
    QDir dir( dirName );
    if ( !dir.exists() )
    {
        return false;
    }

    QStringList filters;
    filters << "*.conf";

    QFileInfoList files = dir.entryInfoList( filters );
    DImportFormat defaultFormat;
    bool defaultFound = false;
    for ( QFileInfo file: files )
    {
        DInsightConfig::Log() << "Loading format: " << file.absoluteFilePath() << Qt::endl;
        DImportFormat format;
        if ( DImportFormat::Load(format, file.absoluteFilePath()) )
        {
            DInsightConfig::Log() << "Format name: " << format.name() << Qt::endl;
            if ( format.name() != "default" )
            {
                formats.push_back(format);
            }
            else
            {
                defaultFormat = format;
                defaultFound = true;
            }
        }
        else
        {
            DInsightConfig::Log() << "ERROR loading format: " << file.absoluteFilePath() << Qt::endl;
        }
    }

    // We would like the default format to be last in the list.
    if ( defaultFound )
    {
        formats.push_back(defaultFormat);
    }

    return true;
}


const DImportFormat* DImportFormats::defaultFormat() const
{
    DImportFormats::const_iterator it = begin();
    DImportFormats::const_iterator itEnd = end();
    for ( ; it != itEnd; it++ )
    {
        if ( it->name() == "default" )
        {
            return it;
        }
    }

    return nullptr;
}


const DImportFormat* DImportFormats::findMatching( const QString& fileName ) const
{
    DImportFormats::const_iterator it = begin();
    DImportFormats::const_iterator itEnd = end();

    // First test if the file format id tool matches
    for ( ; it != itEnd; it++ )
    {
        QString tool = it->fileIdTool( fileName );
        if ( tool.length() != 0 )
        {
            DInsightConfig::Log() << "Executing: " << tool << Qt::endl;

            int ok = QProcess::execute( tool );
            if ( ok == 0 )
            {
                return it;
            }
        }
    }

    // Then regular expression match
    it = begin();
    itEnd = end();
    for ( ; it != itEnd; it++ )
    {
        const DRegExps& patterns = it->patterns();

        for ( const DRegExp& p: patterns )
        {
            QFileInfo info( fileName );

            auto pattern = QString("a .*|") + QRegularExpression::wildcardToRegularExpression(p.pattern());

            // re matches exactly the pattern string p
            auto wildcard = QRegularExpression(QRegularExpression::anchoredPattern(pattern));
            if ( wildcard.match( info.fileName() ).hasMatch() )
            {
                DInsightConfig::Log() << "File name " << fileName << " matches format " << it->name() << " with pattern " << p.pattern() << Qt::endl;
                return it;
            }
        }
    }

    DInsightConfig::Log() << "No format matching: " << fileName << ", returning default" << Qt::endl;
    return defaultFormat();
}

const DImportFormat* DImportFormats::find( const QString& formatName ) const
{
    DImportFormats::const_iterator it = begin();
    DImportFormats::const_iterator itEnd = end();
    for ( ; it != itEnd; it++ )
    {
        if ( it->name() == formatName )
        {
            return it;
        }
    }
    return nullptr;
}
