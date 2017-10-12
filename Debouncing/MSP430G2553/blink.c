// Loads configurations for all MSP430 boards
#include <msp430.h>

void frequencyCalc(int t);

int state = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

	// Button configuration
    P1DIR = BIT0; // Set P1.0 as output
    P1REN |= BIT3; // Connects the on-board resistor to P1.3
    P1OUT = BIT3; // Sets up P1.3 as pull-up resistor
    
	// Interrupt COnfiguration
    P1IES |= BIT3; // Interrupts on button release LO TO HI
    P1IE |= BIT3; // Enable interrupt on button pin
    P1IFG &= ~BIT3; // Clear interrupt flag
    
    TA0CCTL0 = CCIE; // CCR0 interrupt enabled

	// Timer frequency of 100 Hz --> 10 ms intervals
    frequencyCalc(100);    // initialize timer to 100Hz

    __enable_interrupt(); // MUST BE ENABLED IN ADDITION TO GIE
    __bis_SR_register(LPM0 + GIE); // enable interrupts in LPM0

}

// Sets up the timer compare value to 
void frequencyCalc(int t)
{
	int x;
    x = 250000 / t;
    TA0CCR0 = x; // ex. t = 10 --> (10^6 [Hz] / 4) / 25000 = 10 Hz
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)
{

    // TA0CTL = Timer A0 chosen for use
    // TASSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// ID_2 Pre divides the clock by 4
	// TACLR clears timer A0 register
	TA0CTL = TASSEL_2 + MC_1 + ID_2 + TACLR; // Begin timer right away
	
    P1IFG &= ~BIT3;   // Clear P1.3 interrupt flag
    P1IE &= ~BIT3;  // Disable interrupts to prevent false alarm

}

// Interrupt subroutine
// Called when timer reaches TA0CCR0
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A0(void)
{

	// This switch is the logic for determining the status of the button
	// On press, the case 0 loop is entered, and on release the case 1 loop is entered
	
	switch(state) {
	
	case 0:
		P1IES &= ~BIT3; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P1OUT ^= BIT0; // Blink LED
		P1IFG &= ~BIT3; // Clear flag
		P1IES |= BIT3; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P1IE |= BIT3; // Reenable interrupts
	TA0CTL &= ~ TASSEL_2; // Stop timer
	TA0CTL |= TACLR; // Clear Timer
	
}