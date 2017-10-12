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
    P1REN |= BIT1; // Connects the on-board resistor to P1.1
    P1OUT = BIT1; // Sets up P1.1 as pull-up resistor
    
	// Interrupt COnfiguration
    P1IES |= BIT1; // Interrupts on button release LO TO HI
    P1IE |= BIT1; // Enable interrupt on button pin
    P1IFG &= ~BIT1; // Clear interrupt flag
    
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
	
    P1IFG &= ~BIT1;   // Clear P1.3 interrupt flag
    P1IE &= ~BIT1;  // Disable interrupts to prevent false alarm

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
		P1IES &= ~BIT1; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P1OUT ^= BIT0; // Blink LED
		P1IFG &= ~BIT1; // Clear flag
		P1IES |= BIT1; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P1IE |= BIT1; // Reenable interrupts
	TB0CTL &= ~ TBSSEL_2; // Stop timer
	TB0CTL |= TBCLR; // Clear Timer
	
}