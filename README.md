# Synthesator
SDL (or any) freely programmed Synthesator. you may construct any music instruments (drums, guitar, etc), use MLL syntax, and play it

Run 8 different track programs (single string with instructions) parallel. 
Waves - svw,pulse,triangle, noise, sinus, PCM (255 points), Furie (typical 12 harmonics), Bezier (you may construct 255 points for bezier type waves - very natural and powerful) and polynom - wave is polynom function.
also waves may modulated - Ringmodulation -frequency modulated. Filter may modulated (with any track or enevlopes)

Project MLL syntax:
abcdefg - Notes  & time (1,2,4,...32) is 1/time example a1 (full note) b2 c
if not time then default 1/4 time
. dot add half time
o - octav. + change +1, - change -1 number 0-11 set octav
v nr - volume. nr 0...255 - it sets % (nr/100)
s silence 1/time
H Higphass filter invert
H+ set Highpass filter ON
H- set Highpass filter OFF
H nr - nr is cutoff frequency
[MLL]nr - repeats nr time. If not nr time or nr over 255 then infinity loop
(notes)nr - chords. nr is note time 1/nr. only notes and octavmay inside.
W - wait soft syncro
Snr - Send nr bits start soft syncro.
