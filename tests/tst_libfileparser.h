#ifndef TST_LIBFILEPARSER_H
#define TST_LIBFILEPARSER_H

#include <QtTest/QtTest>

class LibFileParserTest : public QObject
{
    Q_OBJECT

private slots:
    void parseFile_missingFile_returnsError();
    void parseFile_singleDefineWithExplicitName_parsesDefinition();
    void parseFile_singleDefineWithoutExplicitName_infersNameFromPath();
    void parseFile_include_parsesIncludedFile();
    void parseFile_recursiveInclude_returnsError();
    void parseFile_invalidStatement_returnsError();
    void parseFile_unknownFunction_returnsError();
    void parseFile_defineWithOptions_parsesOptions();
    void parseFile_duplicateNamedOption_returnsError();
    void parseFile_invalidStringList_returnsError();

    void parseFile_pathIsDirectory_returnsError();
    void parseFile_secondParse_skipsAlreadyParsedFile();
    void parseFile_defineEmptyArgs_returnsError();
    void parseFile_defineTooManyPositionals_returnsError();
    void parseFile_trailingTokensAfterStatement_returnsError();
    void parseFile_technologyNonString_returnsError();
    void parseFile_replicateFalse_parses();
    void parseFile_includeWrongArgCount_returnsError();
};

#endif // TST_LIBFILEPARSER_H
