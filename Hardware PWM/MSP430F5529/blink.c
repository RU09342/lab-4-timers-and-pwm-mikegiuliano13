// Loads configurations for all MSP430 boards
#include <msp430.h>

void timerSetup(int t);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

	// LEDs
    P1DIR = BIT0 + BIT2; // Set P1.0 and BIT2 as output
	P4DIR |= BIT7; // Set P4.7 as output
	P1SEL |= BIT2; //Tied to the specific peripheral connected to pin, not general I/O
    
	// Button and Interrupt Configuration
	P1REN |= BIT1; // Connects the on-board resistor to P1.1
    P1OUT = BIT1; // Sets up P1.1 as pull-up resistor
    P1IE |= BIT1; // Enable interrupt on button pin
    P1IFG &= ~BIT1; // Clear interrupt flag

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
	
    P1IFG &= ~BIT1;   // Clear P2.1 interrupt flag
    P1IES &= ~BIT1;  // Disable interrupt by toggling edge
	
	P1OUT |= BIT0; // turn on status LED

}

// Interrupt subroutine
// Called when timer reaches TA0CCR0
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer_A0(void)
{
	P1OUT &= ~BIT0; // turn off status LED
	
	// Increment duty cycle
	if (TA0CCR1 < 100) {
		TA0CCR1 += 10;
		}
	else TA0CCR1 = 0;
	
	P1IE |= BIT1; // Reenable interrupts
	TA1CTL &= ~ TASSEL_2; // Stop timer
	TA1CTL |= TACLR; // Clear Timer
	
}