#include <windows.h>
#include <stdio.h>
#include <Winsvc.h>
#include <tchar.h>



SERVICE_STATUS ServiceStatus; 
SERVICE_STATUS_HANDLE hStatus; 
void  ServiceMain(int argc, char** argv); 
void  ControlHandler(DWORD request); 
HANDLE stopServiceEvent = 0;
#define SERVICE_NAME  _T("Test Service V7")





/*
 * Function:  ServiceMain 
 * -----------------------------
 * Entry point of a service , It runs in a separate thread
 *
 *  param:  service name &  pointer to the ControlHandlerfunction
 */

void ServiceMain(int argc, char** argv) 
{ 
 
    ServiceStatus.dwServiceType        = SERVICE_WIN32_OWN_PROCESS; 
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    ServiceStatus.dwWin32ExitCode      = 0; 
    ServiceStatus.dwServiceSpecificExitCode = 0; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    hStatus = RegisterServiceCtrlHandler(
		SERVICE_NAME, 
		(LPHANDLER_FUNCTION)ControlHandler); 



	    if (hStatus == (SERVICE_STATUS_HANDLE)0)
    {

        ServiceStatus.dwCurrentState       = SERVICE_STOPPED;
        ServiceStatus.dwWin32ExitCode      = -1; 
        SetServiceStatus(hStatus, &ServiceStatus); 
        return; 
    } 


   
		ServiceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus( hStatus, &ServiceStatus );

		 
		HANDLE    stopServiceEvent = CreateEvent( 0, FALSE, FALSE, 0 );



		do
		{
			Beep( 1000, 100 );
		}
		while ( WaitForSingleObject( stopServiceEvent, 5000 ) == WAIT_TIMEOUT );

		ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus( hStatus, &ServiceStatus );

		CloseHandle( stopServiceEvent );
		stopServiceEvent = 0;

		ServiceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus( hStatus, &ServiceStatus );
	return; 

    
}
 





/*
 * Function:  ControlHandler 
 * --------------------
 * It checks what request was sent by the SCM and acts accordingly
 *
 *  param: SCM control request
 *
 */
void ControlHandler(DWORD request) 
{ 
    switch(request) 
    { 
        case SERVICE_CONTROL_STOP: 
           

            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            SetServiceStatus (hStatus, &ServiceStatus);
            return; 
 
        case SERVICE_CONTROL_SHUTDOWN: 
            

            ServiceStatus.dwWin32ExitCode = 0; 
            ServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            SetServiceStatus (hStatus, &ServiceStatus);
            return; 
        
        default:
            break;
    } 
 
    SetServiceStatus (hStatus,  &ServiceStatus);    // Report current status

    return; 
} 





/*
 * Function:  InstallService 
 * --------------------
 * Installing and configuring a Service
 */
void InstallService()
{
	 SC_HANDLE serviceControlManager = OpenSCManager( 0, 0, SC_MANAGER_CREATE_SERVICE ); /* connect to SCM and tell we intend to create a service */

	if ( serviceControlManager )
	{
		TCHAR path[ _MAX_PATH + 1 ];
		if ( GetModuleFileName( 0, path, sizeof(path)/sizeof(path[0]) ) > 0 )  /* get the name of the .exe that we are running as */

		{
			SC_HANDLE service = CreateService( serviceControlManager,    			/* register this executable as a service */
							SERVICE_NAME, SERVICE_NAME,
							SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
							SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
							0, 0, 0, 0, 0 );
			if ( service )
				CloseServiceHandle( service );
		}

		CloseServiceHandle( serviceControlManager );
	}
}






///*
// * Function:  StartSvc 
// * --------------------
// *  Start the service after installation
// *    
// */
void StartSvc()
{
	SC_HANDLE serviceControlManager = OpenSCManager( 0, 0, SC_MANAGER_ALL_ACCESS );
	SC_HANDLE serviceHandle = OpenService(serviceControlManager,SERVICE_NAME , SERVICE_ALL_ACCESS );

	StartService(serviceHandle,0,NULL);

	CloseServiceHandle(serviceControlManager); 
        CloseServiceHandle(serviceHandle);
        return; 
}





int main()
{ 
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = SERVICE_NAME;
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;
    StartServiceCtrlDispatcher(ServiceTable);      // Start the control dispatcher thread for our service

	InstallService();
	StartSvc();
  
  	return 0;

}