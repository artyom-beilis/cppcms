///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2011  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
//                                                                             
//  This program is free software: you can redistribute it and/or modify       
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////
#define CPPCMS_SOURCE
#include "winservice.h"
#include <booster/nowide/convert.h>
#include <booster/system_error.h>
#include <cppcms/cppcms_error.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <winsvc.h>
#include <process.h>
#include <booster/function.h>
#include <booster/log.h>


namespace cppcms {
namespace impl {
	
winservice::winservice()
{
}

winservice::~winservice()
{
}

winservice &winservice::instance()
{
	static winservice inst;
	return inst;
}

void winservice::uninstall()
{
	std::wstring name = booster::nowide::convert(settings_.get<std::string>("winservice.name"));

	SC_HANDLE schm = OpenSCManagerW(0,0,SC_MANAGER_ALL_ACCESS);
	if(!schm) {
		throw booster::system::system_error(
			GetLastError(),
			booster::system::windows_category,
			"Failed to open sevice imager");
	}

	SC_HANDLE service_handle = OpenServiceW(schm,name.c_str(),DELETE);
	if(service_handle == NULL) {
		booster::system::error_code e(GetLastError(),booster::system::windows_category);
		CloseServiceHandle(schm);
		throw booster::system::system_error(e,"Failed to open the service");
	}
	if(!DeleteService(service_handle)) {
		booster::system::error_code e(GetLastError(),booster::system::windows_category);
		CloseServiceHandle(service_handle);
		CloseServiceHandle(schm);
		throw booster::system::system_error(e,"Failed to delete the service");
	}
	CloseServiceHandle(service_handle);
	CloseServiceHandle(schm);
	std::cout << "The service uninstalled sucessefully" << std::endl;
}

void winservice::install()
{
	std::wstring name = booster::nowide::convert(settings_.get<std::string>("winservice.name"));
	std::wstring display_name = booster::nowide::convert(settings_.get<std::string>("winservice.display_name"));
	std::string start_type = settings_.get("winservice.start","auto");
	std::wstring cmd_line;
	
	
	std::wstring user_name,password;
	wchar_t const *cuser_name=0,*cpassword=0;
	if(!settings_.find("winservice.username").is_undefined()) {
		user_name = booster::nowide::convert(settings_.get<std::string>("winservice.username"));
		cuser_name = user_name.c_str();
	}
	if(!settings_.find("winservice.password").is_undefined()) {
		password = booster::nowide::convert(settings_.get<std::string>("winservice.password"));
		cpassword = password.c_str();
	}

	wchar_t exe[ MAX_PATH + 1];
	if(GetModuleFileNameW(0,exe,MAX_PATH+1) == 0) {
		booster::system::error_code e(GetLastError(),booster::system::windows_category);
		throw booster::system::system_error(e,"Failed to get exe name");
	}
	cmd_line = L"\"";
	cmd_line +=exe;
	cmd_line +=L"\"";
	bool found = false;

	for(size_t i=1;i<args_.size();i++) {
		std::wstring parameter = booster::nowide::convert(args_[i]);
		if(parameter==L"--winservice-mode=install") {
			parameter = L"--winservice-mode=run";
			found = true;
		}
		cmd_line+=L" \"";
		cmd_line+=parameter;
		cmd_line+=L"\"";
	}
	if(!found) {
		throw cppcms_error("Parameter --winservice-mode=install is not provided via command line!");
	}

	BOOSTER_DEBUG("cppcms") << "Installing with parameters" << booster::nowide::convert(cmd_line);

	DWORD start_type_flag = 0;
	if(start_type == "auto")
		start_type_flag= SERVICE_AUTO_START;
	else if(start_type == "demand")
		start_type_flag= SERVICE_DEMAND_START;
	else
		throw cppcms_error("Parameter winservice.mode should be one of auto or demand");

		
	SC_HANDLE schm = OpenSCManagerW(0,0,SC_MANAGER_ALL_ACCESS);
	if(!schm) {
		booster::system::error_code e(GetLastError(),booster::system::windows_category);
		throw booster::system::system_error(e,"Failed to open sevice imager");
	}

	SC_HANDLE service_handle = CreateServiceW(
		schm,
		name.c_str(),
		display_name.c_str(),
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		start_type_flag,
		SERVICE_ERROR_NORMAL,
		cmd_line.c_str(),
		0, // load order group
		0, // tagid
		0, // dependencies
		cuser_name,
		cpassword
	);

	if(service_handle == NULL) {
		booster::system::error_code e(GetLastError(),booster::system::windows_category);
		CloseServiceHandle(schm);
		throw booster::system::system_error(e,"Failed to install service");
	}
	CloseServiceHandle(service_handle);
	CloseServiceHandle(schm);
	std::cout << "The service installed sucessefully" << std::endl;
}

namespace {
	SERVICE_STATUS_HANDLE status_handle;
	SERVICE_STATUS status;

	void WINAPI win_service_handler_proc(DWORD code)
	{
		if(code == SERVICE_CONTROL_SHUTDOWN  || code == SERVICE_CONTROL_STOP) {
			status.dwCurrentState = SERVICE_STOP_PENDING;
			status.dwControlsAccepted = 0;
			status.dwWaitHint = 10000;
			SetServiceStatus(status_handle,&status);
			winservice::instance().stop();
			return;
		}
		SetServiceStatus(status_handle,&status);

	}
	void WINAPI win_service_main(DWORD,wchar_t **)
	{
		status_handle = RegisterServiceCtrlHandlerW(L"",win_service_handler_proc);
		if(status_handle==0) {
			booster::system::error_code e(GetLastError(),booster::system::windows_category);
			BOOSTER_ERROR("cppcms") << "Failed to register windows service handle:" << e.message();
			return;
		}
		status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		try {
			status.dwWaitHint = 10000;
			status.dwCurrentState = SERVICE_START_PENDING;
			status.dwControlsAccepted = (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN);
			SetServiceStatus(status_handle,&status);
			
			winservice::instance().prepare();
			
			status.dwWaitHint = 0;
			status.dwCurrentState = SERVICE_RUNNING;
			SetServiceStatus(status_handle,&status);
		
			winservice::instance().exec();

		}
		catch(std::exception const &e ){
			BOOSTER_ERROR("cppcms") << "Main loop stopped:" << e.what() << booster::trace(e);
			status.dwWin32ExitCode = 1;
		}
		catch(...) {
			BOOSTER_ERROR("cppcms") << "Main loop stopped for unknown exception";
			status.dwWin32ExitCode = 1;
		}
		
		status.dwCurrentState = SERVICE_STOPPED;
		status.dwWaitHint = 0;
		SetServiceStatus(status_handle,&status);
		return;

	}
} // anonymous

void winservice::service()
{
	static wchar_t empty_wide_string = 0;
	SERVICE_TABLE_ENTRYW entry[2] = {
		{ &empty_wide_string ,win_service_main },
		{ 0,0 }
	};
	if(!StartServiceCtrlDispatcherW(entry)) {
		booster::system::error_code e(GetLastError(),booster::system::windows_category);
		throw booster::system::system_error(e,"Failed to start windows service");
	}
}

void winservice::console()
{
	prepare();
	exec();
}

void winservice::run(json::value &conf,int argc,char **argv)
{
	args_.assign(argv,argv+argc);
	settings_=conf;
	std::string mode = settings_.get("winservice.mode","console");
	if(mode == "console")
		console();
	else if(mode == "install")
		install();
	else if(mode == "uninstall")
		uninstall();
	else if(mode == "run") {
		conf.set("service.disable_global_exit_handling",true);
		service();
	}
	else
		throw cppcms_error("Invalid option winservice.mode=" + mode);
}

} // impl
} // cppcms
