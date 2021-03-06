/**
 * \file test-installer-lib-sandbox.cpp
 *
 * Automatic testing of many of the units within the custom action is infeasible.
 * In one case, they rely on the execution environment within an installation session.
 * In another, they rely on the operation system environment as a whole.
 * In these cases, it's easier to verify behavior manually.
 *
 * This file contains a custom action function sandbox() as well as a number of test functions.
 * At any given time, not all of the test functions need to be referenced within the body of custom action.
 */

#include <sstream>
#include <functional>

#include "session.h"
#include "property.h"
#include "database.h"
#include "process.h"
#include "interaction.h"

//-------------------------------------------------------
// log_all_window_handles
//-------------------------------------------------------
class log_single_window_handle
{
  ImmediateSession & session ;

public:
  log_single_window_handle( ImmediateSession & session )
    : session( session )
  {
  }

  bool operator()( HWND window )
  {
    std::stringstream s ;
    s << "Window handle 0x" << std::hex << window ;
    session.Log( s.str() ) ;
    return true ;
  }
} ;

void log_all_window_handles( ImmediateSession & session )
{
  session.Log( "log_all_window_handles" ) ;
  log_single_window_handle lp( session ) ;
  EnumerateWindows( lp ) ;
}

//-------------------------------------------------------
// log_IE_window_handles
//-------------------------------------------------------
class log_single_window_handle_only_if_IE
{
  ImmediateSession & session ;

  ProcessCloser & pc ;

public:
  log_single_window_handle_only_if_IE( ImmediateSession & session, ProcessCloser & pc )
    : session( session ), pc( pc )
  {
  }

  bool operator()( HWND window )
  {
    DWORD pid = CreatorProcess( window ) ;
    if ( pc.Contains( pid ) )
    {
      std::stringstream s ;
      s << "Window handle 0x" << std::hex << window ;
      session.Log( s.str() ) ;
    }
    return true ;
  }
} ;

void log_IE_window_handles( ImmediateSession & session )
{
  session.Log( "log_IE_window_handles" ) ;
  const wchar_t * IE_names[] = { L"IExplore.exe", L"AdblockPlusEngine.exe" } ;
  ProcessSnapshot snapshot ;
  ProcessCloser iec(snapshot, IE_names) ;
  log_single_window_handle_only_if_IE lp( session, iec ) ;
  EnumerateWindows( lp ) ;
}

//-------------------------------------------------------
// log_only_window_handle_in_closer
//-------------------------------------------------------
void log_only_window_handle_in_closer( ImmediateSession & session )
{
  session.Log( "log_only_window_handle_in_closer" ) ;
  const wchar_t * IE_names[] = { L"IExplore.exe", L"AdblockPlusEngine.exe" } ;
  ProcessSnapshot snapshot ;
  ProcessCloser iec( snapshot, IE_names) ;
  iec.IterateOurWindows( log_single_window_handle( session ) ) ;
}

//-------------------------------------------------------
// sandbox
//-------------------------------------------------------
/**
 * Exposed DLL entry point for custom action. 
 * The function signature matches the calling convention used by Windows Installer.

 * \param[in] session_handle
 *     Windows installer session handle
 *
 * \return 
 *    An integer interpreted as a custom action return value.
 *   
 * \sa
 *   - MSDN [Custom Action Return Values](http://msdn.microsoft.com/en-us/library/aa368072%28v=vs.85%29.aspx)
 */
extern "C" UINT __stdcall 
sandbox( MSIHANDLE session_handle )
{
  ImmediateSession session( session_handle, "sandbox" ) ;
   
  try
  {
    session.Log( "Sandbox timestamp " __TIMESTAMP__ ) ;
    log_only_window_handle_in_closer( session ) ;
  }
  catch( std::exception & e )
  {
    session.LogNoexcept( "terminated by exception: " + std::string( e.what() ) ) ;
    return ERROR_INSTALL_FAILURE ;
  }
  catch( ... )
  {
    session.LogNoexcept( "Caught an exception" ) ;
    return ERROR_INSTALL_FAILURE ;
  }

  return ERROR_SUCCESS ;
}

