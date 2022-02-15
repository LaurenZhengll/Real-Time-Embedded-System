#include <mbed.h>

// Function Prototypes
void part1();
void part2();
void part3();
void extension();

// Define LEDs
DigitalOut led1(LED1);
DigitalOut led2(LED2);

// Extern functions
extern "C" int asm_sub(int a, int b);
extern "C" uint32_t asm_led(uint32_t a);
extern "C" uint8_t asm_error(uint32_t val1,uint32_t val2);
extern "C" uint32_t add_6(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f);


void wait_ms(uint32_t ms) {
  wait_us(1000*ms);
}

/* main function for Assignment 2 - comment out parts you aren't currently working on */
int main() {

	while (1) {
		
		part1();

		part2();

	  part3();

		extension();

		wait_ms(100);
	}

}


/*	subtracting integers
*	Part 1 - objective: Modify the assembly function asm_sub to
*	return the subtraction of integers.
*/
void part1() {

		int result = asm_sub(2,1);
		printf("2-1 = %d\n\r",result);

}


/*	Blinking an LED
*	Part 2 - objective: Modify the assembly function asm_led so it will
*	flip an integer between 0 and 1. Use the result to blink an LED.
* 	Also describe why we are using the static qualifier here
*/

void part2() {

	// what does this qualifier do?  Answer: static makes ledVar a global variable, so when call part2() again, ledVar keeps the value of last time instead of 0
	static uint8_t ledVar = 0;

	ledVar = asm_led(ledVar);
	led1 = ledVar;

}


/*	Assembly LED Error
*	Part 3 - objective: Write a new assembly function asm_error and declare it appropriately in the extern functions list above.
*	You will pass in two integers into the function and add them. If there is an overflow flag pass back a value to turn on led2.
*/

void part3() {
	static uint32_t val1 = 0x70000000;
	static uint32_t val2 = 0x7fffffff;  // set this to something that will cause an overflow

	// write your solution here:
  static uint8_t result = 0;
  result = asm_error(val1,val2);
  led2 = result;
  // if(result == 1){
  //   led2 = result;
  // }
  // else{
  //   printf("%x + %x = %x\n\r",val1,val2,result);
  // }
  

}


/* 	Extension 
*	Objective - Write an assembly function to add 6 integers together
*	Hint1: consider the calling convention discussed in class
*	Hint2: see if you can find a way to inspect memory with the debugger
*	Hint3: Disassemble the c code that calls your function for more insight
*	Bonus: Do this without causing an exception
*/
void extension() {

	uint32_t a = 1;
	uint32_t b = 2;
	uint32_t c = 3;
	uint32_t d = 4;
	uint32_t e = 5;
	uint32_t f = 6;
	uint32_t result;

	// write an assembly function to add the above 6 integers
  result = add_6(a,b,c,d,e,f);

	// print the result
	printf("1+2+3+4+5+6 = : %d\n\r", result);
}