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
# dark sky
dsj = requests.request("GET", "https://api.darksky.net/forecast/a9222c59972f1c7cdaba8691ab92499a/41.717,-70.633?exclude=minutely,hourly,daily").json()

#one file per day. Days are DAYNITE, noon to noon


dt_now= datetime.now() # local time
local_hrs = dt_now.hour+ (dt_now.minute+ (dt_now.second/60))/60
# how old is the rj data
hrs_old= (dt_now.timestamp() - (rj['dt']))/3600
if local_hrs > 12:
    datenite= dt_now.date()
else:    
    datenite= dt_now.date() - timedelta(days=1)


s="{:f}".format(local_hrs)
s+=",{:f}".format(hrs_old) # rj data is not realtime
s+=",{}".format(rj['weather'][0]['id']) # 800 is clear
s+=",{},{}".format(rj['main']['temp'], rj['main']['humidity']) # F, %
s+=",{},{}".format(rj['wind']['speed'], rj['wind']['deg'] if 'deg' in rj['wind'] else 0 ) # mph, deg
s+=",{},{},{}".format(wbj['object'], wbj['ambient'], wbj['relay']) # skytemp, air, relay
# add dark sky data
s+=",{:f}".format((dt_now.timestamp() - (dsj['currently']['time']))/3600) # age 
s+=",{:.0f}".format(100*dsj['currently']['cloudCover'])
s+=",{}".format(dsj['currently']['temperature'])
s+=",{:.0f}".format(100*dsj['currently']['humidity'])
s+=",{}".format(dsj['currently']['dewPoint'])
s+=",{}".format(dsj['currently']['windSpeed'])
s+=",{}".format(dsj['currently']['windGust'])
s+=",{}".format(dsj['currently']['windBearing'])
s+=",{}".format(dsj['currently']['summary'])



fname=  Path("C:/Users/aavsonet/Google Drive/WBoxLog/{}.csv".format(datenite))


if os.path.exists(fname):
    f= open(fname, 'a') # append if already exists
else:
    f= open(fname, 'w') # make a new file if not
    # and add header info
    t="Name: {}".format(rj['name'])
    t+= ", Lat: {}, Lon:{}".format(rj['coord']['lat'], rj['coord']['lon'])
    t+= ", timezone: {}".format(rj['timezone']/3600) # hrs
    t+= ", dark sky nearest station: {}".format(dsj['flags']['source']['nearest-station'] if 'nearest-station' in dsj else -1)
    f.write(t+ '\n')
    sun = datetime.fromtimestamp(rj['sys']['sunrise'])
    t= "    Sunrise: {:f}".format(sun.hour+ (sun.minute+ (sun.second/60))/60)
    sun = datetime.fromtimestamp(rj['sys']['sunset'])
    t+= ", Sunset: {:f}".format(sun.hour+ (sun.minute+ (sun.second/60))/60)
    f.write(t+ '\n')
    t= "hour, age, code, temp(F), humidity(%), wind(mph), dir(deg), skytemp, ambient, relay"
    t+= ",DS age,clouds,temp,humidity,dewPoint,windSpeed,windGust,windDir,summary"
    f.write(t+ '\n')

f.write(s+ '\n')    
f.close()

# compute safety

# create one line file in Boltwood format

