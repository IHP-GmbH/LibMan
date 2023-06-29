#include <bitset>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdio.h>
#include <string>
#include <vector>

#include "gdsreader.h"

//*********************************************************************************************************************
// GdsReader::GdsReader
//*********************************************************************************************************************
GdsReader::GdsReader(const QString &fileName)
    : m_fileName(fileName)
{
    m_errorList.clear();
}

//*********************************************************************************************************************
// GdsReader::gdsCreate
//*********************************************************************************************************************
void GdsReader::gdsCreate(const QString &cellName)
{
    if(m_fileName.isEmpty()) {
        return;
    }

    m_gdsFile = fopen(m_fileName.toStdString().c_str(), "wb");
    if(!m_gdsFile) {
        return;
    }

    gdsBegin(cellName);
    gdsEnd();

    fclose(m_gdsFile);
}

//*********************************************************************************************************************
// GdsReader::gdsBegin
//*********************************************************************************************************************
void GdsReader::gdsBegin(const QString &libName) const
{
  int tempArr[1];

  tempArr[0] = 600;
  writeInt(GDS_HEADER, tempArr, 1);
  writeInt(GDS_BGNLIB, gdsTime(), 12);
  writeStr(GDS_LIBNAME, libName.toStdString());
  writeUnits();
}

//*********************************************************************************************************************
// GdsReader::gdsEnd
//*********************************************************************************************************************
void GdsReader::gdsEnd() const
{
    writeRec(GDS_ENDLIB);
}

//*********************************************************************************************************************
// GdsReader::gsdTime
//*********************************************************************************************************************
int* GdsReader::gdsTime() const
{
    static int timeIO[12];
    static bool timeSet = false;
    tm lctn;

    if(!timeSet) {
        time_t now = time(0);
        lctn = *localtime(&now);
        timeSet = true;

        timeIO[0] = lctn.tm_year + 1900;
        timeIO[1] = lctn.tm_mon + 1;
        timeIO[2] = lctn.tm_mday;
        timeIO[3] = lctn.tm_hour;
        timeIO[4] = lctn.tm_min;
        timeIO[5] = lctn.tm_sec;
        timeIO[6] = timeIO[0];
        timeIO[7] = timeIO[1];
        timeIO[8] = timeIO[2];
        timeIO[9] = timeIO[3];
        timeIO[10] = timeIO[4];
        timeIO[11] = timeIO[5];
    }

    return timeIO;
}

//*********************************************************************************************************************
// GdsReader::writeInt
//*********************************************************************************************************************
int GdsReader::writeInt(int record, int arrInt[], int cnt) const
{
    unsigned int dataSize = record & 0xff;

    if (dataSize == 0x02 && cnt > 0) {
        dataSize = 2;
    } else if (dataSize == 0x03 && cnt > 0) {
        dataSize = 4;
    } else if (dataSize == 0x00 && cnt == 0) {
        dataSize = 0;
    } else {
        m_errorList<<QString("Incorrect parameters for record: 0x%1").arg(record);
        return 1;
    }

    unsigned int sizeByte = cnt * dataSize + 4;
    unsigned char outBuffer[4];

    outBuffer[0] = sizeByte >> 8 & 0xff;
    outBuffer[1] = sizeByte & 0xff;
    outBuffer[2] = record >> 8 & 0xff;
    outBuffer[3] = record & 0xff;
    fwrite(outBuffer, 1, 4, m_gdsFile);

    unsigned char dataOut[dataSize];

    for (int i = 0; i < cnt; i++) {
        for (unsigned int j = 0; j < dataSize; j++) {
            dataOut[j] = arrInt[i] >> (((dataSize - 1) * 8) - (j * 8)) & 0xff;
        }

        fwrite(dataOut, 1, dataSize, m_gdsFile);
    }

    return 0;
}

//*********************************************************************************************************************
// GdsReader::writeStr
//*********************************************************************************************************************
int GdsReader::writeStr(int record, std::string inStr) const
{
    if ((record & 0xff) != 0x06) {
        m_errorList<<QString("Incorrect record: 0x%1").arg(record);
        return 1;
    }

    unsigned char outBuffer[4];

    if (inStr.length() % 2 == 1) {
        inStr.push_back('\0');
    }

    int lenStr = inStr.length();

    outBuffer[0] = ((lenStr + 4) >> 8) & 0xff;
    outBuffer[1] = (lenStr + 4) & 0xff;
    outBuffer[2] = record >> 8 & 0xff;
    outBuffer[3] = record & 0xff;
    fwrite(outBuffer, 1, 4, m_gdsFile);

    char dataOut[lenStr];
    strcpy(dataOut, inStr.c_str());
    fwrite(dataOut, 1, lenStr, m_gdsFile);

    return 0;
}

//*********************************************************************************************************************
// GdsReader::writeUnits
//*********************************************************************************************************************
void GdsReader::writeUnits() const
{
    unsigned char data[20];

    data[0] = 0x00;
    data[1] = 0x14;
    data[2] = 0x03;
    data[3] = 0x05;
    data[4] = 0x3e;
    data[5] = 0x41;
    data[6] = 0x89;
    data[7] = 0x37;
    data[8] = 0x4b;
    data[9] = 0xc6;
    data[10] = 0xa7;
    data[11] = 0xf0;
    data[12] = 0x39;
    data[13] = 0x44;
    data[14] = 0xb8;
    data[15] = 0x2f;
    data[16] = 0xa0;
    data[17] = 0x9b;
    data[18] = 0x5a;
    data[19] = 0x50;

    fwrite(data, 1, 20, m_gdsFile);
}

//*********************************************************************************************************************
// GdsReader::writeRec
//*********************************************************************************************************************
int GdsReader::writeRec(int record) const
{
    unsigned char outBuffer[4];

    outBuffer[0] = 0;
    outBuffer[1] = 4;
    outBuffer[2] = record >> 8 & 0xff;
    outBuffer[3] = record & 0xff;

    if (outBuffer[3] != 0) {
        m_errorList<<QString("Record contains no data");
        return 1;
    }

    fwrite(outBuffer, 1, 4, m_gdsFile);

    return 0;
}
