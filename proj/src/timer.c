#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"


uint32_t timer_counter = 0;
static int hook_id = 0;


int (timer_set_frequency)(uint8_t timer, uint32_t freq) {

  uint8_t msb = 0, lsb = 0;
  uint16_t timerfreq = TIMER_FREQ/freq; 
  //the timer generates a square wave with a frequency given by the expression clock/div, where clock is the frequency of the Clock input and div is the value loaded initially in the timer.

  if(util_get_LSB(timerfreq, &lsb) != 0){ //get least significant byte of timerfreq
    printf("It was detected an error in util_get_LSB!\n");
    return 1;
  }

  if(util_get_MSB(timerfreq, &msb) != 0){ //get most significant byte of timerfreq
    printf("It was detected an error in util_get_MSB!\n");
    return 1;
  }
  
  uint8_t aux;

  if (timer_get_conf(timer, &aux) != 0){   
    printf("It was detected an error in timer_get_conf!\n");
    return 1;
  }

  //based on the number of the timer, we have to send the control word to program the timer

  if (timer == 0){ 
    aux = ((aux & 0x0f)|TIMER_SEL0|TIMER_LSB_MSB); //it does not change the 4 LSBs of the timer's control word
    if(sys_outb(TIMER_CTRL, aux) != 0){ //to program the timer we send the control word
      printf("It was detected an error in sys_outb!\n");
      return 1;
    }
    if(sys_outb(TIMER_0, lsb) != 0){ //it programs the timer
      printf("It was detected an error in sys_outb!\n"); 
      return 1;
    }
    if(sys_outb(TIMER_0, msb) != 0){ //it programs the timer
      printf("It was detected a32n error in sys_outb!\n");
      return 1;
    }
    return 0;
  }
  else if (timer == 1){
    aux = ((aux & 0x0f)|TIMER_SEL1|TIMER_LSB_MSB); //it does not change the 4 LSBs of the timer's control word
    if(sys_outb(TIMER_CTRL, aux) != 0){ //to program the timer we send the control word
      printf("It was detected an error in sys_outb!\n"); 
      return 1;
    }
    if(sys_outb(TIMER_1, lsb) != 0){ //it programs the timer
      printf("It was detected an error in sys_outb!\n");
      return 1;
    }
    if(sys_outb(TIMER_1, msb) != 0){ //it programs the timer
      printf("It was detected an error in sys_outb!\n");
      return 1;
    }
    return 0;
  }
  else if (timer == 2){
    aux = ((aux & 0x0f)|TIMER_SEL2|TIMER_LSB_MSB); //it does not change the 4 LSBs of the timer's control word
    if(sys_outb(TIMER_CTRL, aux) != 0){ //to program the timer we send the control word
      printf("It was detected an error in sys_outb!\n");
      return 1;
    }
    if(sys_outb(TIMER_2, lsb) != 0){ //it programs the timer
      printf("It was detected an error in sys_outb!\n");
      return 1;
    }
    if(sys_outb(TIMER_2, msb) != 0){ //it programs the timer
      printf("It was detected an error in sys_outb!\n");
      return 1;
    }
    return 0;
  }
  else{
    printf("Error: Invalid timer\n");
    return 1;
  }
}



int (timer_subscribe_int)(uint8_t *bit_no) { //Subscribes and enables Timer 0 interrupts. 
    
    *bit_no = BIT(hook_id); 

    //sys_irqsetpolicy() should be used to subscribe a notification on every interrupt in the input irq_line

    if(sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id) != 0){ // we use IRQ_REENABLE policy in order not to call sys_irqenable() according to the handouts (section 7.3)
    //hook_id guarda o nosso id

      printf("It was detected an error in sys_irqsetpolicy!\n");
      return 1;
    }

  return 0;
}


int (timer_unsubscribe_int)() { //Unsubscribes Timer 0 interrupts. 
  
  //sys_irqrmpolicy() unsubscribes a previous subscription of the interrupt notification associated with the specified hook_id 

  if(sys_irqrmpolicy(&hook_id) != 0){ 

    //note: hook_id is the value returned by sys_irqsetpolicy() in its hook_id argument

      printf("It was detected an error in sys_irqrmpolicy!\n");
      return 1;
    }

  return 0;
}

void (timer_int_handler)() { // increments counter
  timer_counter++;
}


int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {

  union timer_status_field_val val;
  uint8_t aux;
  
  if (field == tsf_all){ // all --> Display status byte, in hexadecimal
    val.byte = st;   //byte --> The status byte
  }
  else if (field == tsf_initial){ // initial --> Display only the initialization mode
    aux = ((st << 2) >> 6);  // bits: 5 and 4 --> represents initialization mode  (it could be replaced with aux = aux & (BIT(0)|BIT(1)))

    // in_mode --> The initialization mode
    
    if (aux == 0){ // binary: 00
	    val.in_mode = INVAL_val;   // INVAL_val --> Invalid initialization mode
    }
    else if (aux == 1){ // binary: 01
      val.in_mode = LSB_only; // LSB_only --> Initialization only of the LSB
    }
    else if (aux == 2 ){ // binary: 10
      val.in_mode = MSB_only; // MSB_only --> Initialization only of the MSB
    }
    else if (aux == 3){ // binary: 11
      val.in_mode = MSB_after_LSB; //MSB_after_LSB --> Initialization of LSB and MSB, in this order
    }
    else {
      printf("Error: Invalid initialization mode\n");
      return 1;
    }
  }
  else if(field == tsf_mode){ // mode --> Display only the counting mode
    aux = ((st << 4) >> 5); //bits: 3, 2 and 1 --> represents counting mode

    //count_mode --> The counting mode: 0, 1,.., 5 (values from table 1 in section 3.2)


    if(aux == 0x0){  // binary: 000
 	    val.count_mode = 0; 
    }

    else if(aux == 0x1){ // binary: 001
      val.count_mode = 1;
    }

    else if(aux == 0x2 || aux == 0x6){ // binary: 010 and 110
      val.count_mode = 2;
    }
    
    else if(aux == 0x3 || aux == 0x7){ // binary: 011 and 111
      val.count_mode = 3;
    }

    else if(aux == 0x4){ // binary: 100
      val.count_mode = 4;
    }
  
    else if(aux == 0x5){ // binary: 101
      val.count_mode = 5;
    }

    else{
      printf("Error: Invalid counting mode\n");
      return 1;
    }
  }
  
  else if (field == tsf_base){ // base --> Display only the counting base
    aux = ((st << 7) >> 7); // bit: 0 --> represents the counting base

    //bcd --> if aux == 1, bcd is true
    val.bcd = aux;
  }

  else {
    printf("Error: Invalid field\n"); //detects if field is not valid
    return 1;
  }

  if(timer_print_config(timer, field, val) != 0){
    printf("It was detected an error in timer_print_config!\n");  // detects if the function call worked well
    return 1;
  }

  return 0;
}
