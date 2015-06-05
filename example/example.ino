#include <TimerOne.h>
#include "multi_timer.h"

void setup() {
  mt_init();
  pinMode(13, OUTPUT);
  mt_set_timer(2, 1, &hello, NULL);

}

void hello(void *e) {
   digitalWrite( 13, digitalRead( 13 ) ^ 1 );
   mt_set_timer(2, 1, &hello, NULL);
}



void loop(){
  
}