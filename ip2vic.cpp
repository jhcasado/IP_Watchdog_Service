/* A skeleton for writing an NT/2K service */
/* Author :- Nishant S */
/* EMail :- nish@inapp.com */

#include <windows.h>
#include <winsvc.h>

#include <stdio.h>
#include "curl/curl.h"
#include "registry.h"

void ServiceMain(DWORD argc, LPTSTR *argv); 
void ServiceCtrlHandler(DWORD nControlCode);
BOOL UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode,
					 DWORD dwServiceSpecificExitCode, DWORD dwCheckPoint,
					 DWORD dwWaitHint);
BOOL StartServiceThread();
DWORD ServiceExecutionThread(LPDWORD param);
HANDLE hServiceThread;
void KillService();

char *strServiceName = "ip2vic";
SERVICE_STATUS_HANDLE nServiceStatusHandle; 
HANDLE killServiceEvent;
BOOL nServiceRunning;
DWORD nServiceCurrentStatus;


int Ip2vic()
{
      CURL *curl;
      CURLcode res;
      char post[255];

      registry regvic("Software\\CuantoBit\\ip2vic");
      if(!regvic)
          return 0;
      registry::iterator cliente=regvic["cliente"];
      registry::iterator sucursal=regvic["sucursal"];
      registry::iterator web=regvic["web"];
      
      if (strcmp((const char *)web, "") == 0) return 0;
      if (strcmp((const char *)cliente, "") == 0) return 0;
      if (strcmp((const char *)sucursal, "") == 0) return 0;
      
      sprintf(post, "cliente=%s&sucursal=%s", (const char *)cliente, (const char *)sucursal);
	
      curl = curl_easy_init();
      if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, (const char *)web);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
      }
}

int InstalarServicio()
{
	SC_HANDLE ip2vic,scm;
    TCHAR szPath[MAX_PATH];

    if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
    {
        printf("Error instalando servicio: problema con la ruta (%d)\n", GetLastError());
        return 1;
    }
    
    strcat(szPath, " servidor");

	scm=OpenSCManager(0,0,SC_MANAGER_CREATE_SERVICE);
	if(!scm)
	{
		printf("Error instalando servicio: no se puede acceder al Service Manager");
		return 1;
	}
	ip2vic=CreateService(scm,"ip2vic",
		"CuantoBit remote access",
		SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
		SERVICE_ERROR_NORMAL,
		szPath,
		0,0,0,0,0);
	if(!ip2vic)
	{
       DWORD dw = GetLastError(); 
       printf("Error instalando servicio: no se ha podido crear el servicio ");
       if (dw == ERROR_ACCESS_DENIED)
            printf("(Acceso denegado).\n");
       if (dw == ERROR_INVALID_HANDLE)
            printf("(Manejador de Servicios invalido).\n");
       if (dw == ERROR_CIRCULAR_DEPENDENCY)
            printf("(Dependencia circular entre servicios).\n");
       if (dw == ERROR_DUPLICATE_SERVICE_NAME)
            printf("(Ya existe un servicio con esa descripcion).\n");
       if (dw == ERROR_INVALID_NAME)
            printf("(Nombre del nuevo servicio incorrecto).\n");
       if (dw == ERROR_INVALID_PARAMETER)
            printf("(Parametros incorrectos).\n");
       if (dw == ERROR_INVALID_SERVICE_ACCOUNT)
            printf("(No existe la cuenta de usuario).\n");
       if (dw == ERROR_SERVICE_EXISTS)
            printf("(Ya existe un servicio con el mismo nombre).\n");
	   CloseServiceHandle(scm);
       return 1;
	}
	if (!StartService(ip2vic, 0, NULL))
	{
       DWORD dw = GetLastError(); 
       printf("Error instalando servicio: no se ha podido iniciar el servicio ");
       if (dw == ERROR_ACCESS_DENIED)
            printf("(Acceso denegado).\n");
       if (dw == ERROR_INVALID_HANDLE)
            printf("(Manejador de Servicios invalido).\n");
       if (dw == ERROR_PATH_NOT_FOUND)
            printf("(ruta no encontrada).\n");

       if (dw == ERROR_SERVICE_ALREADY_RUNNING)
            printf("(servicio ya iniciado).\n");
       if (dw == ERROR_SERVICE_DATABASE_LOCKED)
            printf("(base de datos bloqueada).\n");
       if (dw == ERROR_SERVICE_DEPENDENCY_DELETED)
            printf("(error de dependencia de borrado).\n");
       if (dw == ERROR_SERVICE_DEPENDENCY_FAIL)
            printf("(error de dependencia).\n");
       if (dw == ERROR_SERVICE_DISABLED)
            printf("(servicio deshabilitado).\n");
       if (dw == ERROR_SERVICE_LOGON_FAILED)
            printf("(error de login).\n");
       if (dw == ERROR_SERVICE_MARKED_FOR_DELETE)
            printf("(servicio marcado para borrar).\n");
       if (dw == ERROR_SERVICE_NO_THREAD)
            printf("(no se puede crear un hilo de ejecucion).\n");
       if (dw == ERROR_SERVICE_REQUEST_TIMEOUT)
            printf("(peticion timeout).\n");

       if (dw == ERROR_CIRCULAR_DEPENDENCY)
            printf("(Dependencia circular entre servicios).\n");
       if (dw == ERROR_DUPLICATE_SERVICE_NAME)
            printf("(Ya existe un servicio con esa descripcion).\n");
       if (dw == ERROR_INVALID_NAME)
            printf("(Nombre del nuevo servicio incorrecto).\n");
       if (dw == ERROR_INVALID_PARAMETER)
            printf("(Parametros incorrectos).\n");
       if (dw == ERROR_INVALID_SERVICE_ACCOUNT)
            printf("(No existe la cuenta de usuario).\n");
       if (dw == ERROR_SERVICE_EXISTS)
            printf("(Ya existe un servicio con el mismo nombre).\n");
       CloseServiceHandle(ip2vic);
       CloseServiceHandle(scm);
       return 1;
    }
	CloseServiceHandle(ip2vic);
	CloseServiceHandle(scm);
    printf("Servicio instalado.\n");
	return 0;
}

int DesinstalarServicio()
{
    LPSERVICE_STATUS *estado = new(LPSERVICE_STATUS);
	SC_HANDLE ip2vic,scm;
	scm=OpenSCManager(0,0,SC_MANAGER_ALL_ACCESS);
	if(!scm)
	{
		printf("Error desinstalando servicio: no se puede acceder al Service Manager\n");
		return 1;
	}

	ip2vic=OpenService(scm,"ip2vic",DELETE|SERVICE_STOP);
	if(!ip2vic)
	{
		CloseServiceHandle(scm);
        printf("Error desinstalando servicio: servicio no encontrado\n");
		return 1;
	}

    ControlService(ip2vic, SERVICE_CONTROL_STOP, *estado);
    /*
	if (!ControlService(ip2vic, SERVICE_CONTROL_STOP, *estado))
	{
       DWORD dw = GetLastError(); 
       printf("Error desinstalando servicio: no se ha podido detener el servicio.");
       CloseServiceHandle(scm);
       return 1;
    }
    */

	if (!DeleteService(ip2vic))
    {
       DWORD dw = GetLastError(); 
       printf("Error desinstalando servicio: no se ha podido borrar ");
       if (dw == ERROR_ACCESS_DENIED)
            printf("(Acceso denegado).\n");
       if (dw == ERROR_INVALID_HANDLE)
            printf("(Manejador invalido).\n");
       if (dw == ERROR_SERVICE_MARKED_FOR_DELETE)
            printf("(Servicio marcado para borrar).\n");
            
   	   CloseServiceHandle(scm);
   	   return 1;
    }
	
	CloseServiceHandle(scm);
    printf("Servicio borrado.\n");
	return 0;
}

int Registro()
{
	registry regvic("Software\\CuantoBit\\ip2vic");

	if(!regvic){
		printf("Error accediendo al registro\n");
		exit(1);
	}

	printf("\nHKLM\\SOFTWARE\\CuantoBit\\ip2vic\n\n");

	registry::iterator cliente=regvic["cliente"];
	printf("Cliente: \"%s\"\n",(const char*)cliente);
	registry::iterator sucursal=regvic["sucursal"];
	printf("Sucursal: \"%s\"\n",(const char*)sucursal);
	registry::iterator web=regvic["web"];
	printf("Web: \"%s\"\n",(const char*)web);
	registry::iterator timer=regvic["timer"];
	printf("Timer: \"%s\" min.\n",(const char*)timer);

	return 0;
}
    
int Cliente(char * idcliente)
{
    char id[255];
    
    strncpy(id, idcliente, 254);
    
	registry regvic("Software\\CuantoBit\\ip2vic");
	if(!regvic){
		printf("Error accediendo al registro\n");
		exit(1);
	}

	registry::iterator cliente=regvic["cliente"];
	cliente = id;

	return 0;
}

int Sucursal(char * idsucursal)
{
    char id[255];
    
    strncpy(id, idsucursal, 254);
    
	registry regvic("Software\\CuantoBit\\ip2vic");
	if(!regvic){
		printf("Error accediendo al registro\n");
		exit(1);
	}

	registry::iterator sucursal=regvic["sucursal"];
	sucursal = id;

	return 0;
}

int Web(char * idweb)
{
    char id[255];
    
    strncpy(id, idweb, 254);
    
	registry regvic("Software\\CuantoBit\\ip2vic");
	if(!regvic){
		printf("Error accediendo al registro\n");
		exit(1);
	}

	registry::iterator web=regvic["web"];
	web = id;

	return 0;
}

int Timer(char * idtimer)
{
    char id[255];
    
    strncpy(id, idtimer, 254);
    
	registry regvic("Software\\CuantoBit\\ip2vic");
	if(!regvic){
		printf("Error accediendo al registro\n");
		exit(1);
	}

	registry::iterator timer=regvic["timer"];
	timer = id;

	return 0;
}

int main(int argc, char* argv[])
{
    if (argc > 1) {
       if (strcmp(argv[1], "instalar") == 0) {
          return InstalarServicio();
       }
       if (strcmp(argv[1], "desinstalar") == 0) {
          return DesinstalarServicio();
       }
       if (strcmp(argv[1], "instalarsetup") == 0) {
          int r = InstalarServicio();
          system("PAUSE");
          return r;
       }
       if (strcmp(argv[1], "desinstalarsetup") == 0) {
          int r = DesinstalarServicio(); 
          system("PAUSE");
          return r;
       }
       if (strcmp(argv[1], "ip2vic") == 0) {
          return Ip2vic();
       }
       if (strcmp(argv[1], "registro") == 0) {
          return Registro();
       }
       if (strcmp(argv[1], "cliente") == 0) {
          if (argc > 2)
              return Cliente(argv[2]);
       }
       if (strcmp(argv[1], "sucursal") == 0) {
          if (argc > 2)
              return Sucursal(argv[2]);
       }
       if (strcmp(argv[1], "web") == 0) {
          if (argc > 2)
              return Web(argv[2]);
       }
       if (strcmp(argv[1], "timer") == 0) {
          if (argc > 2)
              return Timer(argv[2]);
       }
       if (strcmp(argv[1], "servidor") == 0) {
        	SERVICE_TABLE_ENTRY servicetable[]=
        	{
        		{strServiceName,(LPSERVICE_MAIN_FUNCTION)ServiceMain},
        		{NULL,NULL}
        	};
        	BOOL success;
        	success=StartServiceCtrlDispatcher(servicetable);
        	if(!success)
        	{
        		//error
        	}
          return 0;
       }
    }
   printf ("Instala/Desinstala el servicio de control de ip's dinamicas.\n\n");
   printf ("ip2vic (instalar|desinstalar|ip2vic|registro|cliente X|sucursal X|web X|timer X)\n\n");
   printf ("   instalar       Instala el servicio\n");
   printf ("   desinstalar    Desinstala el servicio\n");
   printf ("   ip2vic         Manda la ip una vez sin ejecutarse como servicio\n");
   printf ("   registro       Visualizar los datos del registro cliente/sucursal\n");
   printf ("   cliente X      Cambia el valor del cliente en el registro\n");
   printf ("   sucursal X     Cambia el valor del sucursal en el registro\n");
   printf ("   web X          Cambia el valor de la web en el registro\n");
   printf ("   timer X        Cambia el valor del timer en minutos en el registro\n");
   printf ("\n\n");
   printf ("Este software ha sido desarrollado por CuantoBit.\n");
   return 0;       
}

void ServiceMain(DWORD argc, LPTSTR *argv)
{
	BOOL success;
	nServiceStatusHandle=RegisterServiceCtrlHandler(strServiceName,
		(LPHANDLER_FUNCTION)ServiceCtrlHandler);
	if(!nServiceStatusHandle)
	{
		return;
	}
	success=UpdateServiceStatus(SERVICE_START_PENDING,NO_ERROR,0,1,3000);
	if(!success)
	{
		return;
	}
	killServiceEvent=CreateEvent(0,TRUE,FALSE,0);
	if(killServiceEvent==NULL)
	{
		return;
	}
	success=UpdateServiceStatus(SERVICE_START_PENDING,NO_ERROR,0,2,1000);
	if(!success)
	{
		return;
	}
	success=StartServiceThread();
	if(!success)
	{
		return;
	}
	nServiceCurrentStatus=SERVICE_RUNNING;
	success=UpdateServiceStatus(SERVICE_RUNNING,NO_ERROR,0,0,0);
	if(!success)
	{
		return;
	}
	WaitForSingleObject(killServiceEvent,INFINITE);
	CloseHandle(killServiceEvent);
}



BOOL UpdateServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode,
					 DWORD dwServiceSpecificExitCode, DWORD dwCheckPoint,
					 DWORD dwWaitHint)
{
	BOOL success;
	SERVICE_STATUS nServiceStatus;
	nServiceStatus.dwServiceType=SERVICE_WIN32_OWN_PROCESS;
	nServiceStatus.dwCurrentState=dwCurrentState;
	if(dwCurrentState==SERVICE_START_PENDING)
	{
		nServiceStatus.dwControlsAccepted=0;
	}
	else
	{
		nServiceStatus.dwControlsAccepted=SERVICE_ACCEPT_STOP			
			|SERVICE_ACCEPT_SHUTDOWN;
	}
	if(dwServiceSpecificExitCode==0)
	{
		nServiceStatus.dwWin32ExitCode=dwWin32ExitCode;
	}
	else
	{
		nServiceStatus.dwWin32ExitCode=ERROR_SERVICE_SPECIFIC_ERROR;
	}
	nServiceStatus.dwServiceSpecificExitCode=dwServiceSpecificExitCode;
	nServiceStatus.dwCheckPoint=dwCheckPoint;
	nServiceStatus.dwWaitHint=dwWaitHint;

	success=SetServiceStatus(nServiceStatusHandle,&nServiceStatus);

	if(!success)
	{
		KillService();
		return success;
	}
	else
		return success;
}

BOOL StartServiceThread()
{	
	DWORD id;
	hServiceThread=CreateThread(0,0,
		(LPTHREAD_START_ROUTINE)ServiceExecutionThread,
		0,0,&id);
	if(hServiceThread==0)
	{
		return false;
	}
	else
	{
		nServiceRunning=true;
		return true;
	}
}

DWORD ServiceExecutionThread(LPDWORD param)
{
	while(nServiceRunning)
	{
      registry regvic("Software\\CuantoBit\\ip2vic");
      if(!regvic)
          return 0;
      registry::iterator stimer=regvic["timer"];
      
      int timer = 0;
      
      timer = atoi((const char *)stimer);
      
      if (timer < 1) timer = 10;

      Sleep(timer * 60 * 1000);

      Ip2vic();
      
      //Beep(450,150);
	}
	return 0;
}

void KillService()
{
	nServiceRunning=false;
	SetEvent(killServiceEvent);
	UpdateServiceStatus(SERVICE_STOPPED,NO_ERROR,0,0,0);
}

void ServiceCtrlHandler(DWORD nControlCode)
{
	BOOL success;
	switch(nControlCode)
	{	
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		nServiceCurrentStatus=SERVICE_STOP_PENDING;
		success=UpdateServiceStatus(SERVICE_STOP_PENDING,NO_ERROR,0,1,3000);
		KillService();		
		return;
	default:
		break;
	}
	UpdateServiceStatus(nServiceCurrentStatus,NO_ERROR,0,0,0);
}
