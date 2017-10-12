// Loads configurations for all MSP430 boards
#include <msp430.h>

void frequencyCalc(int t);

volatile int state = 0;
volatile int dutycycle = 50; // duty cycle initialized at 50%

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	
	// Disables default high-impedance mode
	PM5CTL0 &= ~LOCKLPM5;

	// Button configuration
    P1DIR = BIT0 + BIT1; // Set P1.0 and P1.1 as output
    P5REN |= BIT5; // Connects the on-board resistor to P5.5
    P5OUT = BIT5; // Sets up P5.5 as pull-up resistor
    
	// Interrupt Configuration
    P5IES |= BIT5; // Interrupts on button release LO TO HI
    P5IE |= BIT5; // Enable interrupt on button pin
    P5IFG &= ~BIT5; // Clear interrupt flag

	// Timer frequency of 100 Hz --> 10 ms intervals
    frequencyCalc(100);    // initialize timer to 100Hz

    __enable_interrupt(); // MUST BE ENABLED IN ADDITION TO GIE
    __bis_SR_register(GIE); // enable global interrupts
    
	// Compare the current value of the timer in the A1 register
	// with the duty cycle to determine the rate of LED flicker
    while (1) {
        if(TA1R <= dutycycle) 
            P1OUT ^= BIT0;
        else if (TA1R > dutycycle) 
            P1OUT &= ~BIT0;
    }

}

// Sets up the timer compare value to 
void frequencyCalc(int t)
{
	int x;
    x = 1000000 / t;
    TA0CCR0 = x; // ex. t = 10 --> (1000000 [Hz]) / 100000 = 10 Hz
    TA0CCTL0 = CCIE; // capture compare interrupt enabled
    
    // Duty cycle timer
    TA1CCR0 = 100;
    TA1CTL = TASSEL_2 + MC_1 + TACLR; 
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT5_VECTOR
__interrupt void PORT_5(void)
{

    // TA0CTL = Timer A0 chosen for use
    // TASSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TACLR clears timer A0 register
	TA0CTL = TASSEL_2 + MC_1 + TACLR; // Begin timer right away
	
    P5IFG &= ~BIT5;   // Clear P5.5 interrupt flag
    P5IE &= ~BIT5;  // Disable interrupts to prevent false alarm

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
	    if(dutycycle < 100) {
	        dutycycle = dutycycle + 10;
	    }
        else dutycycle = 0;     
		P1OUT ^= BIT1; // Blink green LED
		P5IES &= ~BIT5; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P1OUT ^= BIT1; // Blink green LED
		P5IFG &= ~BIT5; // Clear flag
		P5IES |= BIT5; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P5IE |= BIT5; // Reenable interrupts
	TA0CTL &= ~ TASSEL_2; // Stop timer
	TA0CTL |= TACLR; // Clear Timer
	
}