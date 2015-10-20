#include batch.praat
#procedure action
	s$ = selected$("Sound")
	#execute workpre.praat
	wrk = selected("Sound")
	dur = Get total duration
    pp = To PointProcess (periodic, cc)... 100 300
    tg = To TextGrid (vuv)... 0.1 0.01
    ni = Get number of intervals... 1
    nv = 0
    for i from 1 to 'ni'
      label$ = Get label of interval... 1 'i'
      if label$ == "V"
        nv = 'nv' + 1
 	    st[nv] = Get starting point... 1 'i'
        et[nv] = Get end point... 1 'i'
      endif
    endfor
    select wrk
    pitch = To Pitch... 0.01 100 300
	f0 = Get quantile... 0 0 0.50 Hertz
	f0 = 'f0:3'
    maxf = Get quantile... 0 0 0.99 Hertz
	minf = Get quantile... 0 0 0.01 Hertz
    for i from 1 to 'nv'
      max = Get quantile... st[i] et[i] 0.99 Hertz
      min = Get quantile... st[i] et[i] 0.01 Hertz
      if abs('max' - 'f0') >= abs('f0'-'min')
		gain[i] = 1/abs('max' - 'f0')
      else
		gain[i] = 1/abs('f0'-'min')
      endif
    endfor
	plus wrk
	manipulation = To Manipulation
	pitchtier = Extract pitch tier
    for i from 1 to 'nv'
		si = Get high index from time... st[i]
        ei = Get low index from time... et[i] 
	    for j from si to ei
		  tim = Get time from index... j
		  val = Get value at index... j
          if 'val' > 'f0'
          	val = ('val'-'f0')*('maxf'-'f0')*gain[i] + 'f0'
		  else
			val = ('val'-'f0')*('f0'-'minf')*gain[i] + 'f0'
          endif
          Remove point... 'j'
          Add point... tim val
	    endfor
	endfor
	plus manipulation
	Replace pitch tier
	select manipulation
	res = Get resynthesis (overlap-add)
	#execute workpost.praat
	result = Rename... 's$'_changepitchrange_'pitch_range'
	select pitch
	plus wrk
	plus manipulation
	plus pitchtier
	plus res
#endproc
