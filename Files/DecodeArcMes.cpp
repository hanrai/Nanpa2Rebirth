int DecodeArcMes(char * p_sourceBuffer, char* p_destBuffer,unsigned int size)
{
    char dictionary[4096] = {0};

    unsigned int index=0, dest_index=0, indexDictionary=4078;

    union ByteWorker
    {
        unsigned int dword;
        unsigned char byte[4];
        unsigned short int word[2];
    } key, section, wordSize;
    key.dword=0;

    unsigned char data=0, offset=0;

    while ( 1 )
    {
        key.dword >>= 1;
        if ( !(key.byte[1] & 1) )
        {
            if ( index == size )
                break;
            key.byte[0] = p_sourceBuffer[index++];
            key.byte[1] = 0xFF;
        }

        if ( index == size )
            break;

        if ( key.byte[0] & 1 )
        {
            data = p_sourceBuffer[index++];
            p_destBuffer[dest_index++] = data;
            dictionary[indexDictionary++] = data;
            indexDictionary &= 0xFFF;
        }
        else
        {
            offset = p_sourceBuffer[index++];
            if( index == size )
                break;
            wordSize.dword = 0;
            section.dword = 0;

            wordSize.byte[0] = p_sourceBuffer[index++];

            section.dword = wordSize.dword;
            section.dword &= 0xF0;
            section.dword <<= 4;
            section.dword |= offset;

            wordSize.dword &= 0xF;
            wordSize.dword += 2;

            for (unsigned int i=0; i<=wordSize.dword; i++)
            {
                data = dictionary[(section.dword + i) & 0xFFF];
                p_destBuffer[dest_index++] = data;
                dictionary[indexDictionary++] = data;
                indexDictionary &= 0xFFF;
            }
         }
     }
     return dest_index;
}
