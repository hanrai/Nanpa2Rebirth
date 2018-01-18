#include "engine.h"

void Engine::unknown0xe()
{
    union ByteWorker
    {
        quint8 byte[4];
        quint16 word[2];
        quint32 dword;
    };
    sendMessage("SetBookmark();");
     if ( !bookmarksReady )
     {
        memset(mesBookmarkIndexs, 0xFF, sizeof(mesBookmarkIndexs));
        field_37C=0xFF;
        gStruct.field_190 = 0;
        memset(mesCursorBookmarks, 0, sizeof(mesCursorBookmarks));
        bookmarksReady = 1;
     }
     prepareSubCommands();

     ByteWorker bw;
     bw.byte[0]=getMes();
     bw.byte[1]=getMes();
     bw.byte[2]=getMes();
     bw.byte[3]=getMes();

     g_dword_562890[gStruct.field_190++] = subCommands[0].command;
     int index = 0;
     if ( mesCursorBookmarks[0] )
     {
       do
       {
         index++;
       }
       while ( mesCursorBookmarks[index] );
     }
     else
     {
       index = 0;
     }
     mesBookmarkIndexs[subCommands[0].command] = index;
     mesCursorBookmarks[index] = gStruct.gMesCursor;
     gStruct.gMesCursor = gStruct.mesStartPoint + bw.dword;
}
