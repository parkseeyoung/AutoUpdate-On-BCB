
//---------------------------------------------------------------------------
#ifndef BaseDbH
#define BaseDbH

#include <ADODB.hpp>

#include "DBAccess.hpp"
#include "Ora.hpp"
#include <DB.hpp>

class PACKAGE CBaseDb
{
private:

        AnsiString __fastcall GetLh(AnsiString StationName,AnsiString XbName);
        AnsiString __fastcall GetRegString(AnsiString Key,AnsiString Item);
        
        int __fastcall ExecSql(TOraQuery *Qy1,AnsiString sqlSt1);   //oracle
public:
///////////////////////////////////   接口  公共参数  //////////////////////////
        //构造函数
        CBaseDb();
        ~CBaseDb();

        TOraSession *dbConn;        //oracle 连接对象
        //1. 连接oracle
        bool __fastcall connectToOracle(AnsiString username, AnsiString password, AnsiString server);  //

        //2.  数据库结构
        bool GetOracleTableNameList(TStringList * TableNameList);
        bool GetOracleTableStructInfo(AnsiString TableName, TStringList * FieldNameList, TStringList *FieldTypeList, TStringList *FieldSizeList);
        AnsiString GetOracleTableStructInfo(AnsiString TableName, AnsiString  FieldName);
        bool CreateOracleTableStructInfo(AnsiString TableName, TStringList *FieldNameList, TStringList *FieldTypeList, TStringList *FieldSizeList);
        AnsiString GetFieldToOracleSql(AnsiString FieldName, AnsiString FieldType, AnsiString FieldSize,AnsiString TabName);
        //3. 读写操作
        bool ToExecOracleSql(AnsiString sqlSt1);
        void __fastcall OpenSql(TOraQuery *Qy1,AnsiString sqlSt1);   //oracle
        bool GetSqlFromOracle(AnsiString TableName, AnsiString OraTabName,TStringList * FieldNameList, TStringList * AccessFieldTypeList, TStringList *SqlList);
        TOraQuery *dbQy1;           //oracle
        TOraQuery *dbQy2;           //oracle
        TOraQuery *dbQy3;           //oracle
        TOraQuery *dbQy4;           //oracle

        bool IsOracleConnected();
        bool flag;
        void DisOraConn();    
};
//---------------------------------------------------------------------------
#endif
