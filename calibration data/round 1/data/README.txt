Files:

airsense.csv: data collected by the DIY sensor
OxHiStNOx.csv: data collected by official monitor
data.xlsx: some preliminary analysis
results.txt: we can write some results there

- Started: 18 / 5 / 2017
0 ms at sensor
16:49 at mobile phone
15:53 at monitor time (+/- 2 minutes !)

- Stopped: 24 / 5 / 2017
516905067 ms at sensor
16:32 at mobile phone

samples are collected at the sensor each 60 to 61 seconds
at the monitor at 60 seconds sharp
the monitor skips some measurements every now and then

samples at the sensor are not instantaneous, particulates are sampled over 30
seconds, the other measurements are an average of 10 measurements with a 100 ms
time difference between each measurement

the Alphasense sensor is measured at two pins, one is used as reference, most
probably one should use the difference between the two pins

in the SPEC sensor the reference was not measured