/***************************************************************************
**
**  Implementation of the DXmlParser class
**
**  Creation date:  2017/11/08
**  Created by:     Ole Liabo
**
**
**  Copyright (c) 2017 Piql AS. All rights reserved.
**
***************************************************************************/

//  PROJECT INCLUDES
//
#include    "dxmlparser.h"
#include    "dtreeitem.h"
#include    "dtreemodel.h"

//  SYSTEM INCLUDES
//
#include    <fstream>
#include    <vector>
#include    <iostream>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <fcntl.h>
#include    <stdio.h>

//  PLATFORM INCLUDES
//
#if defined (WIN32)
#include    <io.h>
#else
#include    <unistd.h>
#endif

//  XML INCLUDES
//
#include    "yxml.h"

using namespace std;


/****************************************************************************/
/*! \class UnorderedSetEqualString dxmlparser.cpp
 *  \ingroup Insight
 *  \brief Unordered set item compare
 */

/****************************************************************************/
/*! \class UnorderedSetDereference dxmlparser.cpp
 *  \ingroup Insight
 *  \brief Unordered set hash function
 */

/****************************************************************************/
/*! \class StringHash dxmlparser.cpp
 *  \ingroup Insight
 *  \brief String hashmap
 */

static const char* AddToHashMapInternal( const char* text, DXmlParser::StringHash& map )
{
    DXmlParser::StringHashIterator it = map.find( text );
    if ( it == map.end() )
    {
        char* s = strdup( text );
        it = map.insert( s ).first; 
    }

    return *it;
}

// Global node name hash map
static DXmlParser::StringHash nodeMap;
static DXmlParser::StringHash stringMap;

/****************************************************************************/
/*! \class DXmlContext dxmlparser.cpp
 *  \ingroup Insight
 *  \brief Context for XML parser
 */

class DXmlContext
{
public:
    DXmlContext();

    void incItems( DTreeItem* item, bool finalItem );

    // Processing
    DTreeItem*          m_CurrentNode;
    DTreeItem*          m_LastUpdateNode;
    char                m_Data[64*1024];    
    char*               m_DataPos;    
    const char*         m_Name;    
    bool                m_LastClose; // Was last instruction a close node?

    // Thread
    DXmlParser*         m_Thread;
    int                 m_FileHandle;
    DTreeItems*         m_TreeItems;
    DTreeModel*         m_Model;
    DTreeRootItem*      m_RootNode;

    // Stats
    unsigned long long  m_Items;
    unsigned long long  m_ItemsLastReport;
    unsigned long long  m_FileSize;
    unsigned long long  m_FilePos;
};


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DXmlContext::DXmlContext()
    : m_LastUpdateNode( NULL ),
      m_Items( 0ULL ),
      m_ItemsLastReport( 0ULL )
{
}


//----------------------------------------------------------------------------
/*! 
 *  Called for each item added to the tree data model. Responsible for 
 *  reporting progress to GUI.
 */

void DXmlContext::incItems( DTreeItem* item, bool finalItem )
{
    if ( item )
    {
        m_Items++;
        m_TreeItems->push_back( item );
    }

    const unsigned long reportInterval = 10000;
    if ( finalItem )
    {
        m_Thread->reportProgress( m_Items, 1.0f );
    }
    else if ( m_Items == m_ItemsLastReport + reportInterval )
    {
#if defined (WIN32)        
        m_FilePos = _telli64(m_FileHandle);
#else
        m_FilePos = lseek(m_FileHandle,0,SEEK_CUR);
#endif
        m_Thread->reportProgress( m_Items, (m_FilePos / (float)m_FileSize) );
        m_ItemsLastReport = m_Items;
    }

    m_LastUpdateNode = item;
}


//----------------------------------------------------------------------------
/*! 
 *  XML processor: Builds tree from XML
 */

#define yxml_isSP(c) (c == 0x20 || c == 0x09 || c == 0x0a)
inline static void sax_cb(yxml_t *x, yxml_ret_t r, DXmlContext* context ) 
{
    switch(r) 
    {
    case YXML_OK:
        break;
    case YXML_ELEMSTART:
            // Create new node?
            if ( context->m_Name )
            {
                // If root node is NULL, first element found should be root
                if ( context->m_RootNode == NULL )
                {
                    context->m_Model->beginInsert( 0, 1, QModelIndex() );
                    context->m_RootNode = context->m_Model->createDocumentRoot( context->m_Name );
                    context->m_CurrentNode = context->m_RootNode;
                    context->m_Model->endInsert();
                    DXmlParser::AddToNodeHashMap( context->m_Name );
                }
                else
                {
                    context->m_Model->beginInsert( 0, 1, context->m_Model->index( context->m_CurrentNode->m_Parent ) );
                    context->m_CurrentNode = context->m_Model->createItem( context->m_RootNode, context->m_CurrentNode, context->m_Name );
                    context->m_Model->endInsert();
                    DXmlParser::AddToNodeHashMap( context->m_Name );
                }
            }

            context->m_LastClose = false;
            context->m_Name = DXmlParser::AddToHashMap( x->elem );
        break;
    case YXML_ELEMEND:
            if ( !context->m_LastClose /*context->m_DataPos != context->m_Data*/ )
            {
                *context->m_DataPos = '\0';
                int s = (context->m_DataPos + 1) - context->m_Data;
                char* d = (char*)malloc( s );
                memcpy( d, context->m_Data, s);
                context->m_CurrentNode->addNode( context->m_Model->createLeaf( context->m_RootNode, context->m_Name, d ) );
            }
            else
            {
                context->incItems( context->m_CurrentNode, false );
                context->m_CurrentNode = context->m_CurrentNode->m_Parent;
            }
            context->m_DataPos = context->m_Data;
            *context->m_DataPos = '\0';
            context->m_Name = NULL;
            context->m_LastClose = true;
        break;
    case YXML_ATTRSTART:
    case YXML_ATTREND:
        break;
    case YXML_PICONTENT:
    case YXML_CONTENT:
        {
            const char *text = x->data;
    
            if ( context->m_DataPos == context->m_Data )
            {
                while ( *text )
                {
                    if ( !yxml_isSP( *text ) )
                    {
                        break;    
                    }
                    text++;            
                }
            }
            
            while( *text )
                *(context->m_DataPos++) = *(text++);

            break;
        }
    case YXML_ATTRVAL:
        break;
    case YXML_PISTART:
        break;
    case YXML_PIEND:
        break;
    default:
        exit(0);
    }
}


/****************************************************************************/
/*! \class DXmlParser dxmlparser.cpp
 *  \ingroup Insight
 *  \brief XML parser
 *
 *  Responsble for loading XML file pointed to by fileName into model under 
 *  node rootNode.
 * 
 *  XML parsing is done in a thread.
 */


//----------------------------------------------------------------------------
/*! 
 *  Constructor.
 */

DXmlParser::DXmlParser( DTreeItems* treeItems, const QString& fileName, DTreeModel* model, DTreeRootItem* rootNode )
    : m_TreeItems( treeItems ),
      m_Filename( fileName ),
      m_Model( model ),
      m_RootNode( rootNode ),
      m_NodeCount( 0UL )
{
}


//----------------------------------------------------------------------------
/*! 
 *  Returns number of tree items processed.
 */

unsigned long DXmlParser::nodeCount()
{
    return m_NodeCount;
}


//----------------------------------------------------------------------------
/*! 
 *  Thread entry point
 */

void DXmlParser::run()
{
    yxml_ret_t r;
    yxml_t x[1];
    char stack[8*1024];

    yxml_init(x, stack, sizeof(stack));

    DXmlContext context;
    context.m_CurrentNode = m_RootNode;
    context.m_LastUpdateNode = m_RootNode;
    context.m_DataPos = context.m_Data;
    *context.m_DataPos = '\0';
    context.m_LastClose = false;
    context.m_Name = NULL;
    context.m_Thread = this;
    context.m_FileSize = 0UL;
    context.m_FilePos = 0UL;
#if defined (WIN32)    
    context.m_FileHandle = _open( m_Filename.toStdString().c_str(), _O_RDONLY | _O_SEQUENTIAL );
#else
    context.m_FileHandle = open( m_Filename.toStdString().c_str(), O_RDONLY );    
#endif
    context.m_TreeItems = m_TreeItems;
    context.m_Model = m_Model;
    context.m_RootNode = m_RootNode;

    if ( context.m_FileHandle == -1 )
    {
        DInsightConfig::log() << "Opening failed, file does not exist: " << m_Filename << endl;
        return;
    }
    
#if defined (WIN32)    
    _lseeki64( context.m_FileHandle, 0, SEEK_END );
    context.m_FileSize = _telli64( context.m_FileHandle );
    _lseeki64( context.m_FileHandle, 0, SEEK_SET );
#else
    context.m_FileSize = lseek( context.m_FileHandle, 0, SEEK_END );
    lseek( context.m_FileHandle, 0, SEEK_SET );
#endif
    
    char buffer[1024*16];
#if defined (WIN32)
    int bytesRead = _read( context.m_FileHandle,  buffer, sizeof(buffer));
#else
    int bytesRead = read( context.m_FileHandle,  buffer, sizeof(buffer));
#endif    
    while( bytesRead > 0 && !isInterruptionRequested()) 
    {
        char *b = buffer;
        while ( bytesRead )
        {
        r = yxml_parse(x, *b);
        sax_cb(x, r, &context);

            b++;
            bytesRead--;
        }
#if defined (WIN32)
        bytesRead = _read( context.m_FileHandle,  buffer, sizeof(buffer));
#else
        bytesRead = read( context.m_FileHandle,  buffer, sizeof(buffer));
#endif
    }

#if defined (WIN32)
    _close( context.m_FileHandle );
#else
    close( context.m_FileHandle );
#endif
    context.incItems( context.m_CurrentNode, !isInterruptionRequested() );
    m_NodeCount = context.m_Items;
    m_RootNode = context.m_RootNode;
}


//----------------------------------------------------------------------------
/*! 
 *  Report progress to GUI.
 */

void DXmlParser::reportProgress( unsigned long nodeCount, float progress )
{
    emit nodesReady( nodeCount, progress );
}



//----------------------------------------------------------------------------
/*! 
 *  Add node name to hash map.
 */
const char* DXmlParser::AddToNodeHashMap( const char* text )
{
    return AddToHashMapInternal( text, nodeMap );
}

const char* DXmlParser::AddToHashMap( const char* text )
{
    return AddToHashMapInternal( text, stringMap );
}

DXmlParser::StringHashIterator DXmlParser::NodeHashMapBegin()
{
    return nodeMap.begin();
}

DXmlParser::StringHashIterator DXmlParser::NodeHashMapEnd()
{
    return nodeMap.end();
}


//----------------------------------------------------------------------------
/*! 
 *  Return XML root item. 
 */

DTreeRootItem* DXmlParser::root()
{
    return m_RootNode;
}
