dur = Get total duration
stepsize = 0.02
windowsize = 0.1
threshold = 100
nsteps = 'dur'/ 'stepsize'
writeInfoLine: "start"
for step from 0 to 'nsteps'
std = Get standard deviation... 3 'stepsize'*'step' 'stepsize'*'step'+'windowsize' Hertz
        if std<>undefined 
			if std<threshold and std > 1
		std = Get standard deviation... 2 'stepsize'*'step' 'stepsize'*'step'+'windowsize' Hertz
        if std<>undefined 
			if std<threshold and std > 1
        		std = Get standard deviation... 1 'stepsize'*'step' 'stepsize'*'step'+'windowsize' Hertz
				if std<threshold and std >1
					appendInfoLine: 'step'*stepsize,",",'std'
				endif
			endif
		endif
endif
endif
		#pauseScript: 'step'*stepsize,", ",'formant',", ",'std'
endfor
appendInfoLine: "end"