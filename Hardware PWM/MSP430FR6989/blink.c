// Loads configurations for all MSP430 boards
#include <msp430.h>

void timerSetup(int t);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	
	// Disables default high-impedance mode
	PM5CTL0 &= ~LOCKLPM5;

	// LEDs
    P1DIR = BIT0; // Set P1.0 as output
	P9DIR = BIT7; // Set P9.7 as output
	P9OUT &= ~BIT7; // Initialize P9.7 as off
	P1DIR &= ~BIT2; // Sets P1.2 as input
	P1SEL0 |= BIT0; //Tied to the specific peripheral connected to pin, not general I/O
    
	// Button and Interrupt Configuration
	P1REN |= BIT2; // Connects the on-board resistor to P1.2
    P1OUT = BIT2; // Sets up P1.2 as pull-up resistor
    P1IE |= BIT2; // Enable interrupt on button pin
    P1IFG &= ~BIT2; // Clear interrupt flag

	// Timer frequency of 100 Hz --> 10 ms intervals
    timerSetup(100);    // initialize timer to 100Hz

    __enable_interrupt(); // MUST BE ENABLED IN ADDITION TO GIE
    __bis_SR_register(GIE); // enable global interrupts

}

// Sets up the timer compare value to 
void timerSetup(int t)
{
	int x;
    x = 1000000 / t;
    TA1CCR0 = x; // ex. t = 10 --> (1000000 [Hz]) / 100000 = 10 Hz
    TA1CCTL0 = CCIE; // capture compare interrupt enabled
    
    // DUTY CYCLE Timer
	TA0CCTL1 = OUTMOD_7; // sets and resets the capture compare
    TA0CCR1 = 50; //initialization of duty cycle 50% (variable)
	TA0CCR0 = 100; // maximum duty cycle (fixed)
    TA0CTL = TASSEL_2 + MC_1; 
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)
{

    // TA1CTL = Timer A0 chosen for use
    // TASSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TACLR clears timer A0 register
	TA1CTL = TASSEL_2 + MC_1; // Begin timer right away
	
    P1IFG &= ~BIT2;   // Clear P1.2 interrupt flag
    P1IES &= ~BIT2;  // Disable interrupt by toggling edge
	
	P9OUT |= BIT7; // turn on status LED

}

// Interrupt subroutine
// Called when timer reaches TA0CCR0
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0(void)
{
	P9OUT &= ~BIT7; // turn off status LED
	
	// Increment duty cycle
	if (TA0CCR1 < 100) {
		TA0CCR1 += 10;
		}
	else TA0CCR1 = 0;
	
	P1IE |= BIT2; // Reenable interrupts
	TA1CTL &= ~ TASSEL_2; // Stop timer
	TA1CTL |= TACLR; // Clear Timer
	
}