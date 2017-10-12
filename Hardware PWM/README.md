# Lab 4: Hardware PWM

## General Structure

The initial difference compared to software PWM is the enabling of the P1 Select
command for the green LED. This ties the port specifically to the timer peripheral
as opposed to general I/O. The timer used for duty cycle modulation had to be
chosen carefully since typically a specific one interacts with the Select command.

This program begins by configuring the two LEDs. The button is configured to enable
interrupts and the frequency calculation function is called to set the debounce wait
period (10 ms) and store it in its associated capture compare register. In this way,
one timer is used to manage debounce timing while another is used to modulate the 
duty cycle. The duty cycle timer used OUTMOD_7 to reset the timer when it counts to
the variable duty cycle value. It is set when the timer counts to the fixed value.

When the button is pressed, this signals the timer to begin counting immediately and
also clearing the interrupt flag while preventing future ones from occuring. The button
interrupt also turns the status LED on.

The timer interrupt simply increments the duty cycle by 10%, turns off the status LED,
and prepares the program for another interrupt to fire by reenabling them and resetting
the timer.

## Important Distinctions

Generally, differences were in the pinouts for pull-up resistors and buttons.
The MSP430FR2311 and MSP430FR5994 required the use of Timer B instead of Timer A.
Additionally, the timer used for the duty cycle needed to be connected to the
corresponding LED using Pin Select. This determined which timer was used for a 
given microprocessor.

### MSP430G2553
//---------------------------------------------------------------------------------------

// Loads configurations for all MSP430 boards
#include <msp430.h>

void timerSetup(int t);

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

	// LEDs
    P1DIR = BIT0 + BIT6; // Set P1.0 and BIT6 as output
	P1SEL |= BIT6; //Tied to the specific peripheral connected to pin, not general I/O
    
	// Button and Interrupt Configuration
	P1REN |= BIT3; // Connects the on-board resistor to P1.3
    P1OUT = BIT3; // Sets up P1.3 as pull-up resistor
    P1IE |= BIT3; // Enable interrupt on button pin
    P1IFG &= ~BIT3; // Clear interrupt flag

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
	
    P1IFG &= ~BIT3;   // Clear P1.3 interrupt flag
    P1IES &= ~BIT3;  // Disable interrupt by toggling edge
	
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
	
	P1IE |= BIT3; // Reenable interrupts
	TA1CTL &= ~ TASSEL_2; // Stop timer
	TA1CTL |= TACLR; // Clear Timer
	
}

### MSP430FR2311
//---------------------------------------------------------------------------------------

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

### MSP430F5529
//---------------------------------------------------------------------------------------

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

### MSP430FR5994
//---------------------------------------------------------------------------------------

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

### MSP430FR6989
//---------------------------------------------------------------------------------------

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