#ifndef GDSREADER_H
#define GDSREADER_H

#include <QStringList>
#include <QApplication>

const int GDS_HEADER = 0x0002;
const int GDS_BGNLIB = 0x0102;
const int GDS_LIBNAME = 0x0206;
const int GDS_UNITS = 0x0305;
const int GDS_ENDLIB = 0x0400;
const int GDS_BGNSTR = 0x0502;
const int GDS_STRNAME = 0x0606;
const int GDS_ENDSTR = 0x0700;
const int GDS_BOUNDARY = 0x0800;
const int GDS_PATH = 0x0900;
const int GDS_SREF = 0x0a00;
const int GDS_AREF = 0x0b00;
const int GDS_TEXT = 0x0c00;
const int GDS_LAYER = 0x0d02;
const int GDS_DATATYPE = 0x0e02;
const int GDS_WIDTH = 0x0f03;
const int GDS_XY = 0x1003;
const int GDS_ENDEL = 0x1100;
const int GDS_SNAME = 0x1206;
const int GDS_COLROW = 0x1302;
const int GDS_NODE = 0x1500;
const int GDS_TEXTTYPE = 0x1602;
const int GDS_PRESENTATION = 0x1701;
const int GDS_STRING = 0x1906;
const int GDS_STRANS = 0x1a01;
const int GDS_MAG = 0x1b05;
const int GDS_ANGLE = 0x1c05;
const int GDS_REFLIBS = 0x1f06;
const int GDS_FONTS = 0x2006;
const int GDS_PATHTYPE = 0x2102;
const int GDS_GENERATIONS = 0x2202;
const int GDS_ATTRTABLE = 0x2306;
const int GDS_EFLAGS = 0x2601;
const int GDS_NODETYPE = 0x2a02;
const int GDS_PROPATTR = 0x2b02;
const int GDS_PROPVALUE = 0x2c06;
const int GDS_BOX = 0x2d00;
const int GDS_BOXTYPE = 0x2e02;
const int GDS_PLEX = 0x2f03;

//*********************************************************************************************************************
// GdsReader
//*********************************************************************************************************************
class GdsReader
{
public:
    GdsReader(const QString &fileName);

    void                        gdsCreate(const QString &);
    QStringList                 getErrors() const;

private:
    void                        gdsEnd() const;
    int*                        gdsTime() const;
    void                        gdsBegin(const QString &) const;    

    void                        writeUnits() const;
    int                         writeRec(int record) const;
    int                         writeStr(int record, std::string inStr) const;
    int                         writeInt(int record, int arrInt[], int cnt) const;

private:
    FILE*                       m_gdsFile;
    QString                     m_fileName;
    mutable QStringList         m_errorList;
};

//*********************************************************************************************************************
// GdsReader::getErrors()
//*********************************************************************************************************************
inline QStringList GdsReader::getErrors() const
{
    return m_errorList;
}

#endif // GDSREADER_H
