///////////////////////////////////////////////////////////////////////////////
//                                                                             
//  Copyright (C) 2008-2010  Artyom Beilis (Tonkikh) <artyomtnk@yahoo.com>     
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
#include "daemonize.h"

#ifdef CPPCMS_WIN32
namespace cppcms {
namespace impl {

	void daemonize(json::value const &/*conf*/) {}
	void de_daemonize(json::value const &/*conf*/) {}
}
}
#else // POSIX OS

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <errno.h>

#include <stdlib.h>

#include <sstream>
#include <vector>
#include <locale>

#include <cppcms/json.h>
#include <cppcms/cppcms_error.h>

namespace cppcms {
namespace impl {

	int daemonizer::global_urandom_fd = -1;

	daemonizer::daemonizer(json::value const &conf)
	{
		real_pid = getpid();
		try {
			daemonize(conf);
		}
		catch(...) {
			cleanup();
			throw;
		}
	}
	daemonizer::~daemonizer()
	{
		cleanup();
	}
	void daemonizer::cleanup()
	{
		if(!unlink_file.empty() && real_pid == getpid())
			unlink(unlink_file.c_str());
	}
	void daemonizer::daemonize(json::value const &conf)
	{
		if(getppid() == 1)
			return;
		
		if(!conf.get("daemon.enable",false))
			return;
		
		int gid = -1;
		int uid = -1;
		
		int devnull_fd = -1;
		int lock_fd = -1;

		try {
		
			devnull_fd = open("/dev/null",O_RDWR);
			
			if(devnull_fd < 0) {
				int err = errno;
				throw cppcms_error(err,"Failed to open /dev/null");
			}
			
			std::string group = conf.get("daemon.group","");
			if(!group.empty()) {
				struct group *gr = getgrnam(group.c_str());
				if(!gr)
					throw cppcms_error("Invalid group " + group);
				gid = gr->gr_gid;
			}
			
			std::string user = conf.get("daemon.user","");
			if(!user.empty()) {
				struct passwd *pw = getpwnam(user.c_str());
				if(!pw)
					throw cppcms_error("Invalid user " + user);
				uid = pw->pw_uid;
				if(gid==-1)
					gid = pw->pw_gid;
			}
			
			std::string chroot_dir = conf.get("daemon.chroot","");

			if(!chroot_dir.empty()) {
				global_urandom_fd = open("/dev/urandom",O_RDONLY);
				if(global_urandom_fd < 0) {
					int err = errno;
					throw cppcms_error(err,"Failed to open /dev/urandom");
				}

				if(chdir(chroot_dir.c_str()) < 0) {
					int err = errno;
					throw cppcms_error(err,"Failed to chdir to " + chroot_dir);
				}
				if(chroot(chroot_dir.c_str()) < 0) {
					int err = errno;
					throw cppcms_error(err,"Failed to chroot to " + chroot_dir);
				}

			}

			int fd_limit = conf.get("daemon.fdlimit",-1);
			if(fd_limit != -1) {
				struct rlimit lm;
				lm.rlim_cur = fd_limit;
				lm.rlim_max = fd_limit;
				if(setrlimit(RLIMIT_NOFILE,&lm) < 0) {
					int err = errno;
					throw cppcms_error(err,"Failed to change daemon.fdlimit");
				}
			}
			
			if(gid!=-1) {
				if(setgid(gid) < 0) {
					int err = errno;
					throw cppcms_error(err,"Failed setgid to group " + group);
				}
			}

			if(uid!=-1) {
				if(setuid(uid) < 0) {
					int err = errno;
					throw cppcms_error(err,"Failed setuid to user " + user);
				}
			}


			std::string lock_file = conf.get("daemon.lock","");
			if(!lock_file.empty()) {
				if(lock_file[0]!='/') {
					char *wd = 0;
					std::vector<char> buf(256);
					while((wd=getcwd(&buf.front(),buf.size()))==0 && errno==ERANGE)
						buf.resize(buf.size()*2);
					if(!wd) {
						int err=errno;
						throw cppcms_error(err,"getcwd failed");
					}
					lock_file=wd + ("/" + lock_file);
				}
				lock_fd = ::open(lock_file.c_str(),O_RDWR | O_CREAT | O_EXCL,0666);
				if(lock_fd < 0) {
					int err = errno;
					throw cppcms_error(err,"Failed to create a lock file");
				}
				unlink_file = lock_file;
			}

			// forking twice
			int chid = fork();
			if(chid < 0) {
				int err = errno;
				throw cppcms_error(err,"Fork failed");
			}
			if(chid != 0)
				exit(0);
			
			real_pid = getpid();
			
			if(setsid() < 0) {
				int err = errno;
				throw cppcms_error(err,"setsid failed");

			}
			chid = fork();
			if(chid < 0) {
				int err = errno;
				throw cppcms_error(err,"2nd fork failed");
			}
			if(chid != 0)
				exit(0);
			
			real_pid = getpid();

			dup2(devnull_fd,0);
			dup2(devnull_fd,1);
			dup2(devnull_fd,2);

			close(devnull_fd);
			devnull_fd = -1;

			int chdir_r = chdir("/");
			(void)(chdir_r); // shut up GCC

			if(lock_fd!=-1) {
				std::ostringstream ss;
				ss.imbue(std::locale::classic());
				ss << real_pid;
				std::string pid = ss.str();
				int v = write(lock_fd,pid.c_str(),pid.size());
				if(v!=int(pid.size())) {
					throw cppcms_error("Failed to write to lock file");
				}
				close(lock_fd);
				lock_fd = -1;
			}
		}
		catch(...) {
			if(lock_fd >= 0)
				close(lock_fd);
			if(devnull_fd >=0)
				close(devnull_fd);
			if(global_urandom_fd >= 0)
				close(global_urandom_fd);
			throw;
		}
	}

} // impl
} // cppcms
#endif // POSIX OS
