// Loads configurations for all MSP430 boards
#include <msp430.h>

void frequencyCalc(int t);

volatile int state = 0;
volatile int dutycycle = 50; // duty cycle initialized at 50%
volatile int dutycount = 5; // keeps track of exponential duty cycle

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

	// Button configuration
    P1DIR = BIT0 + BIT6; // Set P1.0 and BIT6 as output
    P1REN |= BIT3; // Connects the on-board resistor to P1.3
    P1OUT = BIT3; // Sets up P1.3 as pull-up resistor
    
	// Interrupt Configuration
    P1IES |= BIT3; // Interrupts on button release LO TO HI
    P1IE |= BIT3; // Enable interrupt on button pin
    P1IFG &= ~BIT3; // Clear interrupt flag

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
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)
{

    // TA0CTL = Timer A0 chosen for use
    // TASSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TACLR clears timer A0 register
	TA0CTL = TASSEL_2 + MC_1 + TACLR; // Begin timer right away
	
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
	
		// LOGARITHMIC CHANGE
		// The logarithmic scale is approximated according to the equation y = e^(x-3) * 100
		// where x ranges from 0 to 3 with a step size of 0.3 to produce 10 intervals.
		// Tweaking on the fringe cases (case 0 and case 9) was done to make the model
		// more logarithmic in nature and consistent in terms of viewing brightness.
	    if(dutycount < 10) {
			switch (dutycount) {
				case 0:
					// e^(0.3-3) * 100 = 6
					dutycycle = dutycycle + 2; // changed reduced by 4 here (from 6)
					dutycount++;
					break;
				case 1:
					// e^(0.6-3) * 100 = 9
					dutycycle = dutycycle + 3; // 9-6=3
					dutycount++;
					break;
				case 2:
					// e^(0.9-3) * 100 = 12
					dutycycle = dutycycle + 3; // 12-9=3
					dutycount++;
					break;
				case 3:
					// e^(1.2-3) * 100 = 17
					dutycycle = dutycycle + 5; // 17-12=5
					dutycount++;
					break;
				case 4:
					// e^(1.5-3) * 100 = 22
					dutycycle = dutycycle + 5; // 22-17=5
					dutycount++;
					break;
				case 5:
					// e^(1.8-3) * 100 = 30
					dutycycle = dutycycle + 8; // 30-22=8
					dutycount++;
					break;
				case 6:
					// e^(2.1-3) * 100 = 41
					dutycycle = dutycycle + 11; // 41-30=11
					dutycount++;
					break;
				case 7:
					// e^(2.4-3) * 100 = 55
					dutycycle = dutycycle + 14; // 55-41=14
					dutycount++;
					break;
				case 8:
					// e^(2.7-3) * 100 = 74
					dutycycle = dutycycle + 19; // 74-55=19
					dutycount++;
					break;
				case 9:
					// e^(3-3) * 100 = 100 = 26
					dutycycle = dutycycle + 30; // change increased by 4 here (from 26)
					dutycount++;
					break;
			}
	    }
        else {
			dutycycle = 0;
			dutycount = 0;
		}
		
		P1OUT ^= BIT6; // Blink green LED
		P1IES &= ~BIT3; // Set edge HI to LO
		state = 1;
		break;
	case 1:
		P1OUT ^= BIT6; // Blink green LED
		P1IFG &= ~BIT3; // Clear flag
		P1IES |= BIT3; // Set Edge LO to HI
		state = 0;
		break;
	}
	
	P1IE |= BIT3; // Reenable interrupts
	TA0CTL &= ~ TASSEL_2; // Stop timer
	TA0CTL |= TACLR; // Clear Timer
	
}