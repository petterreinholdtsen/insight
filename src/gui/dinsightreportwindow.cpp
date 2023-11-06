/***************************************************************************
**
**  Implementation of the DInsightReportWindow class
**
**  Creation date:  2017/08/12
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
#include    "dinsightreportwindow.h"
#include    "dinsightconfig.h"
#include    "qpersistantfiledialog.h"
#include    "dinsightjournalwindow.h"
#include    "dcontext.h"

//  ZIP INCLUDES
//
#include    <JlCompress.h>

//  POPPLER INCLUDES
//
//#include    <qt5/poppler-qt5.h>

//  QT INCLUDES
//
#include    <QPdfDocument>
#include    <QPainter>
#include    <QFileDialog>
#include    <QPrinter>
#include    <QDesktopServices>
#include    <QTemporaryFile>
#include    <QDir>
#include    <QProcess>
#include    <QMessageBox>
#include    <QPrintDialog>


/****************************************************************************/
/*! \class DInsightReportWindow dinsightreportwindow.h
 *  \ingroup Insight
 *  \brief Report Window dialog
 * 
 *  The report window has a display widget showing the report of the selected 
 *  XML nodes. It also has several report action buttons (save, email, etc).
 */
 
//===================================
//  P U B L I C   I N T E R F A C E
//===================================


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DInsightReportWindow::DInsightReportWindow( 
    DInsightReport& report, 
    QStringList& attachments,
    DJournals& journals,
    DImports& imports  )
  : m_Report( report ),
    m_Journals( journals ),
    m_Imports( imports )
{
    m_Ui.setupUi( this );    

    // Set window title
    setWindowTitle( tr("Report") );

    // Signals and slots. The GUI components emits signals that are handled by the slots.
    QObject::connect( m_Ui.okButton, SIGNAL(clicked()), this, SLOT(okButtonClicked()) );
    QObject::connect( m_Ui.saveReportButton, SIGNAL(clicked()), this, SLOT(saveButtonClicked()) );
    QObject::connect( m_Ui.emailReportButton, SIGNAL(clicked()), this, SLOT(emailButtonClicked()) );
    QObject::connect( m_Ui.printReportButton, SIGNAL(clicked()), this, SLOT(printButtonClicked()) );

    m_Ui.reportView->setHtml( m_Report.text() );
    m_Attachments = attachments;
}


//----------------------------------------------------------------------------
/*!                                                                              
 *  Destructor.
 */

DInsightReportWindow::~DInsightReportWindow()
{
}


//----------------------------------------------------------------------------
/*! 
 *  Exit dialog
 */

void DInsightReportWindow::okButtonClicked()
{
    accept();
}


//----------------------------------------------------------------------------
/*! 
 *  Save.
 */

void DInsightReportWindow::saveButtonClicked()
{
    // Get filename
    QString fileName;

    QString fixedFolder = DInsightConfig::Get("FIXED_REPORT_EXPORT_FOLDER");
    if ( fixedFolder.length() )
    {
        fileName = QPersistantFileDialog::getFixedRootSaveFileName(this, tr("Export PDF report"), tr( "report.pdf" ), fixedFolder );
    }
    else
    {
        fileName = QPersistantFileDialog::getSaveFileName( "savereport", this, tr("Export PDF report"), tr("report.pdf"), "*.pdf");
    }

    if ( QFileInfo( fileName ).suffix().isEmpty() )
    {
        fileName.append( ".pdf" );
    }

    createExport(fileName);
}


void DInsightReportWindow::createExport( const QString& fileName )
{
    createPdfReport( fileName );

    if ( m_Attachments.size() || m_Journals.size() )
    {
        QString fileNameZip = QFileInfo( fileName ).path() + QDir::separator() + QFileInfo( fileName ).completeBaseName() + "-attachments.zip";
        fileNameZip = QDir::toNativeSeparators( fileNameZip );
        createAttachmentArchive( fileNameZip, fileName );
    }
}

//----------------------------------------------------------------------------
/*! 
 *  Write PDF report to given fileName.
 */

bool DInsightReportWindow::createPdfReport( const QString& fileName )
{
    return m_Report.save( fileName );
}

bool DInsightReportWindow::createJournalAttachments( QStringList& files )
{
    if ( m_Journals.size() )
    {
        // Convert journal to PDF
        DJournalsIterator it = m_Journals.begin();
        DJournalsIterator itEnd = m_Journals.end();

        int journalCount = 0;
        for ( ; it != itEnd; it++ )
        {
            QString journalFilename = tr( "journal-%1.pdf" ).arg( journalCount );
            DContext::MakeAbsolute( journalFilename, (*it)->m_TreeItem, m_Imports );
            QString tempDir = QFileInfo( journalFilename ).path();
            bool ok = DInsightJournalWindow::GeneratePdf( journalFilename, *it, tempDir );
            if ( !ok )
            {
                QString title = tr( "Failed to create PDF file" );
                QString message = tr( "Failed to create file: %1" ).arg( journalFilename );

                DInsightConfig::Log() << message << Qt::endl;
                QMessageBox::warning( this, title, message );
                return false;
            }

            files.push_back( journalFilename );

            journalCount++;
        }
    }
    return true;
}


//----------------------------------------------------------------------------
/*! 
 *  Write PDF report to given printer.
 */

bool DInsightReportWindow::printReport( QPrinter& printer )
{
    return m_Report.print( printer );
}


//----------------------------------------------------------------------------
/*! 
 *  Write attachments to given printer.
 */

bool DInsightReportWindow::printAttachments( QPrinter& printer )
{
    QStringList files = m_Attachments;
    if ( !createJournalAttachments( files ) )
    {
        return false;
    }

    for ( const QString& attachment: files )
    {        
        QPainter painter( &printer );

        if ( attachment.endsWith(".pdf", Qt::CaseSensitivity::CaseInsensitive) )
        {
            if ( !printPdfAttachment( painter, printer, attachment ) )
            {
                return false;
            }
        }
        else if ( attachment.endsWith(".tif", Qt::CaseSensitivity::CaseInsensitive) ||
                  attachment.endsWith(".tiff", Qt::CaseSensitivity::CaseInsensitive) ||
                  attachment.endsWith(".jpg", Qt::CaseSensitivity::CaseInsensitive) ||
                  attachment.endsWith(".jpeg", Qt::CaseSensitivity::CaseInsensitive) )
        {
            if ( !printImageAttachment( painter, printer, attachment ) )
            {
                return false;
            }
        }
        else
        {
            if ( !printTextAttachment( painter, printer, attachment ) )
            {
                return false;
            }
        }
    }

    return true;
}

bool DInsightReportWindow::printPdfAttachment( QPainter& painter, QPrinter& printer, const QString& attachment )
{
    QPdfDocument pdfDoc( this );
    if ( pdfDoc.load(attachment ) != QPdfDocument::Error::None )
    {
        QMessageBox::warning( this, tr( "Failed to open PDF" ), tr( "Failed to open: %1" ).arg( attachment ) );
        return false;
    }

    for ( int p = 0; p < pdfDoc.pageCount(); p++ )
    {
        QImage image = pdfDoc.render(p, printer.pageRect(QPrinter::Unit::DevicePixel).size().toSize());
        if ( image.isNull() )
        {
            QMessageBox::warning( this, tr( "Failed to render PDF" ), tr( "Failed to render: %1" ).arg( attachment ) );
            return false;
        }
        painter.drawImage( 0, 0, image );
        printer.newPage();
    }

    return true;
}


bool DInsightReportWindow::printImageAttachment( QPainter& painter, QPrinter& printer, const QString& attachment )
{
    QImage image( attachment );
    if ( image.isNull() )
    {
        QMessageBox::warning( this, tr( "Failed to open image" ), tr( "Failed to open image: %1" ).arg( attachment ) );
        return false;
    }

    double xscale = printer.pageRect(QPrinter::Unit::DevicePixel).width() / double(image.width());
    double yscale = printer.pageRect(QPrinter::Unit::DevicePixel).height() / double(image.height());

    double scale = qMin(xscale, yscale);
    painter.translate(printer.paperRect(QPrinter::Unit::DevicePixel).x() + printer.pageRect(QPrinter::Unit::DevicePixel).width() / 2,
                      printer.paperRect(QPrinter::Unit::DevicePixel).y() + printer.pageRect(QPrinter::Unit::DevicePixel).height() / 2);
    painter.scale(scale, scale);
    painter.translate(-image.width() / 2, -image.height() / 2); // note uses the form width/height! use pix.h/w if random image
    //painter.drawPixmap(0, 0, pix);

    painter.drawImage( 0, 0, image );
    printer.newPage();

    return true;
}


bool DInsightReportWindow::printTextAttachment( QPainter& /*painter*/, QPrinter& printer, const QString& attachment )
{
    QFile data(attachment);
    if ( !data.open(QFile::ReadOnly) )
    {
        QMessageBox::warning( this, tr( "Failed to open file" ), tr( "Failed to open file: %1" ).arg( attachment ) );
        return false;
    }

    QTextStream out( &data );
    QTextDocument doc( out.readAll() );
    doc.setPageSize( printer.pageRect(QPrinter::Unit::DevicePixel).size() ); // This is necessary if you want to hide the page number
    doc.print( &printer );

    return true;
}


//----------------------------------------------------------------------------
/*! 
 *  Add attachments to ZIP file.
 */

bool DInsightReportWindow::createAttachmentArchive( const QString& fileName, const QString& reportFileName )
{
    QStringList files = m_Attachments;
    files.push_front( reportFileName );

    if ( !createJournalAttachments( files ) )
    {
        return false;
    }

    if ( !JlCompress::compressFiles( fileName, files ) )
    {
        QString title = tr( "Failed to create ZIP file");
        QString message = tr( "Failed to create file: %1" ).arg( fileName );

        DInsightConfig::Log() << message << Qt::endl;
        QMessageBox::warning( this, title, message );
        return false;
    }
        
    return true;
}


//----------------------------------------------------------------------------
/*! 
 *  Send report on e-mail.
 */

void DInsightReportWindow::emailButtonClicked()
{
    // Warn user that he/she is sending documents outside the organization.
    QMessageBox::StandardButton result = QMessageBox::warning( this, tr("Sensitive Information"), tr("Please note that e-mail is not a secure method for transporting sensitive information, and might not be allowed to use in some organizations. Do you want to continue?"), QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if ( result != QMessageBox::Ok )
    {
        return;
    }

    QDir dir( QDir::temp() );
    QString fileName = QDir::toNativeSeparators( dir.absoluteFilePath( tr("report-%1.pdf").arg( DInsightConfig::FileNameDatePart() ) ) );
    if ( !createPdfReport( fileName ) )
    {
        return;
    }

    QString attachment;
    if ( m_Attachments.size() || m_Journals.size() )
    {
        QString fileNameZip = QDir::toNativeSeparators( QFileInfo(fileName).path() + QDir::separator() + QFileInfo(fileName).completeBaseName() + ".zip" );
        if ( !createAttachmentArchive( fileNameZip, fileName ) )
        {
            return;
        }
        attachment = fileNameZip;
    }
    else
    {
        attachment = fileName; // Send as PDF
    }

    // Alternative 1
    //QString body( tr("Report generated by Insight.") );
    //QUrl url( "mailto:user@foo.com?subject=Report&body=" + body + tr(" Vedlegg: ") + fileName, QUrl::TolerantMode );
    //QDesktopServices::openUrl( url );

    // Alternative 2
    QString emailApp = DInsightConfig::Get( "EMAIL_APPLICATION", "c:\\Program Files (x86)\\Microsoft Office\\root\\Office16\\OUTLOOK.EXE" );
    QString emailArg = DInsightConfig::Get( "EMAIL_ARGUMENTS", "/c ipm.note /m 'mailto:johndoe@domain.com&subject=Report' /a %ATTACHMENT_FILENAME%" );
    emailArg = emailArg.replace( "%ATTACHMENT_FILENAME%", attachment );
    QString command = "\"" + emailApp + "\"  " + emailArg;

    if ( !QProcess::startDetached( command ) )
    {
        QMessageBox::warning( this, tr("E-mail application not found"), tr("E-mail application (%1) not found, please press OK to select location.").arg( emailApp ) );
        emailApp = QFileDialog::getOpenFileName( this, tr("Select e-mail application"), emailApp, tr("Applications (*.exe)") );

        if ( emailApp.length() )
        {
            command = "\"" + emailApp + "\"  " + emailArg;
            if ( !QProcess::startDetached( command ) )
            {
                QMessageBox::warning( this, tr("Failed to send e-mail"), tr("Failed to send e-mail. You could try to open your e-mail application manually and select: %1 as your attachment.").arg( fileName ) );
            }
            else
            {
                DInsightConfig::Set( "EMAIL_APPLICATION", emailApp );
            }
        }
    }
}


//----------------------------------------------------------------------------
/*! 
 *  Print report and attachments.
 */

void DInsightReportWindow::printButtonClicked()
{
    // Ask user if attachments should be included
    int includeAttachments = QMessageBox::question( this, tr( "Print Attachments"), tr( "Should report attachments be included?" ), QMessageBox::Yes, QMessageBox::No );
    
    QPrinter printer;
    QPrintDialog dialog( &printer, this );
    dialog.setWindowTitle( tr("Print Report") );
    if ( dialog.exec() != QDialog::Accepted )
    {
        return;
    }

    bool printOK = printReport( printer );

    if ( printOK && includeAttachments == QMessageBox::Yes )
    {
        printAttachments( printer );
    }
}
