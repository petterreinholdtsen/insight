#ifndef DATTACHMENTPARSER_H
#define DATTACHMENTPARSER_H

/*****************************************************************************
**  
**  Definition of the DAttachmentParser class
**
**  Creation date:  2017/11/08
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
*****************************************************************************/

//  PROJECT INCLUDES
//
#include    "dregexp.h"
#include    "dtreeitem.h"

//  QT INCLUDES
//
#include    <QThread>
#include    <QString>
#include    <QMutex>
#include    <QWaitCondition>

//  FORWARD DECLARATIONS
//
class DTreeItem;

//============================================================================
// CLASS: DAttachment

class DAttachment
{
public:
    const char* m_FileName;
    DTreeItem*  m_TreeItem;
};

typedef unsigned int DAttachmentIndex;

//============================================================================
// CLASS: DAttachments

typedef std::vector<DAttachment*> DAttachments;

//============================================================================
// CLASS: DAttachmentIndexes

typedef std::vector<DAttachmentIndex> DAttachmentIndexes;


//============================================================================
// CLASS: DAttachmentParser

class DAttachmentParser : public QThread
{
    Q_OBJECT
public:

    DAttachmentParser( DTreeItems* treeItems, const QString& rootDir, const DRegExps& attachmentTypeRegExp );
   ~DAttachmentParser();

    DAttachments&       attachments();
    DAttachmentIndexes& attachmentsFound();
    DAttachmentIndexes& attachmentsNotFound();
    qint64              attachmentsSizeInBytes();

public:
    static QString      AttachmentPath( const QString& fileName, const QString& documentRoot );
    static bool         AttachmentExists( const QString& fileName, const QString& documentRoot, qint64& size );

public slots: 
    void                nodesReady( unsigned long count, float progress );

private:
    void                run();
    bool                isAttachmentNode( const char* text );

private:
    unsigned int        m_MaxNodeCount;
    bool                m_FinalNodeReached;

    DTreeItems*         m_TreeItems;
    QString             m_RootDir;
    DAttachments        m_Attachments;
    DAttachmentIndexes  m_AttachmentsFound;
    DAttachmentIndexes  m_AttachmentsNotFound;
    DRegExps            m_AttachmentTypeRegExp;
    QMutex              m_Mutex;
    QWaitCondition      m_Wait;
    qint64              m_AttachmentsSizeBytes;
};


#endif // DATTACHMENTPARSER_H
