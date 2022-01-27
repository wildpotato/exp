
#include "NuCommon.h"
#include <signal.h>

void NuDaemonDenyAllSignal();

static void Default_CB(int Sig, void *Argu)
{
	return;
}

typedef void (*NuDaemonOnSignal_Fn)(int Sig, void *Argu);

static struct
{
	volatile sig_atomic_t	Working;
	void					*Argu;
	NuDaemonOnSignal_Fn		CB_OnSignal;
} g_Daemon = {1, NULL, &Default_CB};

static void SignalHdlr(int Sig)
{
	(g_Daemon.CB_OnSignal)(Sig, g_Daemon.Argu);
	return;
}

void NuDaemonStart()
{
	pid_t		Pid = 0;

	if((Pid = fork()) < 0)
    {
		exit(-1);
    }
	else if(Pid != 0)
    {
		exit(0);
    }

	setsid();

	if((Pid = fork()) < 0)
    {
		exit(-1);
    }
	else if(Pid != 0)
    {
		exit(0);
    }

	umask(027);

	NuDaemonDenyAllSignal();

	return;
}

void NuDaemonSetOnSignalCB(NuDaemonOnSignal_Fn CB_OnSignal, void *Argu)
{
	g_Daemon.CB_OnSignal = CB_OnSignal;
	g_Daemon.Argu = Argu;

	return;
}

void NuDaemonAllowSignal(int Sig)
{
	sigset_t			SigMaskSet, SigActSet;
	struct sigaction	SigAction;

	sigemptyset(&SigActSet);
	sigemptyset(&SigMaskSet);
	sigaddset(&SigActSet, Sig);

	sigprocmask(SIG_BLOCK, NULL, &SigMaskSet);
	if(sigismember(&SigMaskSet, Sig))
    {
		sigprocmask(SIG_UNBLOCK, &SigActSet, NULL);
    }

	pthread_sigmask(SIG_BLOCK, NULL, &SigMaskSet);
	if(sigismember(&SigMaskSet, Sig))
    {
		pthread_sigmask(SIG_UNBLOCK, &SigActSet, NULL);
    }

	SigAction.sa_handler = &SignalHdlr;
	sigfillset(&(SigAction.sa_mask));
	SigAction.sa_flags = 0;

	sigaction(Sig, &SigAction, NULL);

	return;
}

void NuDaemonAllowAllSignal()
{
	sigset_t	SigMaskSet;

	sigemptyset(&SigMaskSet);
	sigprocmask(SIG_SETMASK, &SigMaskSet, NULL);
	pthread_sigmask(SIG_SETMASK, &SigMaskSet, NULL);

	return;
}

void NuDaemonDenySignal(int Sig)
{
	sigset_t			SigMaskSet, SigActSet;
	struct sigaction	SigAction;

	sigemptyset(&SigActSet);
	sigemptyset(&SigMaskSet);
	sigaddset(&SigActSet, Sig);

	sigprocmask(SIG_BLOCK, NULL, &SigMaskSet);
	if(!sigismember(&SigMaskSet, Sig))
    {
		sigprocmask(SIG_BLOCK, &SigActSet, NULL);
    }

	pthread_sigmask(SIG_BLOCK, NULL, &SigMaskSet);
	if(!sigismember(&SigMaskSet, Sig))
    {
		pthread_sigmask(SIG_BLOCK, &SigActSet, NULL);
    }

	SigAction.sa_handler = SIG_IGN;
	sigfillset(&(SigAction.sa_mask));
	SigAction.sa_flags = 0;

	sigaction(Sig, &SigAction, NULL);

	return;
}

void NuDaemonDenyAllSignal()
{
	sigset_t	SigMaskSet;

	sigfillset(&SigMaskSet);

	sigprocmask(SIG_BLOCK, &SigMaskSet, NULL);
	pthread_sigmask(SIG_BLOCK, &SigMaskSet, NULL);

	return;
}

void NuDaemonStop()
{
	g_Daemon.Working = 0;

	return;
}

int NuDaemonKeepGoing()
{
	return g_Daemon.Working;
}

int *NuDaemonVaryKeepGoing()
{
	return (int *)&(g_Daemon.Working);
}

