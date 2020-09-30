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
# see below what it looks like
owj = requests.request("GET", url2).json()
# response is 200?

# the weatherbox sensors
wbj = requests.request("GET", "http://rainsensor.local").json()

# dark sky    will be kaput by 202007
dsj = requests.request("GET", "https://api.darksky.net/forecast/a9222c59972f1c7cdaba8691ab92499a/"+latitude+","+longitude+"?exclude=minutely,hourly,daily").json()

# Climacell 
#  https://www.climacell.co/   George@GASilvis.net / nxw6dturn6KShzT
ccj_API_KEY= d9FWl9c380rBqwiNXLLEbLIzxU021upJ
# see example below    https://api.climacell.co/v3/weather/realtime/?lat=41.717&lon=-70.633&apikey=d9FWl9c380rBqwiNXLLEbLIzxU021upJ&unit_system=us&fields=temp,dewpoint,humidity,wind_speed,wind_direction,wind_gust,cloud_cover,weather_code,baro_pressure
# also has sunrise,sunset      see    https://developer.climacell.co/v3/reference#data-layers-weather
ccj= requests.request("GET", "https://api.climacell.co/v3/weather/realtime/?lat"+latitude+"&lon="+longitude+"&unit_system=us&apikey="+ccj_API_KEY+"&fields=temp,dewpoint,humidity,wind_speed,wind_direction,wind_gust,cloud_cover,weather_code,baro_pressure")
ccd=[]
ccd[temp]= ccj['temp']['value'] # "temp":{"value":72.5,"units":"F"},
ccd['dewpoint']= ccj['dewpoint']['value'] # "dewpoint":{"value":69.13,"units":"F"},
ccd['wind_speed']= ccj['wind_speed']['value'] # "wind_speed":{"value":11.88,"units":"mph"},
ccd['wind_gust']= ccj['wind_gust']['value'] # "wind_gust":{"value":21.25,"units":"mph"},
ccd['baro_pressure']= ccj['baro_pressure']['value'] # "baro_pressure":{"value":29.8124,"units":"inHg"},
ccd['humidity']= ccj['humidity']['value'] # "humidity":{"value":89.06,"units":"%"},
ccd['wind_direction']= ccj['wind_direction']['value'] # "wind_direction":{"value":213.69,"units":"degrees"},
ccd['cloud_cover']= ccj['cloud_cover']['value'] # "cloud_cover":{"value":2.44,"units":"%"},
ccd['weather_code']= ccj['weather_code']['value'] # "weather_code":{"value":"clear"},
ccd['obs_time']= ccj['observation_time']['value'] # "observation_time":{"value":"2020-06-23T21:38:25.467Z"}}



#Create one file per day. Days are DATENITE, noon to noon

dt_now= datetime.now() # local time
local_hrs = dt_now.hour+ (dt_now.minute+ (dt_now.second/60))/60
# how old is the owj data
hrs_old= (dt_now.timestamp() - (owj['dt']))/3600
if local_hrs > 12:
    datenite= dt_now.date()
else:    
    datenite= dt_now.date() - timedelta(days=1)


s="{:f}".format(local_hrs)
s+=",{:f}".format(hrs_old) # owj data is not realtime
s+=",{}".format(owj['weather'][0]['id']) # 800 is clear
s+=",{},{}".format(owj['main']['temp'], owj['main']['humidity']) # F, %
s+=",{},{}".format(owj['wind']['speed'], owj['wind']['deg'] if 'deg' in owj['wind'] else 0 ) # mph, deg
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
    t="Name: {}".format(owj['name'])
    t+= ", Lat: {}, Lon:{}".format(owj['coord']['lat'], owj['coord']['lon'])
    t+= ", timezone: {}".format(owj['timezone']/3600) # hrs
    t+= ", dark sky nearest station: {}".format(dsj['flags']['source']['nearest-station'] if 'nearest-station' in dsj else -1)
    f.write(t+ '\n')
    sun = datetime.fromtimestamp(owj['sys']['sunrise'])
    t= "    Sunrise: {:f}".format(sun.hour+ (sun.minute+ (sun.second/60))/60)
    sun = datetime.fromtimestamp(owj['sys']['sunset'])
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
s+= "  {0:}".format(2 if wbj['relay']   else 0)      # rain based on wbox. It has a 15 min hysterisis
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

'''  open Weather
{"coord":{"lon":-70.63,"lat":41.72}
,"weather":[
     {"id":500    ,"main":"Rain"   ,"description":"light rain"  ,"icon":"10n"}
    ,{"id":701    ,"main":"Mist"   ,"description":"mist"        ,"icon":"50n"}]
    ,"base":"stations"
,"main":{"temp":44.69   ,"feels_like":27.95   ,"temp_min":42.01   ,"temp_max":46.99    ,"pressure":1008  ,"humidity":100}   ,"visibility":3219
,"wind":{"speed":27.51,"deg":80,"gust":39.15}
,"rain":{"1h":0.76}
,"clouds":{"all":90},"dt":1587945716
,"sys":{"type":1,"id":4119,"country":"US","sunrise":1587894304,"sunset":1587944106},"timezone":-14400,"id":4944405,"name":"Monument Beach"
    ,"cod":200}
'''



#Climacell example
'''
{"lat":41.717,
"lon":-70.633,
"temp":{"value":72.5,"units":"F"},
"dewpoint":{"value":69.13,"units":"F"},
"wind_speed":{"value":11.88,"units":"mph"},
"wind_gust":{"value":21.25,"units":"mph"},
"baro_pressure":{"value":29.8124,"units":"inHg"},
"humidity":{"value":89.06,"units":"%"},
"wind_direction":{"value":213.69,"units":"degrees"},
"cloud_cover":{"value":2.44,"units":"%"},
"weather_code":{"value":"clear"},
"observation_time":{"value":"2020-06-23T21:38:25.467Z"}}
'''