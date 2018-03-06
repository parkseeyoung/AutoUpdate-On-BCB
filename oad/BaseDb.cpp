#include <vcl.h>
#include <Filectrl.hpp>
//---------------------------------------------------------------------------
#pragma hdrstop
#include "BaseDb.h"
#include "ShareMem.hpp"
#include <Registry.hpp>  
#pragma package(smart_init)
//---------------------------------------------------------------------------

CBaseDb::CBaseDb()
{
    dbConn = new TOraSession(NULL);
    dbQy1 = NULL;
    dbQy2 = NULL;
    dbQy3 = NULL;
    dbQy4 = NULL;
    flag = false;
    dbConn->Connected = false;
}
CBaseDb::~CBaseDb()
{
    if (dbConn)
    {
        //dbConn->Close();
        dbConn->Options->Net = false;
        dbConn->Disconnect();
        delete dbConn;
    }
    if (dbQy1)
    {
        dbQy1->Close();
        delete dbQy1;
    }
    if (dbQy2)
    {
        dbQy2->Close();
        delete dbQy2;
    }
    if (dbQy3)
    {
        dbQy3->Close();
        delete dbQy3;
    }
    if (dbQy4)
    {
        dbQy4->Close();
        delete dbQy4;
    }
}

//---------------------------------------------------------------------------
void __fastcall CBaseDb:: OpenSql(TOraQuery *Qy,AnsiString sqlSt)
//void __fastcall CBaseDb:: OpenSql(TADOQuery *Qy,AnsiString sqlSt)
{
    Qy->Session = this->dbConn;
    Qy->Close();
    Qy->SQL->Text = sqlSt;
    Qy->Open();
}
//---------------------------------------------------------------------------
int __fastcall CBaseDb:: ExecSql(TOraQuery *Qy,AnsiString sqlSt)
//void __fastcall CBaseDb:: ExecSql(TADOQuery *Qy,AnsiString sqlSt)
{
    int i;
    try
    {
        Qy->Session = this->dbConn;
        Qy->Close();
        Qy->SQL->Text = sqlSt;
        Qy->ExecSQL();
        i = 1;
    }
    catch(...)
    {
        i = 0;
    }
    return i;
}
bool CBaseDb::GetSqlFromOracle(AnsiString TableName, AnsiString OraTabName, TStringList * FieldNameList, TStringList * AccessFieldTypeList, TStringList *SqlList)
{
    AnsiString tmpstr = "", sql = "";
    try
    {
        dbQy2->Close();
        tmpstr = "select * from \"" + OraTabName+"\"";
        dbQy2->SQL->Text = tmpstr;
        dbQy2->ReadOnly = true;
        dbQy2->Open();
    }catch(...)
    {
        Application->MessageBox(tmpstr.c_str(), "oracle数据库 sql 获取数据集失败", MB_OK);
        return false;
    }

    while(!dbQy2->Eof)
    {
        sql = "insert into " + TableName + "(";
        
        for(int i=0; i<FieldNameList->Count; i++)
        {     
            AnsiString temfieldname = FieldNameList->Strings[i];

            sql = sql + "[" + temfieldname + "]";

            if(i<FieldNameList->Count-1)
            {
                sql = sql + ",";
            }
        }
        sql = sql + ") values(";
        for(int i=0; i<FieldNameList->Count; i++)
        {
            AnsiString temfieldname = FieldNameList->Strings[i];
            AnsiString temfieldType = AccessFieldTypeList->Strings[i];
            AnsiString sFieldValue;
            if(temfieldType != "二进制大对象")
            {
                if(!dbQy2->FieldByName(temfieldname)->IsNull)
                {
                    sFieldValue = dbQy2->FieldByName(temfieldname)->AsString;
                    if(sFieldValue.Pos("'"))
                    {
                        sFieldValue = sFieldValue.SubString(1,sFieldValue.Pos("'")-1)+"''"+sFieldValue.SubString(sFieldValue.Pos("'")+1,sFieldValue.Length()-sFieldValue.Pos("'"));
                    }
                    sFieldValue = StringReplace( sFieldValue, "\'", "\'\'", TReplaceFlags() << rfReplaceAll );

                    if(temfieldType == "日期类型")
                    {
                        if(sFieldValue.Pos(":"))
                        {
                            //DateTime dt = StrToDateTime(sFieldValue);
                            sFieldValue = FormatDateTime( "yyyy-mm-dd hh:mm:ss", sFieldValue );
                            sql = sql + "to_date(\'" + sFieldValue + "\','yyyy-mm-dd hh24:mi:ss')";
                        }
                        else
                            sql = sql + "to_date(\'" + sFieldValue + "\','yyyy-mm-dd')";
                    }
                    else if( temfieldType == "整型数字" || temfieldType == "浮点数字" )
                    {
                        sql = sql +  sFieldValue ;
                    }
                    else
                    {
                        sql = sql + "\'" + sFieldValue + "\'";
                    }
                }
                else
                {
                    if(temfieldType == "日期类型")
                    {
                        sql = sql + "to_date(\'1899-12-31\','yyyy-mm-dd')";
                    }
                    else if( temfieldType == "整型数字" || temfieldType == "浮点数字" )
                    {
                        sql = sql +  0 ;
                    }
                    else
                    {
                        sql = sql + "\'\'";

                    }
                }
                sql = sql + ",";
            }
            else
            {
              //  sql = sql + "\'\'";
            }

            if(i<FieldNameList->Count-1)
            {
//                sql = sql + ",";
            }
        }
        sql = sql.SubString(1,sql.Length()-1) + ")";

        SqlList->Add(sql);

        dbQy2->Next();
    }

    return true;  
}

//---------------------------------------------------------------------------
AnsiString __fastcall CBaseDb:: GetLh(AnsiString StationName,AnsiString XbName)
{
    AnsiString sqlStr="";
    try
    {
        sqlStr = "select \"Lh\" from \"MangeUnit\" where \"Name\" = '"+StationName+"' and \"Xb\" = '"+XbName+"'";
        OpenSql( dbQy1, sqlStr);
        if (!dbQy1->Eof)
            return dbQy1->FieldByName("Lh")->AsString;
    }
    catch ( ... )
    {
        return "";
    }
    return "";        
}
//---------------------------------------------------------------------------
//得到注册表中字符串值。
AnsiString __fastcall CBaseDb::GetRegString(AnsiString Key,AnsiString Item)
{
    AnsiString S="";
    TRegistry *Reg;
    Reg=new TRegistry;
    try
    {
        Reg->RootKey = HKEY_LOCAL_MACHINE;
        Reg->OpenKey(Key, false);
        if (Reg->ValueExists(Item))
            S = Reg->ReadString(Item);
    }
    __finally
    {
        delete Reg;
    }
    return S;
}

//---------------------------------------------------------------------------
//1．	初始化数据库
bool __fastcall CBaseDb::connectToOracle(AnsiString username, AnsiString password, AnsiString server)
//bool __fastcall CBaseDb::SetDataPath(AnsiString DBPath)
{
    try
    {
            //dbConn =
            //dbConn->Options->Direct = True;
            dbConn->Disconnect();
            dbConn->Options->Net = true;
            dbConn->Username = username; //forexample: "manager";
            dbConn->Password = password; //forexample: "zbxhzbxh";
            dbConn->Server = server;     //forexample: "192.168.0.169:1521:zbxh";
            dbConn->Options->ConvertEOL = true;
            dbConn->Connect();

            dbQy1 = new TOraQuery(NULL);
            dbQy1->Connection = dbConn;
            dbQy2 = new TOraQuery(NULL);
            dbQy2->Connection = dbConn;
            dbQy3 = new TOraQuery(NULL);
            dbQy3->Connection = dbConn;
            dbQy4 = new TOraQuery(NULL);
            dbQy4->Connection = dbConn;
    }
    catch (Exception &exception)
    {
//        Application->ShowException(&exception);
        flag = false;
        return false;
    }
    flag = true;
    return true;        
}
//--------------------------------------------------------------------------
bool CBaseDb::IsOracleConnected()
{
    flag = dbConn->Connected;
    return flag;
}
//--------------------------------------------------------------------------
void CBaseDb::DisOraConn()
{
    dbConn->Options->Net = false;
    dbConn->Disconnect();
}
//---------------------------------------------------------------------------
bool CBaseDb::GetOracleTableNameList(TStringList * TableNameList)
{
   // dbConn->GetTableNames(TableNameList,false,true);

    AnsiString sqlStr = "select * from user_tables";
    OpenSql( dbQy1, sqlStr);

    TableNameList->Clear();
    while (!dbQy1->Eof)
    {
        AnsiString temps = dbQy1->FieldByName("table_name")->AsString;
        temps = temps.UpperCase();
        TableNameList->Add(temps);
        dbQy1->Next();
    }
    return true;
}
//-------------------------------------------------------------------------
bool CBaseDb::GetOracleTableStructInfo(AnsiString TableName, TStringList * FieldNameList, TStringList *FieldTypeList, TStringList *FieldSizeList)
{                               //字段名称，字段类型，字段长度，是否允许为空
    //AnsiString sqlStr = "select \"column_name\", \"data_type\", \"data_length\", \"nullable\" from all_tab_columns where \"table_name\" = '" + TableName + "'";
    AnsiString sqlStr = "select  distinct column_name, data_type, data_length from all_tab_columns where table_name = '" + TableName + "' order by column_name";
    //AnsiString sqlStr = "select column_name, data_type, data_length, nullable from all_tab_columns where table_name = 'BRIDGEINFO'";
    OpenSql( dbQy1, sqlStr);

    FieldNameList->Clear();
    while (!dbQy1->Eof)
    {
        AnsiString tempcolumn_name = dbQy1->FieldByName("column_name")->AsString;
        AnsiString tempdata_type = dbQy1->FieldByName("data_type")->AsString;
        AnsiString tempdata_length = dbQy1->FieldByName("data_length")->AsString;

        FieldNameList->Add(tempcolumn_name.UpperCase());
        FieldTypeList->Add(tempdata_type.UpperCase());
        FieldSizeList->Add(tempdata_length);
        dbQy1->Next();
    }
    return true;
}
AnsiString CBaseDb::GetOracleTableStructInfo(AnsiString TableName, AnsiString  FieldName)
{
    //AnsiString sqlStr = "select \"column_name\", \"data_type\", \"data_length\", \"nullable\",\"column_id\" from all_tab_columns where \"table_name\" = '" + TableName + "'";
    AnsiString sqlStr = "select data_type from all_tab_columns where table_name = '" + TableName + "' and column_name='"+FieldName+"'";
    //AnsiString sqlStr = "select column_name, data_type, data_length, nullable from all_tab_columns where table_name = 'BRIDGEINFO'";
    OpenSql( dbQy1, sqlStr);
    AnsiString tempdata_type;
    if (!dbQy1->Eof)
    {

        tempdata_type = dbQy1->FieldByName("data_type")->AsString;

    }
    return tempdata_type;
}
//--------------------------------------------------------------------------------------
bool CBaseDb::ToExecOracleSql(AnsiString sqlSt1)
{
    try
    {
        ExecSql(dbQy1, sqlSt1);
    }catch(Exception &e)
    {
        return false;
    }
    return true;
}
//-------------------------------------------------------------------------------------
bool CBaseDb::CreateOracleTableStructInfo(AnsiString TableName, TStringList *FieldNameList, TStringList *FieldTypeList, TStringList *FieldSizeList)
{
    AnsiString sqlstr = "create table " + TableName + "(";

    for(int i=0; i<FieldNameList->Count; i++)
    {
        AnsiString FieldName = FieldNameList->Strings[i];
        AnsiString FieldType = FieldTypeList->Strings[i];
        AnsiString FieldSize = FieldSizeList->Strings[i];

        AnsiString tempstr = GetFieldToOracleSql(FieldName, FieldType, FieldSize,TableName);
        if(tempstr == "无效类型")
        {
            AnsiString errstr = "创建 oracle 数据库 表格：" + TableName + " 失败，该表格的字段： " + FieldName + " 字段类型为<无效类型>.";
            //MessageBox(NULL, errstr.c_str(), "提示" ,MB_OK);
         //   ShowMessage(errstr);
            return false;
        }

        sqlstr = sqlstr +" "+ tempstr;
        if(i < FieldNameList->Count -1)
        {
            sqlstr = sqlstr + ",";
        }
    }
    sqlstr = sqlstr + ")";
    return this->ToExecOracleSql(sqlstr);
}
//---------------------------------------------------------------------------
AnsiString CBaseDb::GetFieldToOracleSql(AnsiString FieldName, AnsiString FieldType, AnsiString FieldSize,AnsiString TabName)
{
    AnsiString sqlstr = "";

    if(FieldType == "VARCHAR2" || FieldType == "NVARCHAR2")
        sqlstr = "\"" + FieldName + "\" varchar2(" + FieldSize + ")";
    else if(FieldType == "NUMBER")
        sqlstr = "\"" + FieldName + "\" number";     //整数部分取值范围是1~38，小数部分取值范围是-84~127。
    else if(FieldType == "NUMBER" && FieldSize == "1" )
        sqlstr = "\"" + FieldName + "\" number(1)";         //可以存储”Y”/”N”或者”T”/”F”的单个字符(使用VARCHAR2(1)类型)，或者存储1/0的单个数值(使用NUMBER(1)类型)。
    else if(FieldType == "RAW")
        sqlstr = "\"" + FieldName + "\" RAW(16)";
    else
        sqlstr =  "\"" + FieldName + "\" "+FieldType;
    return sqlstr;

}
