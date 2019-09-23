# -*- coding: utf-8 -*-
"""
Weather Box logger
"""
import requests
import json
from datetime import datetime, timedelta
import os
from pathlib import Path

# location is SGEO's home on Mashnee 
url2="http://api.openweathermap.org/data/2.5/weather?lat=41.717&lon=-70.633&units=imperial&appid=a93251af2c66650796d56b4246b9f1d1"
rj = requests.request("GET", url2).json()
# response is 200?
wbj = requests.request("GET", "http://rainsensor.local").json()

#one file per day. Days are DAYNITE, noon to noon


dt_now= datetime.now() # local time
local_hrs = dt_now.hour+ (dt_now.minute+ (dt_now.second/60))/60
# how old is the rj data
hrs_old= (dt_now.timestamp() - (rj['dt']))/3600
if local_hrs > 12:
    datenite= dt_now.date()
else:    
    datenite= dt_now.date() - timedelta(days=1)


s="{:f},".format(local_hrs)
s+="{:f},".format(hrs_old) # rj data is not realtime
s+="{},".format(rj['weather'][0]['id']) # 800 is clear
s+="{},{},".format(rj['main']['temp'], rj['main']['humidity']) # F, %
s+="{},{},".format(rj['wind']['speed'], rj['wind']['deg']) # mph, deg
s+="{},{},{}".format(wbj['object'], wbj['ambient'], wbj['relay']) # skytemp, air, relay

fname=  Path("C:/Users/aavsonet/Google Drive/WBoxLog/{}.csv".format(datenite))


if os.path.exists(fname):
    f= open(fname, 'a') # append if already exists
else:
    f= open(fname, 'w') # make a new file if not
    # and add header info
    t="Name: {}".format(rj['name'])
    t+= ", Lat: {}, Lon:{}".format(rj['coord']['lat'], rj['coord']['lon'])
    t+= ", timezone: {}".format(rj['timezone']/3600) # hrs
    f.write(t+ '\n')
    sun = datetime.fromtimestamp(rj['sys']['sunrise'])
    t= "    Sunrise: {:f}".format(sun.hour+ (sun.minute+ (sun.second/60))/60)
    sun = datetime.fromtimestamp(rj['sys']['sunset'])
    t+= ", Sunset: {:f}".format(sun.hour+ (sun.minute+ (sun.second/60))/60)
    f.write(t+ '\n')
    t= "hour, age, code, temp(F), humidity(%), wind(mph), dir(deg), skytemp, ambient, relay"
    f.write(t+ '\n')

f.write(s+ '\n')    
f.close()

# compute safety

# create one line file in Boltwood format

