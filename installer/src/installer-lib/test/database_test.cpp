#include <gtest/gtest.h>
#include "../database.h"

/*
 * Note: These tests need to be run in the same directory as the MSI file.
 * There's a batch file for that purpose "run-tests.cmd".
 */

TEST( Database, open )
{
  FileSystemDatabase db( L"test-installer-lib.msi" ) ;
}

class Database_F
  : public ::testing::Test
{
protected:
  FileSystemDatabase db ;

  Database_F()
    : db( L"test-installer-lib.msi" )
  {}
} ;

TEST_F( Database_F, view_n_columns_and_n_rows )
{
  View v( db, L"SELECT * FROM AbpUIText" ) ;
  Record r( v.First() ) ;
  unsigned int j ;
  for ( j = 0 ; r != v.End() ; ++ j )
  {
    ASSERT_EQ( 3, r.NumberOfFields() ) ;
    r = v.Next() ;
  }
  ASSERT_EQ( 4, j ) ;
}

TEST_F( Database_F, view_single_record )
{
  View v( db, L"SELECT `content` FROM `AbpUIText` WHERE `component`='close_ie' and `id`='dialog_unknown'" ) ;
  Record r( v.First() ) ;
  ASSERT_EQ( 1, r.NumberOfFields() ) ;
  std::wstring s( r.ValueString( 1 ) ) ;
  std::wstring expected( L"IE is still running" ) ;
  ASSERT_GT( s.length(), expected.length() ) ;
  std::wstring prefix( s.substr( 0, expected.length() ) ) ;
  ASSERT_EQ( prefix, expected ) ;
  r = v.Next() ;
  ASSERT_EQ( v.End(), r ) ;
}

TEST_F( Database_F, view_single_record_parametric )
{
  View v( db, L"SELECT `content` FROM `AbpUIText` WHERE `component`='close_ie' and `id`=?" ) ;
  Record arg( 1 ) ;
  arg.AssignString( 1, L"dialog_unknown" ) ;
  Record r( v.First( arg ) ) ;
  ASSERT_EQ( 1, r.NumberOfFields() ) ;
  std::wstring s( r.ValueString( 1 ) ) ;
  std::wstring expected( L"IE is still running" ) ;
  ASSERT_GT( s.length(), expected.length() ) ;
  std::wstring prefix( s.substr( 0, expected.length() ) ) ;
  ASSERT_EQ( prefix, expected ) ;
  r = v.Next() ;
  ASSERT_EQ( v.End(), r ) ;
}
