// Loads configurations for all MSP430 boards
#include <msp430.h>

void timerSetup(int t);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	
	// Disables default high-impedance mode
	PM5CTL0 &= ~LOCKLPM5;

	// LEDs
    P1DIR = BIT0 + BIT1; // Set P1.0 and BIT1 as output
	P1OUT &= ~BIT1; // Initialize P1.1 as off
	P5DIR &= ~BIT5; // Sets P5.5 as input
	P1SEL0 |= BIT0; //Tied to the specific peripheral connected to pin, not general I/O
    
	// Button and Interrupt Configuration
	P5REN |= BIT5; // Connects the on-board resistor to P5.5
    P5OUT = BIT5; // Sets up P5.5 as pull-up resistor
    P5IE |= BIT5; // Enable interrupt on button pin
    P5IFG &= ~BIT5; // Clear interrupt flag

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
#pragma vector = PORT5_VECTOR
__interrupt void PORT_5(void)
{

    // TA1CTL = Timer A0 chosen for use
    // TASSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TACLR clears timer A0 register
	TA1CTL = TASSEL_2 + MC_1; // Begin timer right away
	
    P5IFG &= ~BIT5;   // Clear P1.3 interrupt flag
    P5IES &= ~BIT5;  // Disable interrupt by toggling edge
	
	P1OUT |= BIT1; // turn on status LED

}

// Interrupt subroutine
// Called when timer reaches TA0CCR0
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0(void)
{
	P1OUT &= ~BIT1; // turn off status LED
	
	// Increment duty cycle
	if (TA0CCR1 < 100) {
		TA0CCR1 += 10;
		}
	else TA0CCR1 = 0;
	
	P5IE |= BIT5; // Reenable interrupts
	TA1CTL &= ~ TASSEL_2; // Stop timer
	TA1CTL |= TACLR; // Clear Timer
	
}