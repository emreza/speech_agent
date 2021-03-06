form Counting Syllables in Sound Utterances
   sentence filename C:\Users\Reza\Documents\recorded_samples\wavsam1.wav
   boolean male no
endform

 
# shorten variables
silencedb = -25
mindip = 2
minpause = 0.3
 writeInfoLine: " tmin tmax fmean fmin fmax fstdev frange"
printline  soundname, nsyll, npause, dur (s), phonationtime (s), speechrate (nsyll/dur), articulation rate (nsyll / phonationtime), ASD (speakingtime/nsyll) 

# read files
Read from file... 'filename$'

# use object ID
   end = Get end time
   #pauseScript: end  
   Extract part... 0.5 'end'-0.5 "rectangular" 1 no
   soundname$ = selected$("Sound")
   soundid = selected("Sound")
   
   originaldur = Get total duration
   time_step = originaldur 
   numberOfTimeSteps = originaldur / time_step
   # allow non-zero starting time

   bt = Get starting time

   # Use intensity to get threshold
   To Intensity... 50 0 yes
   intid = selected("Intensity")
   start = Get time from frame number... 1
   nframes = Get number of frames
   end = Get time from frame number... 'nframes'

   # estimate noise floor
   minint = Get minimum... 0 0 Parabolic
   # estimate noise max
   maxint = Get maximum... 0 0 Parabolic
   #get .99 quantile to get maximum (without influence of non-speech sound bursts)
   max99int = Get quantile... 0 0 0.99

   # estimate Intensity threshold
   #silencedb = (minint - max99int)*0.9
   threshold = 52
   threshold2 = maxint - max99int
   threshold3 = silencedb - threshold2
   threshold4 = 52 - maxint
   if threshold < minint
       threshold = minint
   endif

  # get pauses (silences) and speakingtime
   To TextGrid (silences)... threshold4 minpause 0.3 silent sounding
   textgridid = selected("TextGrid")
   silencetierid = Extract tier... 1
   silencetableid = Down to TableOfReal... sounding
   nsounding = Get number of rows
   npauses = 'nsounding'


   select 'intid'
   Down to Matrix
   matid = selected("Matrix")
   # Convert intensity to sound
   To Sound (slice)... 1
   sndintid = selected("Sound")

   # use total duration, not end time, to find out duration of intdur
   # in order to allow nonzero starting times.
   intdur = Get total duration
   intmax = Get maximum... 0 0 Parabolic

   # estimate peak positions (all peaks)
   To PointProcess (extrema)... Left yes no Sinc70
   ppid = selected("PointProcess")

   numpeaks = Get number of points

   # fill array with time points
   for i from 1 to numpeaks
       t'i' = Get time from index... 'i'
   endfor 


   # fill array with intensity values
   select 'sndintid'
   peakcount = 0
   for i from 1 to numpeaks
       value = Get value at time... t'i' Cubic
       if value > threshold
             peakcount += 1
             int'peakcount' = value
             timepeaks'peakcount' = t'i'
       endif
   endfor


   # fill array with valid peaks: only intensity values if preceding 
   # dip in intensity is greater than mindip
   select 'intid'
   validpeakcount = 0
   currenttime = timepeaks1
   currentint = int1

   for p to peakcount-1
      following = p + 1
      followingtime = timepeaks'following'
      dip = Get minimum... 'currenttime' 'followingtime' None
      diffint = abs(currentint - dip)

      if diffint > mindip
         validpeakcount += 1
         validtime'validpeakcount' = timepeaks'p'
      endif
         currenttime = timepeaks'following'
         currentint = Get value at time... timepeaks'following' Cubic
   endfor


   # Look for only voiced parts
   select 'soundid' 
   if male
     To Pitch (ac)... 0.02 40 4 no 0.03 0.25 0.01 0.35 0.25 200
   else
     To Pitch (ac)... 0.02 100 4 no 0.03 0.25 0.01 0.35 0.25 300
   endif
   # keep track of id of Pitch
   pitchid = selected("Pitch")
startloop =1
startploop =1
tid = Create Table with column names... table numberOfTimeSteps times vals
totalspeaking = 0
totalvowelcount = 0
for step to numberOfTimeSteps

       
    tmin = bt + (step - 1) * time_step
    tmax = tmin + time_step


 select 'pitchid'
    mean = Get mean: tmin, tmax, "Hertz"
    minimum = Get minimum: tmin, tmax, "Hertz", "Parabolic"
    maximum = Get maximum: tmin, tmax, "Hertz", "Parabolic"
    stdev = Get standard deviation: tmin, tmax, "Hertz"
    range = maximum - minimum




     voicedcount = 0
    
   for i from startloop to validpeakcount
      
      querytime = validtime'i'

      if querytime < tmax and querytime > tmin

        select 'textgridid'
        whichinterval = Get interval at time... 1 'querytime'
        whichlabel$ = Get label of interval... 1 'whichinterval'

        select 'pitchid'
        value = Get value at time... 'querytime' Hertz Linear

   
        if value <> undefined
           if whichlabel$ = "sounding"
               voicedcount = voicedcount + 1
               voicedpeak'voicedcount' = validtime'i'
           endif
        endif
     else
       startloop = i
       goto done
     endif
   endfor
  label done
  totalvowelcount = totalvowelcount + voicedcount
  select 'silencetableid'
   speakingtot = 0
   for ipause from startploop  to npauses
      #pauseScript:startploop
      beginsound = Get value... 'ipause' 1
      endsound = Get value... 'ipause' 2
       beginsound = max('beginsound','tmin')
       endsound = min('endsound','tmax')
       if endsound > beginsound
        speakingdur = 'endsound' - 'beginsound'
        speakingtot = 'speakingdur' + 'speakingtot'
       else
         startploop = ipause
         goto fin
       endif
   endfor
  label fin
 totalspeaking = totalspeaking + speakingtot
# summarize results in Info window
   speakingrate = 'voicedcount'/'time_step'
   articulationrate = 'voicedcount'/'speakingtot'
   npause = 'npauses'-1
   asd = 'speakingtot'/'voicedcount'
   # print a single header line with column names and units

   select 'intid'
   # estimate noise floor
   minint = Get minimum... tmin tmax Parabolic
   #get .99 quantile to get maximum (without influence of non-speech sound bursts)
   maxint = Get maximum... tmin tmax Parabolic

    meanint = Get mean: tmin, tmax, "energy"
    stdevint = Get standard deviation: tmin, tmax
    rangeint = maxint - minint


appendInfoLine: fixed$ (tmin, 0), " ", fixed$ (tmax, 0)  
   printline 'voicedcount', 'npause', 'time_step:2', 'speakingtot:2', 'speakingrate:2', 'articulationrate:2', 'asd:3'

 appendInfoLine:  fixed$ (mean, 2),
    ... " ", fixed$ (minimum, 2), " ", fixed$ (maximum, 2), " ", fixed$ (stdev, 2),
    ... " ", fixed$ (range, 2) 
    
    appendInfoLine: fixed$ (meanint, 2),
    ... " ", fixed$ (minint, 2), " ", fixed$ (maxint, 2), " ", fixed$ (stdevint, 2),
    ... " ", fixed$ (rangeint, 2)
     
    select 'tid'
    Set numeric value... step times tmax
    Set numeric value... step vals meanint
endfor
Erase all
# overall stats
# pitch
select 'pitchid'
mean = Get mean: tmin, tmax, "Hertz"
minimum = Get minimum: 0, 0, "Hertz", "Parabolic"
maximum = Get maximum: 0, 0, "Hertz", "Parabolic"
stdev = Get standard deviation: 0, 0, "Hertz"
range = maximum - minimum
# intensity
select 'intid'
# estimate noise floor
minint = Get minimum... 0 0 Parabolic
#get .99 quantile to get maximum (without influence of non-speech sound bursts)
maxint = Get maximum... 0 0 Parabolic
meanint = Get mean: 0, 0, "energy"
stdevint = Get standard deviation: 0, 0
rangeint = maxint - minint
   speakingrate = 'totalvowelcount'/'originaldur'
   articulationrate = 'totalvowelcount'/'totalspeaking'
   npause = 'npauses'-1
   asd = 'speakingtot'/'voicedcount'
appendInfoLine: "Overall stats"

printline  nsyll, npause, dur (s), phonationtime (s), speechrate (nsyll/dur), articulation rate (nsyll / phonationtime), ASD (speakingtime/nsyll) 
printline 'totalvowelcount','npause','originaldur:2','totalspeaking:2','speakingrate:2','articulationrate:2','asd:3'

 appendInfoLine:  fixed$ (mean, 2),
    ... " ", fixed$ (minimum, 2), " ", fixed$ (maximum, 2), " ", fixed$ (stdev, 2),
    ... " ", fixed$ (range, 2) 
    
    appendInfoLine: fixed$ (meanint, 2),
    ... " ", fixed$ (minint, 2), " ", fixed$ (maxint, 2), " ", fixed$ (stdevint, 2),
    ... " ", fixed$ (rangeint, 2)
appendInfoLine: " fmean fmin fmax fstdev frange"
appendFile:  left$ (filename$, 15)+"txt", info$ ( )

select all
Remove
