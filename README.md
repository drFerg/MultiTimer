MultiTimer
==========

A timer multiplexer for the Arduino.


Created whilst working on another project requiring callback timers (KNoT) and the available libraries were only polling timers or single timers(TimerOne).

At the moment this uses interrupts to allow timers to be accurately checked in the background whilst the foreground tasks can check and service and overdue timers at any time.

Modifications will be made to also allow callback functions to be executed upon the relevant timer interrupt for immediate execution where timing is critical (radio).