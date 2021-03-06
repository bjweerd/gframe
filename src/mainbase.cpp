//
//
// @ Project : grame
// @ File Name : mainbase.cpp
// @ Date : 31-10-2012
// @ Author : Gijs Kwakkel
//
//
// Copyright (c) 2012 Gijs Kwakkel
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//

#include <gframe/mainbase.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <iostream>
#include <vector>
#include <grp.h>
#include <pwd.h>

std::string NAME = "add std::string NAME in your code";
std::string gNAME = "gframe";
std::string gVERSION = __GIT_VERSION;

void SegFaultAction(int i_num, siginfo_t * i_info, void * i_val)
{
    std::cout<< "SegFault Signal " << i_num << " caught..." << std::endl;

    const siginfo_t & v = * i_info;

    std::cout << v.si_signo << "= Signal number\n";
    std::cout << v.si_errno << "= An errno value\n";
    std::cout << v.si_code << "= Signal code\n";
    std::cout << v.si_pid << "= Sending process ID\n";
    std::cout << v.si_uid << "= Real user ID of sending process\n";
    std::cout << v.si_status << "= Exit value or signal\n";
#if defined(linux) || defined(__linux) || defined(__linux__)
    std::cout << v.si_utime << "= User time consumed\n";
    std::cout << v.si_stime << "= System time consumed\n";
    std::cout << v.si_int << "= POSIX.1b signal\n";
    std::cout << v.si_ptr << "= POSIX.1b signal\n";
#endif
    std::cout << v.si_addr << "= Memory location which caused fault\n";
    std::cout << v.si_band << "= Band event\n";
#if defined(linux) || defined(__linux) || defined(__linux__)
    std::cout << v.si_fd << "= File descriptor\n";
#endif

    throw * i_info;
    exit(0);
}

void TermAction(int i_num, siginfo_t * i_info, void * i_val)
{
    std::cout << "Term Signal " << i_num << " caught..." << std::endl;
    std::cout << "from pid:   " << i_info->si_pid << std::endl;
    std::cout << "si_errno:   " << i_info->si_errno << std::endl;
    std::cout << "si_code:    " << i_info->si_code << std::endl;
    std::cout << "si_uid:     " << i_info->si_uid << std::endl;
    std::cout << "si_status:  " << i_info->si_status << std::endl;
    exit(0);
}

void Usr1Action(int i_num, siginfo_t * i_info, void * i_val)
{
    std::cout<< "User Signal " << i_num << " caught..." << std::endl;
    std::cout << "from pid:   " << i_info->si_pid << std::endl;
    std::cout << "si_errno:   " << i_info->si_errno << std::endl;
    std::cout << "si_code:    " << i_info->si_code << std::endl;
    std::cout << "si_uid:     " << i_info->si_uid << std::endl;
    std::cout << "si_status:  " << i_info->si_status << std::endl;
    output::instance().closeLog();
    usleep(1000000);
    output::instance().openLog();
    std::string startBlock = "+++++++++++++++++++++++++++++++++++++++++++++++";
    output::instance().addOutput(startBlock, 2);
    output::instance().addOutput("+ SIGUSR1 (rotate log) on " + output::instance().sFormatTime("%d-%m-%Y %H:%M:%S") + " +", 2);
    output::instance().addOutput(startBlock, 2);
}

void mainbase::SetupSignal()
{
    struct sigaction new_action;
    struct sigaction old_action;
    /* Set up the structure to specify the new action. */
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = SA_SIGINFO
#if defined(linux) || defined(__linux) || defined(__linux__)
      | SA_NOMASK
#endif
      ;

// usr (rotate log) (signal 10)
    new_action.sa_sigaction = Usr1Action;
    sigaction (SIGUSR1, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
      sigaction (SIGUSR1, &new_action, NULL);

// SegFault
    new_action.sa_sigaction = SegFaultAction;
    sigaction (SIGSEGV, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
      sigaction (SIGSEGV, &new_action, NULL);


// termination
    new_action.sa_sigaction = TermAction;
    sigaction (SIGINT, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
      sigaction (SIGINT, &new_action, NULL);

    sigaction (SIGHUP, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
      sigaction (SIGHUP, &new_action, NULL);

    sigaction (SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
      sigaction (SIGTERM, &new_action, NULL);
}


mainbase::mainbase() :
m_INeedRoot(false),
m_DropRoot(false),
m_Foreground(false),
m_Name(NAME),
m_LogFileLocation("log/"),
m_PidFileLocation("/var/run/" + m_Name + "/"),
m_IniFile("conf/" + m_Name + ".ini"),
m_PidFile(m_PidFileLocation + m_Name + ".pid"),
m_LogFile(m_LogFileLocation + m_Name + ".log")
{
    SetupSignal();
    addVersion(gNAME + " " + gVERSION);
    addHelpItem("Runs the " + m_Name + " (default as " + m_Name + ", " + m_PidFile + ", " + m_LogFile + " " + m_IniFile + ")");
    addHelpItem("USAGE " + m_Name + " [OPTIONS]");
    addHelpItem("Available options:");
    addHelpItem("\t-h, --help List options");
    addHelpItem("\t-v, --version Output version and exit");
    addHelpItem("\t-f, --foreground Don't fork into the background");
    addHelpItem("\t-dr, --droproot [username] [group] run as");
    addHelpItem("\t-c, --config Set config file (default: " + m_IniFile + ")");
    addHelpItem("\t-d, --debug Set debug level [1-10] (default: 5)");
    addHelpItem("\t-p, --pid Set Pid file location (default: " + m_PidFileLocation + ")");
    addHelpItem("\t-l, --log Set log file location (default: " + m_LogFileLocation + ")");
    addHelpItem("\t-n, --name Set name for pid/log files (default: " + m_Name + ")");
    addHelpItem("\t--INeedRootPowerz Requered when running as root (not needed when droproot is specified)");
}

mainbase::~mainbase()
{
    remove(m_PidFile.c_str());
}

void mainbase::parseArgs(std::vector<std::string> args)
{
    for (uint nArg = 0; nArg < args.size(); nArg++)
    {
        if (args[nArg] == "--help" || args[nArg] == "-h")
        {
            showHelp();
            exit(EXIT_SUCCESS);
        }
        else if (args[nArg] == "--version" || args[nArg] == "-v")
        {
            showVersion();
            exit(EXIT_SUCCESS);
        }
        else if (args[nArg] == "--foreground" || args[nArg] == "-f")
        {
            m_Foreground = true;
        }
        else if (args[nArg] == "--droproot" || args[nArg] == "-dr")
        {
            if ((++nArg) < args.size())
            {
                if ((++nArg) < args.size())
                {
                    m_Uid = args[nArg-1];
                    m_Gid = args[nArg];
                    m_DropRoot = true;
                }
            }
        }
        else if (args[nArg] == "--config" || args[nArg] == "-c")
        {
            if ((++nArg) < args.size())
            {
                m_IniFile = args[nArg];
            }
        }
        else if (args[nArg] == "--debug" || args[nArg] == "-d")
        {
            if ((++nArg) < args.size())
            {
                int i;
                std::stringstream ss(args[nArg]);
                ss >> i;
                output::instance().setDebugLevel(i);
            }
        }
        else if (args[nArg] == "--pid" || args[nArg] == "-p")
        {
            if ((++nArg) < args.size())
            {
                m_PidFileLocation = args[nArg];
            }
        }
        else if (args[nArg] == "--log" || args[nArg] == "-l")
        {
            if ((++nArg) < args.size())
            {
                m_LogFileLocation = args[nArg];
            }
        }
        else if (args[nArg] == "--name" || args[nArg] == "-n")
        {
            if ((++nArg) < args.size())
            {
                m_Name = args[nArg];
            }
        }
        else if (args[nArg] == "--INeedRootPowerz")
        {
            m_INeedRoot = true;
        }
    }
}

int mainbase::run()
{
    m_PidFile = m_PidFileLocation + m_Name + ".pid";
    m_LogFile = m_LogFileLocation + m_Name + ".log";
    showVersion();
    if (readPidFile())
    {
        return 1;
    }
    createDirectory(m_LogFileLocation);
    createDirectory(m_PidFileLocation);
    if (isRoot() && !m_DropRoot)
    {
        fprintf(stdout, "Your are running %s as root!\n", m_Name.c_str());
        fprintf(stdout, "this is dangerouse and can cause great damage!\n");
        if (!m_INeedRoot) {
            return 1;
        }
        fprintf(stdout, "You have been warned.\n");
        fprintf(stdout, "Hit CTRL+C now if you don't want to run %s as root.\n", m_Name.c_str());
        fprintf(stdout, "%s will start in 15 seconds.\n", m_Name.c_str());
        sleep(15);
    }
    if (m_DropRoot)
    {
        output::instance().addOutput("dropping root", 2);
        if (!dropRoot())
        {
            return 1;
        }
    }
    int DaemonizeStatus = daemonize(!m_Foreground);
    if (DaemonizeStatus != -1)
    {
        return DaemonizeStatus;
    }
    output::instance().setLogFile(m_LogFile);
    output::instance().openLog();
    std::string startBlock = "+++++++++++++++++++++++++++++++++";
    for (unsigned int m_Name_Iterator = 0; m_Name_Iterator < m_Name.size(); m_Name_Iterator++)
    {
        startBlock = startBlock + "+";
    }
    output::instance().addOutput(startBlock, 2);
    output::instance().addOutput("+ Start " + m_Name + " on " + output::instance().sFormatTime("%d-%m-%Y %H:%M:%S") + " +", 2);
    output::instance().addOutput(startBlock, 2);
    if(!configreader::instance().readFile(m_IniFile))
    {
        return 1;
    }
    usleep(2000000);
    return -1;
}

bool mainbase::createDirectory(std::string directory)
{
    struct stat sb;
    if (stat(directory.c_str(), &sb) != 0)
    {
        umask(0);
        printf("creating %s\r\n", directory.c_str());
        mkdir(directory.c_str(), S_ISVTX | S_IRWXU | S_IRWXG | S_IRWXO);
        return true;
    }
    else
    {
        if (!S_ISDIR(sb.st_mode))
        {
            printf("%s exists but is not a directory!\r\n", directory.c_str());
            return false;
        }
    }
    return true;
}

bool mainbase::readPidFile()
{
    std::string sPid = "-2";
    int iFilePid = -2;

    // check if process in pidfile is still running
    std::ifstream ifFilePid (m_PidFile.c_str());
    if (ifFilePid.is_open())
    {
        while ( ifFilePid.good() )
        {
            getline (ifFilePid,sPid);
            //std::cout << sPid << std::endl;
        }
        ifFilePid.close();
    }
    std::stringstream ss(sPid);
    ss >> iFilePid;
    if (kill(iFilePid, 0) != -1)
    {
        printf("still running \r\n");
        return true;
        //exit(EXIT_FAILURE);
    }
    return false;
    // end check
}

void mainbase::writePidFile(int Pid)
{
    // write current pid to pidfile
    if (Pid < 0) {
        printf("pid < 0 FAIL \r\n");
        exit(EXIT_FAILURE);
    }

    std::ofstream ofPidFile (m_PidFile.c_str());
    printf("%s\n", m_PidFile.c_str());
    if (ofPidFile.is_open())
    {
        ofPidFile << Pid;
        ofPidFile.close();
    }
    else
    {
        printf("cant open pid file \r\n");
        exit(EXIT_FAILURE);
    }
    // end writing pid file
}

bool mainbase::isRoot()
{
# if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__)
    // User root? If one of these were root, we could switch the others to root, too
    if ((geteuid() == 0) || (getuid() == 0) || (getegid() == 0) || (getgid()== 0))
    {
        return true;
    }
    else
    {
        return false;
    }
    //return (geteuid() == 0 || getuid() == 0);
# else
    return false;
# endif
}

bool mainbase::dropRoot()
{
    struct passwd *pUser = getpwnam(m_Uid.c_str());
    struct group *pGroup = getgrnam(m_Gid.c_str());
    if (!pUser)
    {
        output::instance().addStatus(false, "User not found");
        return false;
    }
    else if (!pGroup)
    {
        output::instance().addStatus(false, "Group not found");
        return false;
    }
    else
    {
        int gid = pGroup->gr_gid;
        int uid = pUser->pw_uid;
        if (gid == 0 || uid == 0)
        {
            output::instance().addStatus(false, "Cannot run as root");
            return false;
        }
        else
        {
            if ((geteuid() == 0) || (getuid() == 0) || (getegid() == 0) || (getgid()== 0))
            {
                int u, eu, g, eg, sg;
                sg = setgroups(0, NULL);
                if (sg < 0)
                {
                    output::instance().addStatus(false, "Could not remove supplementary groups!");
                    return false;
                }
                g = setgid(pGroup->gr_gid);
                eg = setegid(pGroup->gr_gid);
                if ((g < 0) || (eg < 0))
                {
                    output::instance().addStatus(false, "Could not switch group id!");
                    return false;
                }
                u = setuid(pUser->pw_uid);
                eu = seteuid(pUser->pw_uid);
                if ((u < 0) || (eu < 0))
                {
                    output::instance().addStatus(false, "Could not switch user id!");
                    return false;
                }
            }
            return true;
        }
        return true;
    }
    return true;
}

void mainbase::showVersion()
{
    std::cout << "Copyright (c) 2012 Gijs Kwakkel" << std::endl;
    std::cout << "GNU Version 2" << std::endl;
    for (unsigned int Version_iterator = 0; Version_iterator < m_VersionItems.size(); Version_iterator++)
    {
        std::cout << m_VersionItems[Version_iterator] << std::endl;
    }
}

void mainbase::addVersion(std::string VersionItem)
{
    m_VersionItems.push_back(VersionItem);
}

void mainbase::showHelp()
{
    for (unsigned int HelpItems_iterator = 0; HelpItems_iterator < m_HelpItems.size(); HelpItems_iterator++)
    {
        std::cout << m_HelpItems[HelpItems_iterator] << std::endl;
    }
}

void mainbase::addHelpItem(std::string HelpItem)
{
    m_HelpItems.push_back(HelpItem);
}

int mainbase::daemonize(bool Daemonize)
{
    if (!Daemonize)
    {
        int Pid = getpid();
        writePidFile(Pid);
        fprintf(stdout, "Staying open for debugging\n");
        fprintf(stdout, "pid [%d]\n", Pid);
    }
    else
    {
        fprintf(stdout, "Forking into the background\n");

        int Pid = fork();

        if (Pid == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            return 1;
        }

        if (Pid > 0) {
            // We are the parent. We are done and will go to bed.
            writePidFile(Pid);
            fprintf(stdout, "pid [%d]\n", Pid);
            return 0;
        }
        // Redirect std in/out/err to /dev/null
        close(0); open("/dev/null", O_RDONLY);
        close(1); open("/dev/null", O_WRONLY);
        close(2); open("/dev/null", O_WRONLY);

        // We are the child. There is no way we can be a process group
        // leader, thus setsid() must succeed.
        setsid();
        // Now we are in our own process group and session (no
        // controlling terminal). We are independent!
        fprintf(stdout, "child running\n");
    }
    return -1;
}
