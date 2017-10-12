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
	P2DIR = BIT0; // Set P2.0 as output
	P2SEL0 |= BIT0; //Tied to the specific peripheral connected to pin, not general I/O
    
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
    TB0CCR0 = x; // ex. t = 10 --> (1000000 [Hz]) / 100000 = 10 Hz
    TB0CCTL0 = CCIE; // capture compare interrupt enabled
    
    // DUTY CYCLE Timer
	TB1CCTL1 = OUTMOD_7; // sets and resets the capture compare
    TB1CCR1 = 50; //initialization of duty cycle 50% (variable)
	TB1CCR0 = 100; // maximum duty cycle (fixed)
    TB1CTL = TBSSEL_2 + MC_1; 
}

// Interrupt subroutine
// Called whenever button is pressed
#pragma vector = PORT1_VECTOR
__interrupt void PORT_1(void)
{

    // TB0CTL = Timer A0 chosen for use
    // TBSSEL_2 Selects SMCLK as clock source
    // MC_1 Count-up mode
	// TBCLR clears timer A0 register
	TB0CTL = TBSSEL_2 + MC_1; // Begin timer right away
	
    P1IFG &= ~BIT1;   // Clear P1.1 interrupt flag
    P1IES &= ~BIT1;  // Disable interrupt by toggling edge
	
	P1OUT |= BIT0; // turn on status LED

}

// Interrupt subroutine
// Called when timer reaches TB0CCR0
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B0(void)
{
	P1OUT &= ~BIT0; // turn off status LED
	
	// Increment duty cycle
	if (TB1CCR1 < 100) {
		TB1CCR1 += 10;
		}
	else TB1CCR1 = 0;
	
	P1IE |= BIT1; // Reenable interrupts
	TB0CTL &= ~ TBSSEL_2; // Stop timer
	TB0CTL |= TBCLR; // Clear Timer
	
}