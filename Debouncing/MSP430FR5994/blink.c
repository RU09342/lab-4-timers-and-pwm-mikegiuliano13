// Loads configurations for all MSP430 boards
#include <msp430.h>

void frequencyCalc(int t);

int state = 0;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	
	// Disables default high-impedance mode
	// Done by clearing the register PM5CTL0
	// and unlocking I/O pins  (~LOCKLPM5)
	PM5CTL0 &= ~LOCKLPM5;

	// Button configuration
    P1DIR = BIT0; // Set P1.0 as output
    P5REN |= BIT6; // Connects the on-board resistor to P5.6
    P5OUT = BIT6; // Sets up P1.1 as pull-up resistor
    
	// Interrupt COnfiguration
    P5IES |= BIT6; // Interrupts on button release LO TO HI
    P5IE |= BIT6; // Enable interrupt on button pin
    P5IFG &= ~BIT6; // Clear interrupt flag
    
    TB0CCTL0 = CCIE; // CCR0 interrupt enabled

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
    TB0CCR0 = x; // ex. t = 10 --> (10^6 [Hz] / 4) / 25000 = 10 Hz
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)
{

    // TB0CTL = Timer A0 chosen for use
    // TBSSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// ID_2 Pre divides the clock by 4
	// TBCLR clears timer A0 register
	TB0CTL = TBSSEL_2 + MC_1 + ID_2 + TBCLR; // Begin timer right away
	
    P5IFG &= ~BIT6;   // Clear P1.3 interrupt flag
    P5IE &= ~BIT6;  // Disable interrupts to prevent false alarm

}

// Interrupt subroutine
// Called when timer reaches TA0CCR0
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B0(void)
{

	// This switch is the logic for determining the status of the button
	// On press, the case 0 loop is entered, and on release the case 1 loop is entered
	
	switch(state) {
	
	case 0:
		P5IES &= ~BIT6; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P1OUT ^= BIT0; // Blink LED
		P5IFG &= ~BIT6; // Clear flag
		P5IES |= BIT6; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P5IE |= BIT6; // Reenable interrupts
	TB0CTL &= ~ TBSSEL_2; // Stop timer
	TB0CTL |= TBCLR; // Clear Timer
	
}