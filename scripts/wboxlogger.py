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
latitude= "41.717"
longitude= "-70.633"
# OpenWeather 
url2="http://api.openweathermap.org/data/2.5/weather?lat="+latitude+"&lon="+longitude+"&units=imperial&appid=a93251af2c66650796d56b4246b9f1d1"
rj = requests.request("GET", url2).json()
# response is 200?
wbj = requests.request("GET", "http://rainsensor.local").json()
# dark sky
dsj = requests.request("GET", "https://api.darksky.net/forecast/a9222c59972f1c7cdaba8691ab92499a/"+latitude+","+longitude+"?exclude=minutely,hourly,daily").json()


#Create one file per day. Days are DAYNITE, noon to noon

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

# TBD compute safety

# Report data via MQTT?

# create one line file in Boltwood format and make accessible to ACP
s= dt_now.strftime("%Y-%m-%d %H:%M:%S")+".{0:0>6}".format(dt_now.microsecond)[:3]
s+= " F M {0:6.1f} {1:6.1f}".format(wbj['object'], dsj['currently']['temperature']) # SkyT, AmbT 
s+= " {0:6.1f}".format(wbj['ambient']) # SenT
s+= " {0:6.1f}".format(dsj['currently']['windGust']) # Wind
s+= " {0:3.0f}".format(100*dsj['currently']['humidity']) # Hum
s+= " {0:6.1f}  50".format(dsj['currently']['dewPoint']) # DewPt, Hea
s+= ""
f= open("bolt.txt", 'w')
f.write(s)
f.close() 

'''
This recommended format gives access to all of the data Cloud Sensor II can provide.  
The data is similar to the display fields in the Clarity II window.  The format has been split across 
two lines to make it fit on this page: 
Date       Time        T V   SkyT   AmbT   SenT   Wind Hum  DewPt Hea  R W Since  Now() Day's c w r d C A 
2005-06-03 02:07:23.34 C K  -28.5   18.7   22.5   45.3  75   10.3   3  0 0 00004 038506.08846 1 2 1 0 0 0 
2019-10-31 23:21:56.7  F M   60.4   69.3   73.6   51.0  82   63.5  50


The header line is here just for illustration.  It does not actually appear anywhere. The fields mean:     
Heading Col’s    Meaning     
Date          1-10 local date yyyy-mm-dd     
Time         12-22 local time hh:mm:ss.ss (24 hour clock)     
T               24 temperature units displayed and in this data, 'C' for Celsius or 'F' for Fahrenheit     
V               26 wind velocity units displayed and in this data, ‘K’ for km/hr or ‘M’ for mph or 'm' for m/s     
SkyT         28-33 sky-ambient temperature, 999. for saturated hot, -999. for saturated cold, or –998. for wet     
AmbT         35-40 ambient temperature     
SenT         41-47 sensor case temperature, 999. for saturated hot, -999. for saturated cold.  Neither saturated condition should ever occur.     
Wind         49-54 wind speed or: -1. if still heating up,  -2. if wet,  -3. if the A/D from the wind probe is bad (firmware <V56 only) ,  -4. if the probe is not heating (a failure condition),  -5. if the A/D from the wind probe is low (shorted, a failure condition) (firmware >=V56 only),  -6. if the A/D from the wind probe is high (no probe plugged in or a failure) (firmware >=V56 only).     
Hum          56-58 relative humidity in %     
DewPt        60-65 dew point temperature     
Hea          67-69 heater setting in %     
R               71 rain flag, =0 for dry, =1 for rain in the last minute, =2 for rain right now     
W               73 wet flag, =0 for dry, =1 for wet in the last minute, =2 for wet right now     
Since        75-79 seconds since the last valid data     
Now() Day's  81-92 date/time given as the VB6 Now() function result (in days) when Clarity II last wrote this file     
c               94 cloud condition (see the Cloudcond enum in section 20) 
w               96 wind condition (see the Windcond enum in section 20)     
r               98 rain condition (see the Raincond enum in section 20)     
d              100 daylight condition (see the Daycond enum in section 20)     
C              102 roof close, =0 not requested, =1 if roof close was requested on this cycle     
A              104 alert, =0 when not alerting, =1 when alerting

Public Enum CloudCond    
    cloudUnknown = 0    
    'Those below are based upon thresholds set in the setup window.    
    cloudClear = 1 
    cloudCloudy = 2    
    cloudVeryCloudy = 3 
End Enum 
Public Enum WindCond    
    windUnknown = 0    
    'Those below are based upon thresholds set in the setup window.    
    windCalm = 1    
    windWindy = 2    
    windVeryWindy = 3 
End Enum Public 
Enum RainCond    
    rainUnknown = 0    
    'Those below are based upon thresholds set in the setup window.    
    rainDry = 1    
    rainWet = 2  'sensor has water on it    
    rainRain = 3 'falling rain drops detected 
End Enum Public 
Enum DayCond    
    dayUnknown = 0    
    'Below are based upon thresholds set in the setup window.    
    dayDark = 1    
    dayLight = 2    
    dayVeryLight = 3
    
End Enum 

'''



