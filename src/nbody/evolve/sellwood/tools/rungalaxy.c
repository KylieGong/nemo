/*
 *  preprocessor to run (a series of) GALAXY:
 *
 *  The first argument must be a (non-existent) directory name, in
 *  which a new 'galaxy.dat' file will be written, and in which
 *  galaxy will be run. The 'galaxy' program is assumed to be in the
 *  PATH
 *
 *	24-jun-1997		written
 *	18-jul-2001		default is galaxy.exe
 *      18-mar-2004             finally made it work
 *       7-mar-2006             no need for snap2ini
 *
 */

#include <stdinc.h>
#include <getparam.h>
#include <vectmath.h>
#include <filestruct.h>
#include <history.h>

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <snapshot/snapshot.h>
#include <snapshot/body.h>
#include <snapshot/get_snap.c>

string defv[] = {
    "in=???\n         input snapshot (nemo format)",
    "outdir=???\n     output directory in which GALAXY will be run",
    "scale=15.0\n     number of grid cells per unit length",
    "dt=0.05\n        integration time step",
    "dtout=0.5\n      time between particle outputs",
    "dtlog=0.1\n      time between integral checks",
    "tstop=1.0\n      end time",
    "format=%g\n      format for pos,vel for galaxy.ini",
    "grid=33,33,33\n  number of grid cells in (x,y,z,)",
    "exe=galaxy.exe\n name of GALAXY executable",
    "VERSION=2.0\n    7-mar-06 PJT",
    NULL,
};

string usage = "frontend to run Sellwood's galaxy code";

string cvsid="$Id$";

int nemo_main()
{
    int i, n, nbody, bits,ngrid[3];
    real scale, dt, dtout, dtlog, tstop, tsnap, mass;
    string exefile = getparam("exe");
    string rundir = getparam("outdir");
    string fmt = getparam("format");
    stream datstr, instr;
    char fullname[256];
    char command[256];
    char fmt6[256];
    Body *btab,*bp;

    n = nemoinpi(getparam("grid"),ngrid,3);
    if (n>0 && n<=3) {
        for (i=n; i<3; i++)
            ngrid[i] = ngrid[i-1];
    } else
        error("%d Syntax error: %s (need 1,2 or 3 integers)",
                n,getparam("grid"));
    scale = getdparam("scale");
    dt = getdparam("dt");
    dtout = getdparam("dtout");
    dtlog = getdparam("dtlog");
    tstop = getdparam("tstop");
    sprintf(fmt6,"%s %s %s %s %s %s\n",fmt,fmt,fmt,fmt,fmt,fmt);

    make_rundir(rundir);


    sprintf(fullname,"%s/%s",rundir,"galaxy.dat");
    datstr = stropen(fullname,"w");    
    fprintf(datstr,"%d %d %d\n",ngrid[0],ngrid[1],ngrid[2]);
    fprintf(datstr,"%g\n",scale);
    fprintf(datstr,"%g\n",dt);
    fprintf(datstr,"%g\n",dtout);
    fprintf(datstr,"%g\n",dtlog);
    fprintf(datstr,"%g\n",tstop);
    strclose(datstr);

    instr = stropen(getparam("in"),"r");
    get_history(instr);
    if (!get_tag_ok(instr, SnapShotTag)) error("no snapshot");
    get_snap(instr, &btab, &nbody, &tsnap, &bits);
    if ( (bits & PhaseSpaceBit) == 0) error("no phasespace");
    for (bp=btab;bp<btab+nbody; bp++)
      mass += Mass(bp);
    
    sprintf(fullname,"%s/%s",rundir,"galaxy.ini");
    datstr = stropen(fullname,"w");
    fprintf(datstr,"%g %g %d\n",tsnap,mass,nbody);
    for (bp=btab;bp<btab+nbody; bp++)
      fprintf(datstr,fmt6,
	      Pos(bp)[0],Pos(bp)[1],Pos(bp)[2],
	      Vel(bp)[0],Vel(bp)[1],Vel(bp)[2]);
    strclose(datstr);

    goto_rundir(rundir);
    run_program(exefile);
}


goto_rundir(string name)
{
    if (chdir(name))
        error("Cannot change directory to %s",name);
}

make_rundir(string name)
{
    if (mkdir(name, 0755))
        error("Run directory %s already exists",name);
}


run_program(string exe)
{
#if 1
    system(exe);
#else
    if (execlp(exe,NULL)) {
        fprintf(stderr,"Problem executing %s\n",exe);
        exit(1);
    }
#endif
}
