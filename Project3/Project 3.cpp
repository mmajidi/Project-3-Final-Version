#include <windows.h>
#include <stdio.h>
#include <Winsvc.h>



SERVICE_STATUS ServiceStatus; 
SERVICE_STATUS_HANDLE hStatus; 
void  ServiceMain(int argc, char** argv); 
void  ControlHandler(DWORD request); 
int InitService();
HANDLE stopServiceEvent = 0;





/*
 * Function:  ServiceMain 
 * -----------------------------
 * Entry point of a service , It runs in a separate thread
 *
 *  param:  service name &  pointer to the ControlHandlerfunction
 */

void WINAPI ServiceMain( DWORD /*argc*/, TCHAR* /*argv*/[] )
{ 
 
    ServiceStatus.dwServiceType        = SERVICE_WIN32; 
    ServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    ServiceStatus.dwControlsAccepted   = 0;
    ServiceStatus.dwWin32ExitCode      = NO_ERROR; 
    ServiceStatus.dwServiceSpecificExitCode = NO_ERROR; 
    ServiceStatus.dwCheckPoint         = 0; 
    ServiceStatus.dwWaitHint           = 0; 
 
    hStatus = RegisterServiceCtrlHandler(
		"Test Service B2", 
		(LPHANDLER_FUNCTION)ControlHandler); 


    if (hStatus) 
    { 
        		// service is starting
		ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
		SetServiceStatus( hStatus, &ServiceStatus );

		// do initialisation here
		stopServiceEvent = CreateEvent( 0, FALSE, FALSE, 0 );

				// running

				ServiceStatus.dwControlsAccepted |= (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus( hStatus, &ServiceStatus );



		do
		{
			Beep( 1000, 100 );
		}
		while ( WaitForSingleObject( stopServiceEvent, 5000 ) == WAIT_TIMEOUT );

		// service was stopped
		ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus( hStatus, &ServiceStatus );

		// do cleanup here
		CloseHandle( stopServiceEvent );
		stopServiceEvent = 0;

		// service is now stopped
		ServiceStatus.dwControlsAccepted &= ~(SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus( hStatus, &ServiceStatus );


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
	SC_HANDLE serviceHandle = OpenService(serviceControlManager,"Test Service B2" , SERVICE_ALL_ACCESS );

	StartService(serviceHandle,0,NULL);

	CloseServiceHandle(serviceControlManager); 
        CloseServiceHandle(serviceHandle);
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
							"Test Service B2", "Test Service B2",
							SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
							SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
							0, 0, 0, 0, 0 );
			if ( service )
				CloseServiceHandle( service );
		}

		CloseServiceHandle( serviceControlManager );
	}
}




int main()
{ 
    SERVICE_TABLE_ENTRY ServiceTable[2];
    ServiceTable[0].lpServiceName = "Test Service B2";
    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

    ServiceTable[1].lpServiceName = NULL;
    ServiceTable[1].lpServiceProc = NULL;
    StartServiceCtrlDispatcher(ServiceTable);      // Start the control dispatcher thread for our service

	InstallService();
	StartSvc();
	
  
  	return 0;

}