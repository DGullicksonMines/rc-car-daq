#!/usr/bin/env python

import subprocess

def getData():
    
    return


subprocess.Popen(f"ssh rcCar@rcCar ./daq",  shell = True, stdout = subprocess.PIPE, stderr=subprocess.PIPE).communicate()
