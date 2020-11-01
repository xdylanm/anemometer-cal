#!/usr/bin/python3 

import sys
import time
import getopt

from rpm_monitor import RpmMonitor

def print_help(cmd):
    print('{} -f <outputfile> -a <rpmport> -g <gpsport>'.format(cmd))

def parse_opts(argv):
    logfilename = None
    rpmport = None
    gpsport = None

    try:
        opts, args = getopt.getopt(argv[1:],"hf:a:g:",["rpm-port=","gps-port="])
    except getopt.GetoptError:
        print_help(argv[0])
        sys.exit(2)

    for opt, arg in opts:
        if opt == '-h':
            print_help(argv[0])
            sys.exit()
        elif opt == '-f':
            logfilename = arg
        elif opt in ('-a', '--rpm-port'):
            rpmport = arg
        elif opt in ('-g', '--gps-port'):
            gpsport = arg

    return (logfilename, rpmport, gpsport)


if __name__ == "__main__":

    logfile_prefix, rpm_port, gps_port = parse_opts(sys.argv)

    print("Opening RPM sensor on port {}".format(rpm_port))
    rpm_mon = RpmMonitor(rpm_port)
    if not rpm_mon.ready():
        print("Failed to start serial comms on {}".format(rpm_mon.port()))
    else:
        logfilename = logfile_prefix + time.strftime("_%Y%m%d_%H%M%S.log")
        print("Logging averaged angular velocity (deg/s) to {}".format(logfilename))
        rpm_mon.start()
        time.sleep(2)
        with open(logfilename,"w") as hflog:
            try:
                for j in range(10):
                    vth, rpms = rpm_mon.next()
                    hflog.write("{}, {:7.1f}, {:7.1f}\n".format(time.strftime("%H:%M:%S"), vth, rpms))
            except (KeyboardInterrupt, SystemExit):
                pass
        
        rpm_mon.stop()
        print("Done.")
